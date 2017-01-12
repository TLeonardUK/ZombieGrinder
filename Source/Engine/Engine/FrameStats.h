// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_FRAMESTATS_
#define _ENGINE_FRAMESTATS_

#include "Generic/Types/HashTable.h"

struct FrameStat
{
	MEMORY_ALLOCATOR(FrameStat, "Engine");

public:
	const char* Name;
	float		Value;
};

class FrameStats 
{
	MEMORY_ALLOCATOR(FrameStats, "Engine");

private:
	static HashTable<FrameStat*, unsigned int> m_stats;
	
	FrameStats() {} // We are a static class.

public:
	static void	New_Tick();
	static void	Increment(const char* hash, float value = 1.0f);
	static void Print_Output();

};

#endif

