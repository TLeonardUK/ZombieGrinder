// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Tasks/GroupTask.h"
#include "Engine/Profiling/ProfilingManager.h"

GroupTask::GroupTask(std::string name)
	: m_name(name)
{
}

void GroupTask::Run()
{
	PROFILE_SCOPE("Group Task");
}

std::string GroupTask::Get_Name()
{
	return m_name;
}