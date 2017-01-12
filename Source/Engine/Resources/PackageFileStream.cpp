// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Resources/PackageFileStream.h"
#include "Engine/Resources/PackageFile.h"

PackageFileStream::PackageFileStream(PackageFile* file, PackageFileChunk* chunk, PackageFileBaseStream* base_stream)
	: m_package_file(file)
	, m_package_chunk(chunk)
	, m_base_stream(base_stream)
	, m_compressed(chunk->compressed != 0)
{
	m_chunk_position = file->Get_Header()->chunk_data_offset + chunk->offset;
	m_chunk_size = chunk->uncompressed_size;

	m_base_stream->in_use = true;
	m_base_stream->base_stream->Seek(m_chunk_position);

	if (chunk->compressed != 0)
	{
		m_position = 0;
		m_decompress_offset = 0;
		m_decompress_buffer.Reserve(0, false);
	}
}

PackageFileStream::~PackageFileStream()
{
	m_base_stream->in_use = false;
}

bool PackageFileStream::IsEOF()
{
	return Position() >= Length();
}

unsigned int PackageFileStream::Position()
{
	if (m_compressed == true)
	{
		return m_position;
	}
	else
	{
		return m_base_stream->base_stream->Position() - m_chunk_position;
	}
}

void PackageFileStream::Seek(unsigned int offset)
{
	DBG_ASSERT(offset >= 0 && offset <= m_chunk_size);
	if (m_compressed == true)
	{
		// Gross. We have to go to the start of the stream and seek
		// to the compressed chunk that contains our data. Maybe add
		// some kinda lookup table for this?

		m_base_stream->base_stream->Seek(m_chunk_position); 

		m_position = 0;
		m_decompress_offset = 0;
		m_decompress_buffer.Reserve(0, false);

		ReadBuffer(NULL, 0, offset);
	}
	else
	{
		m_base_stream->base_stream->Seek(m_chunk_position + offset); 
	}
}

unsigned int PackageFileStream::Length()
{
	return m_chunk_size;
}

void PackageFileStream::WriteBuffer(const char* buffer, int offset, int length)
{
	DBG_ASSERT_STR(false, "Cannot write into package file stream! Packages are read-only.");
}

void PackageFileStream::ReadBuffer(char* buffer, int offset, int length)
{
	unsigned int final_position = Position() + length;
	DBG_ASSERT_STR(final_position <= m_chunk_position + m_chunk_size, "Attempt to read beyond chunk data in package.")
		
	if (m_compressed == true)
	{
		int read_offset = 0;

		while (read_offset < length)
		{
			int read_left = (length - read_offset);
			int bytes_in_chunk = m_decompress_buffer.Size() - m_decompress_offset;

			// Get the next compressed chunk.
			if (bytes_in_chunk <= 0)
			{
				// Read in size of chunk.
				DataBuffer uncompressed;
				uncompressed.Reserve(sizeof(int));

				int compressed_size = m_base_stream->base_stream->Read<int>();

				uncompressed.Reserve(compressed_size, true);
				m_base_stream->base_stream->ReadBuffer(uncompressed.Buffer(), 0, compressed_size);
				
				uncompressed.Decompress(m_decompress_buffer);

				//DBG_LOG("Read new compressed chunk of size %i/%i.", compressed_size, m_decompress_buffer.Size());

				m_decompress_offset = 0;
			}

			// Read in data from chunk.
			else
			{
				int can_read = Min(read_left, bytes_in_chunk);

				if (buffer != NULL)
				{
					memcpy(buffer + read_offset, m_decompress_buffer.Buffer() + m_decompress_offset, can_read);
				}

				//DBG_LOG("Read %i bytes from compressed chunk (chunk-offset:%i position:%i).", can_read, m_decompress_offset, m_position);

				m_decompress_offset += can_read;
				read_offset += can_read;
			}
		}

		m_position += length;
	}
	else
	{
		m_base_stream->base_stream->ReadBuffer(buffer, offset, length);
	}
}

void PackageFileStream::Close()
{
	// Do nothing here. We release base stream on destruction.
}

void PackageFileStream::Flush()
{
	// Nothing to do, we can't write to this stream!
}
