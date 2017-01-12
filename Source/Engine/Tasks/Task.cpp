// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Tasks/Task.h"

Task::Task()
	: m_completed(false)
{
}

void Task::Reset()
{
	m_completed = false;
}

void Task::Complete()
{
	m_completed = true;
}

bool Task::Is_Completed()
{
	return m_completed;
}
