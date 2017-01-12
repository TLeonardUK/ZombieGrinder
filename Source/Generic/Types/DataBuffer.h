// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_DATABUFFER_
#define _GENERIC_DATABUFFER_

// Just stores a char* buffer of data and manages its size as required.

#include <cstring>

#include "Generic/Math/Math.h"

class DataBuffer
{
	MEMORY_ALLOCATOR(DataBuffer, "Data Buffers");

private:
	char*	m_buffer;
	int		m_size;

	enum
	{
		initial_size = 0
	};

public:
	DataBuffer();
	DataBuffer(const DataBuffer& other);
	DataBuffer(const char* data, int size);

	void operator=(const DataBuffer &other);

	~DataBuffer();

	int Get_Compressed_Size();

	int Size();
	char* Buffer();

	void Set(const char* data, int size);
	void Reserve(int size, bool preserve_data = true);

	void Compress(DataBuffer& output, bool bBestSpeed = false);
	void Decompress(DataBuffer& output);

	DataBuffer Delta_Encode(DataBuffer& reference);
	DataBuffer Delta_Decode(DataBuffer& reference);

	void Delta_Encode_In_Place(DataBuffer& reference);
	void Delta_Decode_In_Place(DataBuffer& reference);
};

#endif