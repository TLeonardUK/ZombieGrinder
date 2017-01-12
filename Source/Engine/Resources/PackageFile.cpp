// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Resources/PackageFile.h"
#include "Engine/Resources/PackageFileStream.h"
#include "Generic/Helper/StringHelper.h"
#include "Engine/IO/StreamFactory.h"
#include "Generic/Data/Hashes/CRC32DataTransformer.h"
#include "Generic/Math/Math.h"
#include "Engine/Platform/Platform.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <stdlib.h>
#include <unistd.h>
#endif

#define SIGN_BUFFER_LENGTH 1024 * 1024

PackageFile::PackageFile()
{
	m_header.magic				= PackageFileHeader::MAGIC_NUMBER;
	m_header.version			= PackageFileHeader::CURRENT_VERSION;
	m_header.chunk_count		= 0;
	m_header.chunk_strings_size = 0;
	m_header.chunk_data_size	= 0;
	m_header.workshop_id		= 0;
	m_header.workshop_name[0]	= '\0';

	memset(m_header.signature, 0, PackageFileHeader::SIGNATURE_LENGTH);
	memset(m_header.guid, 0, PackageFileHeader::MAX_GUID_LENGTH);

	m_header.chunk_strings		= NULL;
	m_header.chunks				= NULL;
	m_header.chunk_data_offset	= 0;

	m_use_compression = false;
	m_is_signed = false;
	m_is_whitelisted = false;

	m_package_streams_mutex = Mutex::Create();
}

PackageFile::~PackageFile()
{
	SAFE_DELETE_ARRAY(m_header.chunk_strings);
	SAFE_DELETE_ARRAY(m_header.chunks);
}

bool PackageFile::Is_Whitelisted()
{
	return m_is_whitelisted;
}

void PackageFile::Whitelist()
{
	m_is_whitelisted = true;
}

void PackageFile::Set_Keys(CryptoKey private_key, CryptoKey public_key)
{
	m_private_key = private_key;
	m_public_key = public_key;
}

bool PackageFile::Is_Signed()
{
	return m_is_signed;
}

void PackageFile::Use_Compression(bool value)
{
	m_use_compression = value;
}

void PackageFile::Set_Tmp_Folder(const char* url)
{
	m_tmp_dir = url;
}

PackageFileHeader* PackageFile::Get_Header()
{
	return &m_header;
}

std::string PackageFile::Get_Name()
{
	std::string workshop_name = m_header.workshop_name;
	if (workshop_name == "")
	{
		workshop_name = Platform::Get()->Extract_Basename(m_package_path);
	}
	return workshop_name;
}

int PackageFile::Get_Chunk_Count()
{
	return m_header.chunk_count;
}

PackageFileChunk* PackageFile::Get_Chunk(int index)
{
	return m_header.chunks + index;
}

const char* PackageFile::Get_String(int offset)
{
	PackageFileString* string = reinterpret_cast<PackageFileString*>(m_header.chunk_strings + offset);
	char* name = (char*)string + sizeof(PackageFileString);

	return name;
}

unsigned int PackageFile::Get_String_Hash(int offset)
{
	PackageFileString* string = reinterpret_cast<PackageFileString*>(m_header.chunk_strings + offset);
	return string->hash;
}

