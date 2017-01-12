// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Memory/MemoryManager.h"
#include "Generic/Memory/MemoryAllocator.h"

#include "Generic/Types/PooledMemoryHeap.h"

#include <algorithm>
#include <cstring>
#include <vector>

std::vector<MemoryAllocator*> g_allocators;
MemoryAllocator g_system_allocator("System", "System");

struct MemoryCategoryStats
{
	int bytes_allocated;
	const char* category;
};

bool Sort_By_Memory_Allocator(MemoryAllocator* i, MemoryAllocator* j) 
{ 
	return j->Get_Bytes_Allocated() < i->Get_Bytes_Allocated(); 
}

bool Sort_By_Category_Stats(MemoryCategoryStats i, MemoryCategoryStats j) 
{ 
	return j.bytes_allocated < i.bytes_allocated; 
}

void MemoryManager::Register_Allocator(MemoryAllocator* allocator)
{
	g_allocators.push_back(allocator);
};

bool MemoryManager::Init()
{
	// Malloc pool needs to be reentrant from here on out!
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	platform_get_malloc_heap()->Become_Reentrant();
#endif
	return true;
}

MemoryAllocator* MemoryManager::Get_System_Allocator()
{
	return &g_system_allocator;
}

void MemoryManager::Dump_Stats()
{
	std::vector<MemoryCategoryStats> category_stats;
	int total_bytes = 0;

	// Total up all category stats.
	for (std::vector<MemoryAllocator*>::iterator iter = g_allocators.begin(); iter != g_allocators.end(); iter++)
	{
		MemoryAllocator* allocator = *iter;
		const char* category = allocator->Get_Category();

		total_bytes += allocator->Get_Bytes_Allocated();

		bool found = false;

		for (std::vector<MemoryCategoryStats>::iterator iter2 = category_stats.begin(); iter2 != category_stats.end(); iter2++)
		{
			MemoryCategoryStats& stats = *iter2;
			if (stricmp(category, stats.category) == 0)
			{
				stats.bytes_allocated += allocator->Get_Bytes_Allocated();
				found = true;
				break;
			}
		}

		if (found == false)
		{
			MemoryCategoryStats stats;
			stats.category = category;
			stats.bytes_allocated = allocator->Get_Bytes_Allocated();

			category_stats.push_back(stats);
		}
	}	

	// Sort from highest to lowest.
	std::sort(category_stats.begin(), category_stats.end(), Sort_By_Category_Stats);
	
	// Dump out totals.
	DBG_LOG("==== MEMORY STATS ====");
	DBG_LOG("Total: %i (%.2fmb)", total_bytes, float(total_bytes) / 1024.0f / 1024.0f);
	DBG_LOG("");

	DBG_LOG("==== CATEGORY ALLOCATION STATS ====");
	DBG_LOG("%-45s %s", "CATEGORY", "BYTES ALLOCATED");

	// Dump out the informationz.
	for (std::vector<MemoryCategoryStats>::iterator iter = category_stats.begin(); iter != category_stats.end(); iter++)
	{
		MemoryCategoryStats& stats = *iter;
		DBG_LOG("%-45s %i (%.2fmb)", stats.category, stats.bytes_allocated, float(stats.bytes_allocated) / 1024.0f / 1024.0f);
	}
	DBG_LOG("");

	// Dump out class specific allocation stats.
	DBG_LOG("==== CLASS ALLOCATION STATS ====");
	DBG_LOG("%-45s %s", "CLASS", "BYTES ALLOCATED");

	std::sort(g_allocators.begin(), g_allocators.end(), Sort_By_Memory_Allocator);

	for (std::vector<MemoryAllocator*>::iterator iter = g_allocators.begin(); iter != g_allocators.end(); iter++)
	{
		MemoryAllocator* allocator = *iter;
		DBG_LOG("%-45s %i (%.2fmb)", allocator->Get_Class_Name(), allocator->Get_Bytes_Allocated(), float(allocator->Get_Bytes_Allocated()) / 1024.0f / 1024.0f);
	}
	DBG_LOG("");
}