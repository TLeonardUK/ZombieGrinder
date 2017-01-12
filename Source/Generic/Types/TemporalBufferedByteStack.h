// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_TEMPORALBUFFEREDBYTESTACK_
#define _GENERIC_TEMPORALBUFFEREDBYTESTACK_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector4.h"

#include "Generic/Math/Math.h"

// This is essentially the same as ByteStack except data is buffered by a temporal offset. Data written in can only be
// read out after x number of time. This is mainly used for buffering playback of things like voice chat which may be
// choppy if we try to play them immediately as packets are incoming.

class TemporalBufferedByteStack
{
	MEMORY_ALLOCATOR(TemporalBufferedByteStack, "Data Types");

public:
	char*  m_buffer;
	double* m_time_buffer;
	char*  m_head;
	char*  m_tail;
	int	   m_buffer_size;
	double m_temporal_offset;

public:
	TemporalBufferedByteStack(int initial_size, double temporal_offset)
		: m_buffer(NULL)
		, m_time_buffer(NULL)
		, m_head(NULL)
		, m_tail(NULL)
		, m_buffer_size(0)
		, m_temporal_offset(temporal_offset)
	{
		Ensure_Size(initial_size);
	}

	~TemporalBufferedByteStack()
	{
		SAFE_DELETE_ARRAY(m_buffer);
		SAFE_DELETE_ARRAY(m_time_buffer);
	}

	INLINE void Clear()
	{
		m_head = m_buffer;
		m_tail = m_buffer;
	}

	INLINE int Size()
	{
		return m_head - m_tail;
	}

	INLINE int Available(double new_time)
	{
		int amount = 0;
		char* offset = m_tail;
		while (offset != m_head)
		{
			double old_time = m_time_buffer[offset - m_tail];
			double elapsed = new_time - old_time;
			if (elapsed < m_temporal_offset)
			{
				break;
			}
			amount++;
			offset++;
		}
		return amount;
	}

	INLINE void Ensure_Size(int size)
	{
		int head_offset = (m_head - m_buffer);
		int tail_offset = (m_tail - m_buffer);

		int available_infront = (m_buffer_size - head_offset);

		if (available_infront < size || m_buffer == NULL)
		{
			int fragmented_size = tail_offset + available_infront;

			// Is there enough fragmented space (total space behind and in-front of data)? If so shunt everything backward to the start.
			if (m_buffer != NULL && fragmented_size > size)
			{
				for (int i = tail_offset; i < head_offset; i++)
				{
					m_buffer[i - tail_offset] = m_buffer[i];
					m_time_buffer[i - tail_offset] = m_buffer[i];
				}

				m_tail = m_buffer;
				m_head -= tail_offset;
			}

			// Dam, resize time, this will be slow :(
			else
			{
				// Keep sizes as power-of-2. Better than resizing after
				// every few bytes we push on to the stack.
				int new_size = Max(1, m_buffer_size);
				while (new_size < m_buffer_size + size)
				{
					new_size *= 2;
				}

				char* new_buffer = new char[new_size];
				double* new_time_buffer = new double[new_size];
				if (m_buffer != NULL)
				{
					memcpy(new_buffer, m_buffer + tail_offset, head_offset - tail_offset);
					memcpy(new_time_buffer, m_time_buffer + tail_offset, head_offset - tail_offset);
					SAFE_DELETE_ARRAY(m_buffer);
					SAFE_DELETE_ARRAY(m_time_buffer);
				}

				m_buffer	  = new_buffer;	
				m_time_buffer = new_time_buffer;		
				m_buffer_size = new_size;
				m_head		  = m_buffer + head_offset;
				m_tail		  = m_buffer;
			}
		}
	}
	
	INLINE void Ensure_Space(int space)
	{
		Ensure_Size(space);
	}
	
	INLINE void Write(const char* data, int size, double time)
	{
		Ensure_Size(size);

		memcpy(m_head, data, size);

		for (int i = 0; i < size; i++)
		{
			*(m_time_buffer + (m_head - m_buffer) + i) = time;
		}

		m_head += size;
	}

	INLINE bool IsValid(const char* ptr)
	{
		return (ptr >= m_buffer && ptr < m_buffer + m_buffer_size);
	}

	INLINE char* GetBuffer(int size, double time)
	{
		Ensure_Size(size);

		char* start = m_head;

		for (int i = 0; i < size; i++)
		{
			*(m_time_buffer + (m_head - m_buffer) + i) = time;
		}

		m_head += size;

		return start;
	}

	INLINE void Read(char* output, int size)
	{
		memcpy(output, m_tail, size);

		m_tail += size;
	}

};

#endif