void PackageFile::Open(const char* path)
{
	Stream* stream = StreamFactory::Open(path, StreamMode::Read);
	DBG_ASSERT(stream != NULL);

	// Read in header
	stream->ReadBuffer((char*)&m_header, 0, sizeof(PackageFileHeader));
	m_header.chunk_strings = new char[m_header.chunk_strings_size];
	m_header.chunks = new PackageFileChunk[m_header.chunk_count];
	
	// Read in all the chunk strings.
	stream->ReadBuffer(m_header.chunk_strings, 0, m_header.chunk_strings_size);

	// Read in all the chunks.
	stream->ReadBuffer((char*)m_header.chunks, 0, m_header.chunk_count * sizeof(PackageFileChunk));

	// Store data offset.
	m_header.chunk_data_offset = stream->Position();

	DBG_LOG("Loaded %i package chunks.", m_header.chunk_count);

	// Add all chunks to hash table.
	m_chunk_hash_table.Clear();
	for (unsigned int i = 0; i < m_header.chunk_count; i++)
	{
		PackageFileChunk* chunk = m_header.chunks + i;

		// We don't add source files as there will almost certainly be hash collisions there!
		if (chunk->type == PackageFileChunkType::Source)
		{
			continue;
		}

		const char* name_str = Get_String(chunk->name_offset);
		unsigned int name_hash = StringHelper::Hash(StringHelper::Lowercase(Normalize_Chunk_Path(name_str).c_str()).c_str());
		m_chunk_hash_table.Set(name_hash, chunk);
	}

	m_package_path = path;

	// Validate signature.
	if (!m_public_key.SameAs(CryptoKey::Empty))
	{
		CryptoStream* crypto_stream = new CryptoStream(CryptoStreamMode::Validate, m_public_key);

		stream->Seek(sizeof(PackageFileHeader));
		int data_remaining = stream->Length() - sizeof(PackageFileHeader);
		char* data_buffer = new char[SIGN_BUFFER_LENGTH];
		while (data_remaining > 0)
		{
			int to_read = Min(data_remaining, SIGN_BUFFER_LENGTH);
			stream->ReadBuffer(data_buffer, 0, to_read);
			crypto_stream->WriteBuffer(data_buffer, 0, to_read);
			data_remaining -= to_read;
		}		

		SAFE_DELETE(data_buffer);
		m_is_signed = crypto_stream->CheckSignature(m_header.signature, PackageFileHeader::SIGNATURE_LENGTH);
		SAFE_DELETE(crypto_stream);
	}
	else
	{
		m_is_signed = false;
	}

	stream->Close();
	SAFE_DELETE(stream);
}

void PackageFile::Close()
{
	MutexLock lock(m_package_streams_mutex);

	// Wait for all streams to close.
	DBG_LOG("Waiting for package streams to close ...");
	while (true)
	{
		bool any_open = false;

		for (std::vector<PackageFileBaseStream*>::iterator iter = m_package_streams.begin(); iter != m_package_streams.end(); iter++)
		{
			PackageFileBaseStream* stream = *iter;
			if (stream->in_use == true)
			{
				any_open = true;
			}
		}

		if (any_open == false)
		{
			break;
		}

		Platform::Get()->Sleep(10.0f);
	}

	// Dispose of streams.
	for (std::vector<PackageFileBaseStream*>::iterator iter = m_package_streams.begin(); iter != m_package_streams.end(); iter++)
	{
		PackageFileBaseStream* stream = *iter;
		stream->base_stream->Close();
		SAFE_DELETE(stream->base_stream);
		SAFE_DELETE(stream);
	}

	m_package_streams.clear();
}

std::string PackageFile::Normalize_Chunk_Path(const char* path)
{
	std::string normalized = path;
	//normalized = StringHelper::Lowercase(path);							// lowercase for all.
	normalized = StringHelper::Replace(normalized.c_str(), "\\", "/");	// back slashes to forward slashes
	normalized = StringHelper::Replace(normalized.c_str(), "//", "/");	// replace duplicated slashes

	// Replace all .. and . paths.
	std::vector<std::string> segments;
	int segment_count = StringHelper::Split(normalized.c_str(), '/', segments);

	normalized = "";

	for (int i = segment_count - 1; i >= 0; i--)
	{
		std::string seg = segments.at(i);
		if (seg == ".")
		{
			// Skip us.
			continue;
		}
		else if (seg == "..")
		{
			// Skip the directory before us.
			i--;
			continue;
		}
		else
		{
			if (normalized != "")
			{
				seg += "/";
			}
			normalized = seg + normalized;
		}
	}

	return normalized;
}

PackageFileStream* PackageFile::Open_Chunk_Stream(PackageFileChunk* chunk)
{
	// Wait for available stream.
	while (true)
	{
		{
			MutexLock lock(m_package_streams_mutex);

			// Look for free-stream.
			for (std::vector<PackageFileBaseStream*>::iterator iter = m_package_streams.begin(); iter != m_package_streams.end(); iter++)
			{
				PackageFileBaseStream* stream = *iter;
				if (stream->in_use == false)
				{
					return new PackageFileStream(this, chunk, stream);
				}
			}

			// As non are available, see if we can create a new one.
			if (m_package_streams.size() < max_package_streams)
			{
				PackageFileBaseStream* stream = new PackageFileBaseStream();
				stream->base_stream = StreamFactory::Open(m_package_path.c_str(), StreamMode::Read);
				stream->in_use = false;
				m_package_streams.push_back(stream);

				return new PackageFileStream(this, chunk, stream);
			}
		}

		// As we are at max streams, lets just wait a bit until one is available.
		Platform::Get()->Sleep(1.0f);
	}

	// Shouldn't get to here.
	DBG_ASSERT(false);
}

