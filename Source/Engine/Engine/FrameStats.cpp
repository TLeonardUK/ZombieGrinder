// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Engine/FrameStats.h"

#include "Generic/Helper/StringHelper.h"

HashTable<FrameStat*, unsigned int> FrameStats::m_stats;

void FrameStats::New_Tick()
{
#ifdef ENABLE_STAT_COLLECTION
	for (HashTable<FrameStat*, unsigned int>::Iterator iter = m_stats.Begin(); iter != m_stats.End(); iter++)
	{
		FrameStat* stat = *iter;
		stat->Value = 0.0f;
	}
#endif
}

void FrameStats::Increment(const char* name, float value)
{
#ifdef ENABLE_STAT_COLLECTION
	unsigned int hash = StringHelper::Hash(name);

	FrameStat* stat;

	if (m_stats.Get(hash, stat))
	{
		stat->Value += value;
	}
	else
	{
		stat = new FrameStat();	// Going to ignore disposing of these, there will only be a handful at max.
		stat->Name = name;
		stat->Value = value;
		m_stats.Set(hash, stat);
	}
#endif
}

void FrameStats::Print_Output()
{
#ifdef ENABLE_STAT_COLLECTION
	DBG_LOG("============= FRAME STATS =============");
	for (HashTable<FrameStat*, unsigned int>::Iterator iter = m_stats.Begin(); iter != m_stats.End(); iter++)
	{
		FrameStat* stat = *iter;
		DBG_LOG(" %s = %f", stat->Name, stat->Value)
	}
	DBG_LOG("");
#endif
}
