// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_DATA_DATATRANSFORMER_
#define _GENERIC_DATA_DATATRANSFORMER_

#include "Generic/Traits/IsPointer.h"

class DataTransformer
{
	MEMORY_ALLOCATOR(DataTransformer, "Generic");

private:
		
	template<typename T>
	T Calculate_Internal(TrueTraitType type, void* data, int size)
	{
		int output_size;
		void* result = Calculate(data, size, output_size);
		return reinterpret_cast<T*>(result);
	}

	template<typename T>
	T Calculate_Internal(FalseTraitType type, void* data, int size)
	{
		int output_size;
		void* result = Calculate(data, size, output_size);
		DBG_ASSERT(output_size == sizeof(T));
		return *reinterpret_cast<T*>(result);
	}
	
public:

	// Required overrides.
	virtual void  Reset() = 0;
	virtual void  Add(void* data, int size) = 0;
	virtual void* Get_Result(int& output_size) = 0;

	// Helper methods.
	void* Calculate(void* data, int size, int& output_size)
	{
		Reset();
		Add(data, size);
		return Get_Result(output_size);
	}
	
	template<typename T>
	T Calculate(void* data, int size)
	{
		return Calculate_Internal<T>(IsPointer<T>::Type, data, size);
	}

};

#endif