PackageFileStream* PackageFile::Open_Chunk_Stream(const char* url)
{	
	// Normalize chunk name.
	std::string normalised_name = StringHelper::Lowercase(Normalize_Chunk_Path(url).c_str());

	// Find chunk in package file.
	PackageFileChunk* chunk = m_chunk_hash_table.Get(StringHelper::Hash(normalised_name.c_str()));
	if (chunk == NULL)
	{
		//DBG_LOG("Failed to open stream to chunk '%s'.", url);
		return NULL;
	}

	return Open_Chunk_Stream(chunk);
}

unsigned int PackageFile::Add_String(const char* value)
{
	int string_len = strlen(value) + 1; // Include null-terminator.
	int required_space = string_len + sizeof(PackageFileString);

	// Look for pre-existing string.
	for (unsigned int offset = 0; offset < m_header.chunk_strings_size; offset++)
	{
		PackageFileString* string = reinterpret_cast<PackageFileString*>(m_header.chunk_strings + offset);
		char* name = (char*)string + sizeof(PackageFileString);
		int len = strlen(name);

		if (strcmp(name, value) == 0)
		{
			return offset;
		}

		offset += sizeof(PackageFileString) + len;
	}

	// Extend the buffer.
	char* new_chunk_strings = new char[m_header.chunk_strings_size + required_space];
	if (m_header.chunk_strings != NULL)
	{
		memcpy(new_chunk_strings, m_header.chunk_strings, m_header.chunk_strings_size);
	}
	SAFE_DELETE_ARRAY(m_header.chunk_strings);
	m_header.chunk_strings = new_chunk_strings;

	// Add our new string to the mix.
	PackageFileString* string = reinterpret_cast<PackageFileString*>(m_header.chunk_strings + m_header.chunk_strings_size);
	string->hash = StringHelper::Hash(value);

	char* name = (char*)string + sizeof(PackageFileString);
	memcpy(name, value, string_len);

	unsigned int offset = m_header.chunk_strings_size;
	m_header.chunk_strings_size += required_space;

	return offset;
}

unsigned int PackageFile::Get_File_Checksum(const char* url)
{
	CRC32DataTransformer transformer;

	// Calculate file checksum in chunks.
	Stream* stream = StreamFactory::Open(url, StreamMode::Read);
	if (stream == NULL)
	{
		DBG_LOG("[ERROR] Failed to open file: %s", url);
		_exit(1);
		return 0;
	}
	
	unsigned int buffer_size = 2 * 1024 * 1024;
	char* buffer = new char[buffer_size];

	while (stream->Bytes_Remaining())
	{
		int to_read = Min(buffer_size, stream->Bytes_Remaining());
		stream->ReadBuffer(buffer, 0, to_read);
		transformer.Add(buffer, to_read);
	}

	stream->Close();
	SAFE_DELETE(stream);
	SAFE_DELETE_ARRAY(buffer);
	
	int output_size;
	unsigned int* checksum = (unsigned int*)transformer.Get_Result(output_size);
	return *checksum;
}

void PackageFile::Add_Chunk(const char* source_file, const char* package_location, PackageFileChunkType::Type type, bool force_uncompressed, const char* priority, bool has_changed)//, int checksum)
{
	std::string normalized_name = Normalize_Chunk_Path(package_location);

	std::string compressed_file = StringHelper::Format("%s/%08x.checksum", m_tmp_dir.c_str(), StringHelper::Hash(source_file));
	bool old_data_exists = Platform::Get()->Is_File(compressed_file.c_str());

	//has_changed = true;

	int checksum = 0;

	// calculate checksum or get old one if file hasn't changed.
	if (old_data_exists == false || has_changed == true)
	{
		checksum = Get_File_Checksum(source_file);

		Stream* checksum_stream = StreamFactory::Open(compressed_file.c_str(), StreamMode::Write);
		checksum_stream->Write<int>(checksum);
		SAFE_DELETE(checksum_stream);
	}
	else
	{
		Stream* checksum_stream = StreamFactory::Open(compressed_file.c_str(), StreamMode::Read);
		checksum = checksum_stream->Read<int>();
		SAFE_DELETE(checksum_stream);
	}

	PackageFileChunk chunk;
	chunk.type				= type;
	chunk.name_offset		= Add_String(normalized_name.c_str());
	chunk.priority_offset	= Add_String(priority);
	chunk.checksum			= checksum;
	chunk.compressed		= !force_uncompressed && m_use_compression == true;

	// Check to see if a chunk with this checksum already exists, if so
	// we use that instead to dedupe the data.
	bool deduplicated = false;

	for (unsigned int i = 0; i < m_header.chunk_count; i++)
	{
		PackageFileChunk* c = &m_header.chunks[i];
		if (c->checksum == chunk.checksum)
		{
			chunk.offset = c->offset;
			chunk.compressed = c->compressed;
			chunk.compressed_size = c->compressed_size;
			chunk.uncompressed_size = c->uncompressed_size;

			//DBG_LOG("De-Duplicating chunk, data with same checksum already exists: %s", source_file);

			deduplicated = true;
			break;
		}
	}

	if (deduplicated == false)
	{
		Add_File_Data(source_file, chunk.offset, chunk.uncompressed_size, chunk.compressed_size, chunk.compressed != 0, has_changed);
	}

	// Extend the buffer.
	char* new_chunks = new char[(m_header.chunk_count + 1) * sizeof(PackageFileChunk)];
	if (m_header.chunks != NULL)
	{
		memcpy(new_chunks, m_header.chunks, m_header.chunk_count * sizeof(PackageFileChunk));
	}
	SAFE_DELETE_ARRAY(m_header.chunks);

	// Stick new chunk in buffer.
	m_header.chunks = reinterpret_cast<PackageFileChunk*>(new_chunks);
	m_header.chunks[m_header.chunk_count] = chunk;
	m_header.chunk_count++;
}

