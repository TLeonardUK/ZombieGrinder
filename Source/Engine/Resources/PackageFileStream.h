// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PACKAGE_FILE_STREAM_
#define _ENGINE_PACKAGE_FILE_STREAM_

#include "Engine/IO/Stream.h"
#include "Engine/IO/BinaryStream.h"

#include <vector>

class PackageFile;
struct PackageFileChunk;
struct PackageFileBaseStream;

class PackageFileStream : public Stream
{
	MEMORY_ALLOCATOR(PackageFileStream, "IO");

private:
	PackageFileBaseStream* m_base_stream;

	PackageFile* m_package_file;
	PackageFileChunk* m_package_chunk;

	bool m_compressed;

	DataBuffer m_decompress_buffer;
	int m_decompress_offset;

	u32 m_chunk_position;
	u32 m_chunk_size;

	u32 m_position;

protected:
	friend class PackageFile;

	PackageFileStream(PackageFile* file, PackageFileChunk* chunk, PackageFileBaseStream* base_stream);
	~PackageFileStream();

public:

	bool IsEOF();
	unsigned int Position();
	void Seek(unsigned int offset);
	unsigned int Length();
	void WriteBuffer(const char* buffer, int offset, int length);
	void ReadBuffer(char* buffer, int offset, int length);
	void Close();
	void Flush();

};

#endif

