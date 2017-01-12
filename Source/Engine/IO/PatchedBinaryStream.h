// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PATCHED_BINARY_STREAM_
#define _ENGINE_PATCHED_BINARY_STREAM_

#include "Engine/IO/Stream.h"
#include "Engine/IO/BinaryStream.h"

#include <vector>

struct PatchedBinaryStreamPatch
{
public:
	u32 dest_offset;
	u32 source_offset;
};

class PatchedBinaryStream : public Stream
{
	MEMORY_ALLOCATOR(PatchedBinaryStream, "IO");

private:
	BinaryStream m_data_stream;
	Stream* m_output_stream;

	bool m_owns_output_stream;
	StreamMode::Type m_mode;

	std::vector<PatchedBinaryStreamPatch> m_patches;

	void Read_Data();

public:
	PatchedBinaryStream(const char* file_url, StreamMode::Type mode);
	PatchedBinaryStream(Stream* output_stream, StreamMode::Type mode);
	PatchedBinaryStream();
	~PatchedBinaryStream();

	bool IsEOF();
	unsigned int Position();
	void Seek(unsigned int offset);
	unsigned int Length();
	void WriteBuffer(const char* buffer, int offset, int length);
	void ReadBuffer(char* buffer, int offset, int length);
	void Close();
	void Flush();

	char* Get_Data();
	char* Take_Data();

	void Align_To_Pointer();
	int  Create_Pointer();
	void Patch_Pointer(int ptr_index);
	void Patch_Pointer(int ptr_index, int dest_ptr_index, bool use_dest = false);

};

#endif