void PackageFile::Add_File_Data(const char* source, unsigned int& offset, unsigned int& uncompressed_size, unsigned int& compressed_size, bool should_compress, bool has_changed)
{
	// Write-Out compressed data.
	std::string compressed_file = StringHelper::Format("%s/%08x.compressed", m_tmp_dir.c_str(), StringHelper::Hash(source));
	bool old_data_exists = Platform::Get()->Is_File(compressed_file.c_str());

	Stream* source_stream = StreamFactory::Open(source, StreamMode::Read);
	int length = source_stream->Bytes_Remaining();

	if (has_changed == true || old_data_exists == false)
	{
		Stream* out_stream = StreamFactory::Open(compressed_file.c_str(), StreamMode::Write);

		// Dickbutt compression.
		if (should_compress == false)
		{
			source_stream->CopyTo(out_stream);

			uncompressed_size = length;
			compressed_size = length;
		}
		else
		{
			// We compress in chunks, so we can stream data in, less efficient for disk-space
			// more efficient for RAM.
			int chunk_offset = 0;
			DataBuffer uncompressed_chunk;
			DataBuffer compressed_chunk;
		
			uncompressed_size = length;
			compressed_size = 0;

			//DBG_LOG("Compressing: %s", source);

			while (chunk_offset < length)
			{
				int bytes_left = length - chunk_offset;
				int chunk_size = Min(compression_chunk_size, bytes_left);

				uncompressed_chunk.Reserve(chunk_size, false);
				source_stream->ReadBuffer(uncompressed_chunk.Buffer(), 0, chunk_size);
				uncompressed_chunk.Compress(compressed_chunk);

				out_stream->Write<int>(compressed_chunk.Size());
				out_stream->WriteBuffer(compressed_chunk.Buffer(), 0, compressed_chunk.Size());

				compressed_size += compressed_chunk.Size();
				compressed_size += sizeof(int);
		
				chunk_offset += chunk_size;
			}
		}

		out_stream->Close();
		SAFE_DELETE(out_stream);
	}
	else
	{
		if (should_compress == false)
		{
			uncompressed_size = length;
			compressed_size = length;
		}
		else
		{
			Stream* out_stream = StreamFactory::Open(compressed_file.c_str(), StreamMode::Read);

			uncompressed_size = length;
			compressed_size = out_stream->Length();

			out_stream->Close();
			SAFE_DELETE(out_stream);
		}
	}

	source_stream->Close();
	SAFE_DELETE(source_stream);

	// We just keep a log of chunk source files for when we put packages together, no need to save them in the file.
	offset = m_header.chunk_data_size;
	m_header.chunk_data_size += compressed_size;

	m_chunk_source_files.push_back(compressed_file);
}

void PackageFile::Write_Patched_Header(const char* path)
{
	Stream* stream = StreamFactory::Open(path, (StreamMode::Type)((int)StreamMode::Read|(int)StreamMode::Write));
	stream->WriteBuffer((char*)&m_header, 0, sizeof(PackageFileHeader));
	SAFE_DELETE(stream);
}

