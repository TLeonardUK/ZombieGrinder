/* *****************************************************************

		CVMValueStack.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMVALUESTACK_H_
#define _CVMVALUESTACK_H_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

// =================================================================
//	Simple optimized stack that holds vm value instances.
// =================================================================
template <typename ValueType>
class CVMValueStack
{
private:
	CVirtualMachine* m_vm;
	ValueType* m_data;
	ValueType* m_data_end;
	int m_size;
	ValueType* m_top;
	ValueType* m_watermark;

public:
	CVMValueStack(CVirtualMachine* vm, int size)
		: m_vm(vm)
		, m_size(size)
	{
		m_data = vm->Get_GC()->Alloc_Unmanaged<ValueType>(m_size);
		m_data_end = m_data + m_size;
		m_top = m_data;
		m_watermark = 0;
	}

	~CVMValueStack()
	{
		m_vm->Get_GC()->Free_Unmanaged<ValueType>(m_data);
	}

	INLINE int Available()
	{
		return m_size - Size();
	}

	INLINE void Push_Block(int count)
	{
		DBG_ASSERT(m_top + count <= m_data_end);
		m_top += count;

		if (m_top > m_watermark)
		{
			m_watermark = m_top;
		}

		//DBG_LOG("0x%08x [%i] Push_Block %i", this, Size(), count);
	}

	INLINE void Push(ValueType& value)
	{
		DBG_ASSERT(m_top < m_data_end);
		*m_top = value;
		m_top++;

		if (m_top > m_watermark)
		{
			m_watermark = m_top;
		}
		//DBG_LOG("0x%08x [%i] Push %i", this, Size(), 1);
	}

	INLINE ValueType& Push_And_Return()
	{
		DBG_ASSERT(m_top < m_data_end);
		//DBG_LOG("0x%08x [%i] Push_And_Return %i", this, Size(), 1);
		ValueType* result = (m_top++);

		if (m_top > m_watermark)
		{
			m_watermark = m_top;
		}

		return *result;
	}
	
	INLINE ValueType Pop()
	{
		m_top--;
		DBG_ASSERT(m_top >= m_data);

		ValueType original = *m_top;

		// TODO: We should really clear the pop'd value or we can end up keeping value ref-ptr's indentifitely.
		m_top->Clear();

		//DBG_LOG("0x%08x [%i] Pop %i", this, Size(), 1);

		return original;
	}

	INLINE ValueType Pop_Fast()
	{
		m_top--;
		DBG_ASSERT(m_top >= m_data);
		return *m_top;
	}
	INLINE void Pop_Block(int count)
	{
		m_top -= count;
		
		//DBG_LOG("0x%08x [%i] Pop_Block %i", this, Size(), count);
		DBG_ASSERT(m_top >= m_data);
		
		// TODO: Read comment above.
		for (int i = 0; i < count; i++)
			m_top[i].Clear();
	}

	INLINE int Size()
	{
		return (m_top - m_data);
	}

	INLINE void Clear_To_Watermark()
	{
		for (ValueType* val = m_top; val < m_watermark; val++)
		{
			val->Clear();
		}
		m_watermark = m_top;
	}

	INLINE ValueType& operator[] (const int index)
	{
 		DBG_ASSERT(index >= 0 && index < m_size);
		return m_data[index];
	}

	INLINE ValueType& Peek(int offset = 0)
	{
		offset++;

		DBG_ASSERT(m_top > m_data && m_top - offset >= m_data);
		return *(m_top - offset);
	}

};

#endif