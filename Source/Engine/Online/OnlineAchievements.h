// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ONLINE_ACHIEVEMENTS_
#define _ENGINE_ONLINE_ACHIEVEMENTS_

#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/FrameTime.h"

class OnlineAchievements : public Singleton<OnlineAchievements>
{
	MEMORY_ALLOCATOR(OnlineAchievements, "Network");

private:

public:

public:
	virtual ~OnlineAchievements() {} 

	static OnlineAchievements* Create();
	
	// Connects to online platform.
	virtual bool Initialize() = 0;

	// Tick tock tick tock
	virtual void Tick(const FrameTime& time) = 0;
	
	// ----------------------------------------------------------------------------
	// Achievments
	// ----------------------------------------------------------------------------

	// Unlock an chiiieve.
	virtual void Unlock(const char* id) = 0;
	
	// Resets the state of an achievement.s
	virtual void Reset(const char* id) = 0;

	// Sets the progress of the achievement.
	virtual void Set_Progress(const char* id, float progress, float max_progress) = 0;
	virtual void Set_Progress(const char* id, int progress, int max_progress) = 0;
	
	// ----------------------------------------------------------------------------
	// Statistics
	// ----------------------------------------------------------------------------
	
	// Resets the state of a statistic.
	virtual bool Get_Global_Stat(const char* id, float& value, int aggregation_period = 1) = 0;
	virtual bool Get_Global_Stat(const char* id, int& value, int aggregation_period = 1) = 0;
	
	// Modifies the value of a global statistic.
	virtual void Set_Global_Stat(const char* id, float value) = 0;
	virtual void Set_Global_Stat(const char* id, int value) = 0;

	// Modifies the value of a statistic.
	virtual void Set_Stat(const char* id, float value) = 0;
	virtual void Set_Stat(const char* id, int value) = 0;

	// Gets the value of a stat.
	virtual void Get_Stat(const char* id, float& value) = 0;
	virtual void Get_Stat(const char* id, int& value) = 0;

	// Force syncs everything.
	virtual void Force_Sync() = 0;

};

#endif