void PackageFile::Write_To_Disk(const char* path)
{
	BinaryStream signed_data_stream;

	// Generate a new guid.
	std::string guid = Platform::Get()->Generate_UUID();
	m_header.guid[0] = '\0';
	strcpy(m_header.guid, guid.c_str());

	// Write in all the chunk strings.
	signed_data_stream.WriteBuffer(m_header.chunk_strings, 0, m_header.chunk_strings_size);

	// Write in all the chunks.
	signed_data_stream.WriteBuffer((char*)m_header.chunks, 0, m_header.chunk_count * sizeof(PackageFileChunk));

	DBG_LOG("Expected String Size: %ikb", m_header.chunk_strings_size / 1024);
	DBG_LOG("Expected Chunk Size: %ikb", (m_header.chunk_count * sizeof(PackageFileChunk)) / 1024);
	DBG_LOG("Expected Data Size: %imb", m_header.chunk_data_size / 1024 / 1024);

	// Write in all the chunk data.
	unsigned int buffer_size = 128 * 1024;
	char* buffer = new char[buffer_size];
	for (std::vector<std::string>::iterator iter = m_chunk_source_files.begin(); iter != m_chunk_source_files.end(); iter++)
	{
		std::string source = *iter;

		// Calculate file checksum in chunks.
		Stream* source_stream = StreamFactory::Open(source.c_str(), StreamMode::Read);

		while (source_stream->Bytes_Remaining())
		{
			int to_read = Min(buffer_size, source_stream->Bytes_Remaining());
			source_stream->ReadBuffer(buffer, 0, to_read);
			signed_data_stream.WriteBuffer(buffer, 0, to_read);
		}

		source_stream->Close();
		SAFE_DELETE(source_stream);
	}
	SAFE_DELETE_ARRAY(buffer);

	char* signed_data = signed_data_stream.Take_Data();
	int signed_data_len = signed_data_stream.Length();
	signed_data_stream.Close();

	// Write the actual data out.
	Stream* stream = StreamFactory::Open(path, StreamMode::Write);

	// If we have a private key, sign the data.
	if (!m_private_key.SameAs(CryptoKey::Empty))
	{
		CryptoStream* crypto_stream = new CryptoStream(CryptoStreamMode::Sign, m_private_key);
		crypto_stream->WriteBuffer(signed_data, 0, signed_data_len);
		crypto_stream->GetSignature(m_header.signature, PackageFileHeader::SIGNATURE_LENGTH);
		SAFE_DELETE(crypto_stream);
	}
	else
	{
		memset(m_header.signature, 0, PackageFileHeader::SIGNATURE_LENGTH);
	}

	// Write in the header.
	stream->WriteBuffer((char*)&m_header, 0, sizeof(PackageFileHeader));

	// Write in the signed data.
	stream->WriteBuffer(signed_data, 0, signed_data_len);

	stream->Close();
	SAFE_DELETE(stream);

	SAFE_DELETE(signed_data);
}

void PackageFile::Write_Manifest_To_Disk(const char* path)
{
	Stream* stream = StreamFactory::Open(path, StreamMode::Write);
	stream->WriteLine(StringHelper::Format("[ Manifest: %s ]", path).c_str());
	stream->WriteLine(StringHelper::Format("String Size: %.2f kb", m_header.chunk_strings_size/1024.0f).c_str());
	stream->WriteLine(StringHelper::Format("Chunk Size: %.2f kb", (m_header.chunk_count * sizeof(PackageFileChunk))/1024.0f).c_str());
	stream->WriteLine(StringHelper::Format("Data Size: %.2f mb", m_header.chunk_data_size/1024.0f/1024.0f).c_str());

	int offset =
		sizeof(PackageFileHeader) +
		m_header.chunk_strings_size +
		(m_header.chunk_count * sizeof(PackageFileChunk));

	PackageFileChunk* header = m_header.chunks;

	for (std::vector<std::string>::iterator iter = m_chunk_source_files.begin(); iter != m_chunk_source_files.end(); iter++)
	{
		std::string source = *iter;

		Stream* source_stream = StreamFactory::Open(source.c_str(), StreamMode::Read);

		stream->WriteLine(StringHelper::Format("[Size:%i] Source:%s Compressed:%s", 
			source_stream->Length(), 
			Get_String(header->name_offset), 
			source.c_str()
			).c_str());

		offset += source_stream->Length();
		header++;

		source_stream->Close();
		SAFE_DELETE(source_stream);
	}

	stream->Close();
	SAFE_DELETE(stream);
}