// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/IO/PatchedBinaryStream.h"
#include "Engine/IO/StreamFactory.h"

PatchedBinaryStream::PatchedBinaryStream(const char* file_url, StreamMode::Type mode)
{
	m_output_stream = StreamFactory::Open(file_url, mode);
	DBG_ASSERT(m_output_stream != NULL);
	m_mode = mode;
	m_owns_output_stream = true;

	if ((m_mode & StreamMode::Read) != 0)
	{
		Read_Data();
	}
}

PatchedBinaryStream::PatchedBinaryStream(Stream* output_stream, StreamMode::Type mode)
{
	m_output_stream = output_stream;
	m_mode = mode;
	m_owns_output_stream = false;

	if ((m_mode & StreamMode::Read) != 0)
	{
		Read_Data();
	}
}

PatchedBinaryStream::PatchedBinaryStream()
{
	m_output_stream = NULL;
	m_mode = StreamMode::Write;
	m_owns_output_stream = true;
}

PatchedBinaryStream::~PatchedBinaryStream()
{
	Close();

	if (m_owns_output_stream == true)
	{
		SAFE_DELETE(m_output_stream);
	}
}

void PatchedBinaryStream::Read_Data()
{
	//DBG_LOG("Reading patched binary stream (0x%08x)", m_output_stream);

	u32 patch_count = m_output_stream->Read<u32>();

	m_patches.clear();
	m_patches.reserve(patch_count);

	u32* patches = new u32[patch_count * 2];
	m_output_stream->ReadBuffer((char*)patches, 0, sizeof(u32) * patch_count * 2);

	for (unsigned int i = 0; i < patch_count; i++)
	{
		PatchedBinaryStreamPatch patch;
#ifdef PLATFORM_LITTLE_ENDIAN
		patch.dest_offset = patches[i * 2];//m_output_stream->Read<u32>();
		patch.source_offset = patches[(i * 2) + 1];//m_output_stream->Read<u32>();
#else
		patch.dest_offset = Swap_Endian(&patches[i * 2]);//m_output_stream->Read<u32>();
		patch.source_offset = Swap_Endian(&patches[(i * 2) + 1]);//m_output_stream->Read<u32>();
#endif 
		m_patches.push_back(patch);
	}

	m_data_stream.Reserve_Exactly(m_output_stream->Bytes_Remaining());
	m_output_stream->CopyTo(&m_data_stream);

	//DBG_LOG("Patching binary data (%i patches) ...", patch_count);
	char* data = m_data_stream.Data();
	for (std::vector<PatchedBinaryStreamPatch>::iterator iter = m_patches.begin(); iter != m_patches.end(); iter++)
	{
		PatchedBinaryStreamPatch& patch = *iter;

		DBG_ASSERT(patch.source_offset >= 0 && patch.source_offset < m_data_stream.Length());
		DBG_ASSERT(patch.dest_offset >= 0 && patch.dest_offset < m_data_stream.Length());

		char** patch_src = reinterpret_cast<char**>(data + patch.source_offset);
		char* patch_dest = (char*)(data + patch.dest_offset);

		if (patch.dest_offset == 0)
		{
			patch_dest = NULL;
		}
	
		(*patch_src) = patch_dest;
	}

	m_patches.clear();
}

char* PatchedBinaryStream::Get_Data()
{
	return m_data_stream.Data();
}

char* PatchedBinaryStream::Take_Data()
{
	return m_data_stream.Take_Data();
}

bool PatchedBinaryStream::IsEOF()
{
	return m_data_stream.IsEOF();
}

unsigned int PatchedBinaryStream::Position()
{
	return m_data_stream.Position();
}

void PatchedBinaryStream::Seek(unsigned int offset)
{
	return m_data_stream.Seek(offset);
}

unsigned int PatchedBinaryStream::Length()
{
	return m_data_stream.Length();
}

void PatchedBinaryStream::WriteBuffer(const char* buffer, int offset, int length)
{
	m_data_stream.WriteBuffer(buffer, offset, length);
}

void PatchedBinaryStream::ReadBuffer(char* buffer, int offset, int length)
{
	m_data_stream.ReadBuffer(buffer, offset, length);
}

void PatchedBinaryStream::Close()
{
	if (m_output_stream != NULL && 
		(m_mode & StreamMode::Write) != 0)
	{
		m_output_stream->Write<u32>(m_patches.size());
		for (std::vector<PatchedBinaryStreamPatch>::iterator iter = m_patches.begin(); iter != m_patches.end(); iter++)
		{
			PatchedBinaryStreamPatch& patch = *iter;
			m_output_stream->Write<u32>(patch.dest_offset);
			m_output_stream->Write<u32>(patch.source_offset);
		}

		m_data_stream.Seek(0);
		//m_data_stream.Reserve_Exactly(m_output_stream->Bytes_Remaining());
		m_data_stream.CopyTo(m_output_stream);

		m_output_stream->Close();
		m_output_stream = NULL;
	}
}

void PatchedBinaryStream::Align_To_Pointer()
{
	int ptr_size   = sizeof(int*);
	int length     = m_data_stream.Length();
	int length_mod = (length % ptr_size);
	int align_mod  = length_mod > 0 ? ptr_size - length_mod : 0;
	for (int i = 0; i < align_mod; i++)
	{
		Write<u8>(0);
	}
}

int PatchedBinaryStream::Create_Pointer()
{
	PatchedBinaryStreamPatch patch;
	patch.source_offset = Length();
	patch.dest_offset = 0;
	m_patches.push_back(patch);

	for (int i = 0; i < sizeof(int*); i++)
	{
		Write<u8>(0);
	}

	return m_patches.size() - 1;
}

void PatchedBinaryStream::Patch_Pointer(int ptr_index)
{
	PatchedBinaryStreamPatch& patch = m_patches.at(ptr_index);
	patch.dest_offset = Length();
}

void PatchedBinaryStream::Patch_Pointer(int ptr_index, int dest_ptr_index, bool use_dest)
{
	PatchedBinaryStreamPatch& patch = m_patches.at(ptr_index);
	PatchedBinaryStreamPatch& dest_patch = m_patches.at(dest_ptr_index);

	if (use_dest)
		patch.dest_offset = dest_patch.dest_offset;
	else
		patch.dest_offset = dest_patch.source_offset;
}

void PatchedBinaryStream::Flush()
{
	// Nothing to do here.
}
