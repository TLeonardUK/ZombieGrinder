// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_PERSISTENTLOGHELPER_
#define _GENERIC_HELPER_PERSISTENTLOGHELPER_

#include <string>
#include <vector>
#include "Generic/Types/Color.h"

struct PersistentLog
{
	std::string message;
	Color color;
	float timeRemaining;
	int id;
};

class PersistentLogHelper
{
private:
	PersistentLogHelper();

	static std::vector<PersistentLog> m_logs;

public:
	static void Write(int id, Color Color, float lifetimeSeconds, const char* format, ...);
	static void Tick(float deltaTime);
	static std::vector<PersistentLog> GetActive();

};

#define DBG_ONSCREEN_LOG(id, color, lifetime, format,  ...) PersistentLogHelper::Write(id, color, lifetime, format, ##__VA_ARGS__);
#define DBG_ONSCREEN_LOG_STRID(id, color, lifetime, format,  ...) PersistentLogHelper::Write(StringHelper::Hash(id), color, lifetime, format, ##__VA_ARGS__);

#endif