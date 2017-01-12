// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMWORKS_ONLINE_ACHIEVEMENTS_
#define _ENGINE_STEAMWORKS_ONLINE_ACHIEVEMENTS_

#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"

#include "Engine/Online/OnlineAchievements.h"

struct Steamworks_AchievementProgress
{
public:
	std::string id;
	bool as_int;
	float progress;
	float max_progress;
};

struct Steamworks_StatProgress
{
public:
	bool as_int;
	bool is_global;
	std::string id;
	float increment;
};


class Steamworks_OnlineAchievements : public OnlineAchievements
{
	MEMORY_ALLOCATOR(Steamworks_OnlineAchievements, "Network");

private:
	STEAM_CALLBACK(Steamworks_OnlineAchievements, Callback_OnUserStatsReceived,		 UserStatsReceived_t,		m_on_user_stats_recieved_callback);
	STEAM_CALLBACK(Steamworks_OnlineAchievements, Callback_OnUserStatsStored,		 UserStatsStored_t,			m_on_user_stats_stored_callback);
	STEAM_CALLBACK(Steamworks_OnlineAchievements, Callback_OnUserAchievementsStored, UserAchievementStored_t,	m_on_user_achievements_stored_callback);
	
	bool m_initialized;

	std::vector<std::string> m_queued_unlocks;
	std::vector<std::string> m_queued_resets;
	std::vector<Steamworks_AchievementProgress> m_queued_progress;
	std::vector<Steamworks_StatProgress> m_queued_global_stats;

	SteamAPICall_t m_global_stats_call;
	CCallResult<Steamworks_OnlineAchievements, GlobalStatsReceived_t> m_global_stats_call_result;

	double m_progress_timer;
	double m_stat_timer;
	double m_sync_timer;

	bool m_force_sync;
	bool m_should_sync;

	enum
	{
		progress_update_interval = 10 * 1000,
		stat_update_interval = 5 * 60 * 1000,
		sync_interval = 5 * 60 * 1000,
		global_stat_interval = 7
	};

private:
	void On_Find_Global_Stats(GlobalStatsReceived_t* params, bool bIOFailure);

public:

public:
	Steamworks_OnlineAchievements();
	
	bool Initialize();

	void Tick(const FrameTime& time);
	
	void Unlock(const char* id);
	
	void Reset(const char* id);
	
	void Set_Progress(const char* id, float progress, float max_progress);
	void Set_Progress(const char* id, int progress, int max_progress);
		
	bool Get_Global_Stat(const char* id, float& value, int aggregation_period);
	bool Get_Global_Stat(const char* id, int& value, int aggregation_period);
	
	void Set_Global_Stat(const char* id, float value);
	void Set_Global_Stat(const char* id, int value);

	void Set_Stat(const char* id, float value);
	void Set_Stat(const char* id, int value);

	void Get_Stat(const char* id, float& value);
	void Get_Stat(const char* id, int& value);

	void Force_Sync();

};

#endif

