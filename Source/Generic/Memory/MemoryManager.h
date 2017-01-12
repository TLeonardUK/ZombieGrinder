// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MEMORY_MANAGER_
#define _GENERIC_MEMORY_MANAGER_

// THis is included in defines.h, so lets slim it down.
#include "Generic/Memory/MemoryAllocator.h"

//#include <typeinfo>
//#include <stddef.h>
//#include <new>

class MemoryManager
{
private:
	friend class MemoryAllocator;

	static void Register_Allocator(MemoryAllocator* allocator);

public:
	static bool Init();
	static void Dump_Stats();

	static MemoryAllocator* Get_System_Allocator();

	template <typename class_type>
	static MemoryAllocator* Get_Allocator(const char* category_name)
	{
		static MemoryAllocator allocator = MemoryAllocator(typeid(class_type).name(), category_name);	
		return &allocator;
	}

};

#endif
