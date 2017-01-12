// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Helper/PersistentLogHelper.h"

#include <stdarg.h> 
#include <cstring>
#include <cstdio>
#include <math.h>

std::vector<PersistentLog> PersistentLogHelper::m_logs;

void PersistentLogHelper::Write(int id, Color Color, float lifetimeSeconds, const char* format, ...)
{
	va_list va;
	va_start(va, format);

	std::string result;

	char buffer[512];
	int num = vsnprintf(buffer, 512, format, va);
	if (num >= 511)
	{
		char* new_buffer = new char[num + 1];
		vsnprintf(new_buffer, num + 1, format, va);
		result = new_buffer;
		delete[] new_buffer;
		va_end(va);
	}
	else
	{
		result = buffer;
	}

	va_end(va);

	for (std::vector<PersistentLog>::iterator iter = m_logs.begin(); iter != m_logs.end(); iter++)
	{
		PersistentLog& log = *iter;
		if (log.id == id)
		{
			log.timeRemaining = lifetimeSeconds;
			log.color = Color;
			log.message = result;
			return;
		}
	}

	PersistentLog log;
	log.timeRemaining = lifetimeSeconds;
	log.color = Color;
	log.id = id;
	log.message = result;

	m_logs.push_back(log);
}

void PersistentLogHelper::Tick(float deltaTime)
{
	for (std::vector<PersistentLog>::iterator iter = m_logs.begin(); iter != m_logs.end(); )
	{
		PersistentLog& log = *iter;
		log.timeRemaining -= deltaTime;

		if (log.timeRemaining < 0.0f) 
		{
			iter = m_logs.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

std::vector<PersistentLog> PersistentLogHelper::GetActive()
{
	return m_logs;
}
