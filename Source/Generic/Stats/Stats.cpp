// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Stats/Stats.h"
#include "Generic/Helper/StringHelper.h"

#include <cstring>

Stat* StatManager::s_statistics[max_statistics];
int StatManager::s_statistic_count = 0;

Stat::Stat(const char* path, bool display)
{
	Display = display;
	
	std::vector<std::string> segments;
	StringHelper::Split(path, '/', segments);

	DBG_ASSERT(segments.size() <= max_path_segments);

	// Would rather use a std::vector<std::string> but because of static destructor
	// order we don't know if the backing memory is gone or not at destruction, better
	// to store statically.
	for (unsigned int i = 0; i < segments.size(); i++)
	{
		char* dest = Path[i];
		strcpy(dest, segments[i].c_str());
	}

	Path_Count = segments.size();

	StatManager::Register(this);
}

void Stat::New_Frame()
{
	// Do in derived class.
}

StatManager::~StatManager()
{
}

void StatManager::Register(Stat* stat)
{
	s_statistics[s_statistic_count++] = stat;
}

int StatManager::Get_Statistics(Stat**& output)
{
	output = s_statistics;
	return s_statistic_count;
}

void StatManager::New_Frame()
{
	for (int i = 0; i < s_statistic_count; i++)
	{
		s_statistics[i]->New_Frame();
	}
}
