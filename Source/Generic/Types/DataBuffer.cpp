// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/DataBuffer.h"

#include "zlib/zlib.h"
	
DataBuffer::DataBuffer()
{
	m_buffer = NULL;//new char[initial_size];
	m_size = initial_size;
}

DataBuffer::DataBuffer(const DataBuffer& other)
	: m_size(initial_size)
	, m_buffer(NULL)
{
	Set(other.m_buffer, other.m_size);
}

DataBuffer::DataBuffer(const char* data, int size)
	: m_size(0)
	, m_buffer(NULL)
{
	Set(data, size);
}

void DataBuffer::operator=(const DataBuffer &other)
{ 
	Set(other.m_buffer, other.m_size);
}

DataBuffer::~DataBuffer()
{
	if (m_buffer)
		Get_Allocator()->Free(m_buffer);
}

int DataBuffer::Size()
{
	return m_size;
}
	
char* DataBuffer::Buffer()
{
	return m_buffer;
}

void DataBuffer::Set(const char* data, int size)
{
	if (m_buffer != NULL)
	{
		Get_Allocator()->Free(m_buffer);
	}

	m_buffer = (char*)Get_Allocator()->Alloc(size);
	m_size = size;

	if (data != NULL)
		memcpy(m_buffer, data, size);
}

void DataBuffer::Reserve(int size, bool preserve_data)
{
	int new_size = size;

	//while (size > new_size)
	//{
	//	new_size *= 2;
	//}

	if (m_size != new_size || m_buffer == NULL)
	{
		char* new_buffer = (char*)Get_Allocator()->Alloc(new_size);

		if (preserve_data == true && m_buffer != NULL)
		{
			memcpy(new_buffer, m_buffer, Min(m_size, size));
		}
		
		if (m_buffer != NULL)
		{
			Get_Allocator()->Free(m_buffer);
		}

		m_buffer = new_buffer;
		m_size = new_size;
	}
}

void DataBuffer::Compress(DataBuffer& output, bool bBestSpeed)
{
	uLongf size = Size();
	uLongf out_size = size + size / 10 + 32;

	output.Reserve(out_size, false);
		
	int ret = compress2((Bytef*)output.m_buffer + 4, &out_size, (Bytef*)m_buffer, size, bBestSpeed ? Z_BEST_SPEED : Z_BEST_COMPRESSION);
	DBG_ASSERT(ret == Z_OK); 

	*reinterpret_cast<int*>(output.m_buffer) = (int)size;
	output.Reserve(out_size + 4, true);
}

void DataBuffer::Decompress(DataBuffer& output)
{
	uLongf out_size = (uLongf)*reinterpret_cast<int*>(m_buffer);

	output.Reserve(out_size, false);

	int ret = uncompress((Bytef*)output.m_buffer, &out_size, (Bytef*)m_buffer + 4, m_size - 4);
	DBG_ASSERT(ret == Z_OK);
}

int DataBuffer::Get_Compressed_Size()
{
	return *reinterpret_cast<int*>(m_buffer);
}

DataBuffer DataBuffer::Delta_Encode(DataBuffer& reference)
{
	DataBuffer output;
	output.Reserve(m_size, false);

	for (int i = 0; i < m_size; i++)
	{
		int src_value = m_buffer[i];
		int ref_value = 0;

		if (i < reference.Size())
		{
			ref_value = reference.m_buffer[i];
		}

		output.m_buffer[i] = src_value ^ ref_value;
	}

	return output;
}

void DataBuffer::Delta_Encode_In_Place(DataBuffer& reference)
{
	for (int i = 0; i < m_size; i++)
	{
		int src_value = m_buffer[i];
		int ref_value = 0;

		if (i < reference.Size())
		{
			ref_value = reference.m_buffer[i];
		}

		m_buffer[i] = src_value ^ ref_value;
	}
}

DataBuffer DataBuffer::Delta_Decode(DataBuffer& reference)
{
	return Delta_Encode(reference);
}

void DataBuffer::Delta_Decode_In_Place(DataBuffer& reference)
{
	Delta_Encode_In_Place(reference);
}

