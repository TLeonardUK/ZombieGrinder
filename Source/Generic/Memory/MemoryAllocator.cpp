// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Memory/MemoryAllocator.h"
#include "Generic/Memory/MemoryManager.h"

#include <cstring>
#include <stdlib.h>
#include <stdio.h>

MemoryAllocator::MemoryAllocator(const char* class_name, const char* category)
	: m_class_name(class_name)
	, m_category(category)
	, m_bytes_allocated(0)
{
	MemoryManager::Register_Allocator(this);
//	DBG_LOG("Initializing new memory allocator for class '%s' in category '%s'.", class_name, category);
}

MemoryAllocator::~MemoryAllocator()
{
	// Derp, this should never actually get called.
}

void* MemoryAllocator::Alloc(size_t size)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	void* new_ptr = platform_malloc(size);
	int new_size = platform_memory_size(new_ptr);

	m_bytes_allocated += new_size;
	
	// Well thats not right.
	if (m_bytes_allocated < 0)
	{
	//	TODO: Fix, atomic acess otherwise threads will get fuckzored.
	//	DBG_ASSERT(false);
	}

	//if (size > 1024 * 1024)
	//{
	//	DBG_LOG("LARGE ALLOC: %.2fmb", size / 1024.0f / 1024.0f);
	//}

	DBG_ASSERT(new_ptr != NULL);

	return new_ptr;
#else
	return malloc(size);
#endif
}

void* MemoryAllocator::Realloc(void* ptr, size_t size)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	int original_size = platform_memory_size(ptr);
	void* new_ptr = platform_realloc(ptr, size);
	int new_size = platform_memory_size(new_ptr);

	m_bytes_allocated += (new_size - original_size);
	
	// Well thats not right.
	if (m_bytes_allocated < 0)
	{
	//	TODO: Fix, atomic acess otherwise threads will get fuckzored.
	//	DBG_ASSERT(false);
	}

	DBG_ASSERT(new_ptr != NULL);

	return new_ptr;
#else
	return realloc(ptr, size);
#endif
}

void MemoryAllocator::Free(void* ptr)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	int size = platform_memory_size(ptr);
	m_bytes_allocated -= size;

	// Well thats not right.
	if (m_bytes_allocated < 0)
	{
	//	TODO: Fix, atomic acess otherwise threads will get fuckzored.
	//	DBG_ASSERT(false);
	}

	//if (size > 1024 * 1024)
	//{
	//	DBG_LOG("LARGE FREE: %.2fmb", size / 1024.0f / 1024.0f);
	//}

	return platform_free(ptr);
#else
	free(ptr);
#endif
}

int MemoryAllocator::Get_Bytes_Allocated()
{
	return m_bytes_allocated;
}

const char* MemoryAllocator::Get_Class_Name()
{
	return m_class_name;
}

const char* MemoryAllocator::Get_Category()
{
	return m_category;
}