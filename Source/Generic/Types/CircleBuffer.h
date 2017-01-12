// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_CIRCLEBUFFER_
#define _GENERIC_CIRCLEBUFFER_

#include "Generic/Math/Math.h"

template <typename T, int default_capacity = 2>
class CircleBuffer
{
	MEMORY_ALLOCATOR(CircleBuffer<T COMMA default_capacity>, "Data Types");

private:
	T* m_buffer;
	int m_offset;
	int m_length;
	int m_capacity;

public:
	CircleBuffer()
		: m_length(0)
		, m_offset(0)
		, m_buffer(NULL)
		, m_capacity(0)
	{
		Resize(default_capacity);
	}

	CircleBuffer(CircleBuffer& buffer)
	{
		m_length = buffer.m_length;
		m_offset = buffer.m_offset;
		m_capacity = buffer.m_capacity;
		m_buffer = new T[buffer.m_capacity];
		memcpy(m_buffer, buffer.m_buffer, buffer.m_capacity * sizeof(T));
	}

	CircleBuffer& operator=( const CircleBuffer& buffer ) 
	{
		m_length = buffer.m_length;
		m_offset = buffer.m_offset;
		m_capacity = buffer.m_capacity;
		m_buffer = new T[buffer.m_capacity];
		memcpy(m_buffer, buffer.m_buffer, buffer.m_capacity * sizeof(T));

		return *this;
	}

	~CircleBuffer()
	{
		SAFE_DELETE_ARRAY(m_buffer);
	}

	void Resize(int size)
	{
		T* new_buffer = new T[size];

		if (m_buffer != NULL)
		{
			for (int i = 0; i < Min(m_capacity, size); i++)
			{
				new_buffer[i] = m_buffer[i];
			}
			SAFE_DELETE_ARRAY(m_buffer);
		}

		m_buffer = new_buffer;
		m_capacity = size;
	}

	void Push(const T& value)
	{
		m_buffer[m_offset] = value;
		m_offset = (m_offset + 1) % m_capacity;
		m_length = Min(m_capacity, m_length + 1);
	}

	T& Peek(int off = 0)
	{
		int offset = m_offset - (1 + off);
		if (offset < 0)
		{
			offset = m_capacity + offset;
		}
		return m_buffer[offset];
	}

	T& Get(int index)
	{
		return m_buffer[index % m_capacity];
	}

	int Size()
	{
		return m_length;
	}

	int Capacity()
	{
		return m_capacity;
	}

};

#endif