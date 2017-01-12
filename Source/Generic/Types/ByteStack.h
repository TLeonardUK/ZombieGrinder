// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_BYTESTACK_
#define _GENERIC_BYTESTACK_

#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector4.h"

#include "Generic/Math/Math.h"

#include <cstring>

// Byte stacks are FIFO. Their memory / head pointer will also keep expanding
// until you call Clear, the head pointer will not decrease as you read values (this is done primarily for speed),
// they should only contain small amounts of data that will usually be read in one chunk (render queue, etc).

// We read from the tail (lowest address), and write to the head (highest address);
//
//	-------------------------------
//	|        TAIL######HEAD       |
//  -------------------------------
//
// If head runs out of space ahead and there is space behind tail, we memcpy the entire thing to the begining to shift everything back.

//#define ENABLE_BYTE_STACK_ASSERTS

class ByteStack
{
	MEMORY_ALLOCATOR(ByteStack, "Data Types");

public:
	char* m_buffer;
	char* m_head;
	char* m_tail;
	int	  m_buffer_size;

public:
	ByteStack(int initial_size)
		: m_buffer(NULL)
		, m_head(NULL)
		, m_tail(NULL)
		, m_buffer_size(0)
	{
		Ensure_Size(initial_size);
	}

	~ByteStack()
	{
		SAFE_DELETE_ARRAY(m_buffer);
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
				if (m_buffer != NULL)
				{
					memcpy(new_buffer, m_buffer + tail_offset, head_offset - tail_offset);
					SAFE_DELETE_ARRAY(m_buffer);
				}

				m_buffer	  = new_buffer;		
				m_buffer_size = new_size;
				m_head		  = m_buffer + head_offset;
				m_tail		  = m_buffer;
			}
		}
	}
	
	INLINE void Ensure_Space(int space)
	{;
		Ensure_Size(space);
	}
	
	INLINE void Write(const char* data, int size)
	{
		Ensure_Size(size);

		memcpy(m_head, data, size);

		m_head += size;
	}

	template<typename T>
	INLINE void Write(T value)
	{
		Ensure_Size(sizeof(T));

		//*reinterpret_cast<T*>(m_head) = value; 
		memcpy(m_head, &value, sizeof(T));

		m_head += sizeof(T);
	}

	template<typename T>
	INLINE void Write_Fast(T value)
	{
		// This is unsafe as we do no bounds checking! We assume
		// the callee has checked available size first!

		//*reinterpret_cast<T*>(m_head) = value; 
		memcpy(m_head, &value, sizeof(T));

		m_head += sizeof(T);
	}

	INLINE void Read(char* output, int size)
	{
#ifdef ENABLE_BYTE_STACK_ASSERTS
		DBG_ASSERT(m_tail + size <= m_head);
#endif
		
		memcpy(output, m_tail, size);

		m_tail += size;
	}

	template<typename T>
	INLINE T Read()
	{
		T result;
		
#ifdef ENABLE_BYTE_STACK_ASSERTS
		DBG_ASSERT(m_tail + sizeof(T) <= m_head);
#endif
		
		//result = *reinterpret_cast<T*>(m_tail); 
		memcpy(&result, m_tail, sizeof(T));

		m_tail += sizeof(T);
		 
		return result;
	}

};

#endif