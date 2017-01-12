// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineAchievements.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Platform/Platform.h"

Steamworks_OnlineAchievements::Steamworks_OnlineAchievements()
	: m_on_user_stats_recieved_callback(this, &Steamworks_OnlineAchievements::Callback_OnUserStatsReceived)
	, m_on_user_stats_stored_callback(this, &Steamworks_OnlineAchievements::Callback_OnUserStatsStored)
	, m_on_user_achievements_stored_callback(this, &Steamworks_OnlineAchievements::Callback_OnUserAchievementsStored)
	, m_initialized(false)
	, m_progress_timer(0.0f)
	, m_stat_timer(0.0f)
	, m_sync_timer(0.0f)
	, m_should_sync(false)
	, m_force_sync(false)
{

}

void Steamworks_OnlineAchievements::Callback_OnUserStatsReceived(UserStatsReceived_t* params)
{
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();

	if (platform->Get_AppID() == params->m_nGameID)
	{
		if (k_EResultOK == params->m_eResult)
		{
			DBG_LOG("[Steamworks] Recieved stats and achievements from steam.");
			m_initialized = true;
		}
		else
		{
			DBG_LOG("[Steamworks] RequestStats failed with 0x%08x", params->m_eResult);
		}
	}
}

void Steamworks_OnlineAchievements::Callback_OnUserStatsStored(UserStatsStored_t* params)
{
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();

	if (platform->Get_AppID() == params->m_nGameID)
	{
		if (k_EResultOK == params->m_eResult)
		{
			DBG_LOG("[Steamworks] Stored stats.");
		}
		else
		{
			DBG_LOG("[Steamworks] Stats storage failed with 0x%08x", params->m_eResult);
		}
	}
}

void Steamworks_OnlineAchievements::Callback_OnUserAchievementsStored(UserAchievementStored_t* params)
{
	Steamworks_OnlinePlatform* platform = Steamworks_OnlinePlatform::Get();

	if (platform->Get_AppID() == params->m_nGameID)
	{
		DBG_LOG("[Steamworks] Stored achievements.");
	}
}

bool Steamworks_OnlineAchievements::Initialize()
{
	DBG_LOG("Initializing steamworks achievements.");

	if (*EngineOptions::reset_stats)
	{
		SteamUserStats()->ResetAllStats(*EngineOptions::reset_achievements);
	}
	//SteamUserStats()->ResetAllStats(true);

	bool result = SteamUserStats()->RequestCurrentStats();
	DBG_ASSERT_STR(result, "[Steamworks] RequestCurrentStats failed.");


	uint32 achievementCount = SteamUserStats()->GetNumAchievements();
	for (uint32 i = 0; i < achievementCount; i++)
	{
		const char* name = SteamUserStats()->GetAchievementName(i);
		bool bAchieved = false;
		SteamUserStats()->GetAchievement(name, &bAchieved);

		DBG_LOG("[Steamworks] Achievements[%i] name=%s achieved=%i", i, name, bAchieved);
	}

	return result;
}

void Steamworks_OnlineAchievements::Tick(const FrameTime& time)
{
	double ticks = Platform::Get()->Get_Ticks();

	if (m_initialized == true)
	{
		if (m_queued_unlocks.size() > 0)
		{
			for (std::vector<std::string>::iterator iter = m_queued_unlocks.begin(); iter != m_queued_unlocks.end(); iter++)
			{
				std::string id = *iter;		
				DBG_LOG("[Steamworks] Unlocking steamworks achievement '%s'.", id.c_str());

				SteamUserStats()->SetAchievement(id.c_str());
				m_force_sync = true;
			}
			m_queued_unlocks.clear();
		}

		if (m_queued_resets.size() > 0)
		{
			for (std::vector<std::string>::iterator iter = m_queued_resets.begin(); iter != m_queued_resets.end(); iter++)
			{
				std::string id = *iter;		
				DBG_LOG("[Steamworks] Resetting steamworks achievement '%s'.", id.c_str());
				
				SteamUserStats()->ClearAchievement(id.c_str());
			}
			m_queued_resets.clear();
		}

		if (m_queued_progress.size() > 0 || m_queued_global_stats.size() > 0)
		{
			double elapsed = ticks - m_progress_timer;
			if (elapsed > progress_update_interval || m_progress_timer == 0.0f)
			{
				for (std::vector<Steamworks_AchievementProgress>::iterator iter = m_queued_progress.begin(); iter != m_queued_progress.end(); iter++)
				{
					Steamworks_AchievementProgress prog = *iter;		
					if (prog.as_int)
					{
						SteamUserStats()->SetStat(prog.id.c_str(), (int32)prog.progress);
					}
					else
					{
						SteamUserStats()->SetStat(prog.id.c_str(), prog.progress);
					}
				}

				for (std::vector<Steamworks_StatProgress>::iterator iter = m_queued_global_stats.begin(); iter != m_queued_global_stats.end(); iter++)
				{
					Steamworks_StatProgress prog = *iter;		
					if (prog.as_int)
					{	
						SteamUserStats()->SetStat(prog.id.c_str(), (int32)prog.increment);
					}
					else
					{
						SteamUserStats()->SetStat(prog.id.c_str(), prog.increment);
					}
				}

				m_queued_progress.clear();
				m_queued_global_stats.clear();
				m_progress_timer = ticks;
				m_should_sync = true;

				DBG_LOG("[Steamworks] Performing periodic statistic sync.");
			}
		}

		if (m_should_sync || m_force_sync == true)
		{
			if (ticks - m_sync_timer >= sync_interval || m_force_sync == true)
			{
				bool result = SteamUserStats()->StoreStats();
				if (!result)
				{
					DBG_LOG("[Steamworks] StoreStats failed.");
				}

				m_should_sync = false;
				m_force_sync = false;
				m_sync_timer = ticks;
			}
		}

		// Refresh global statistics.
		if (ticks - m_stat_timer > stat_update_interval || m_stat_timer == 0.0f)
		{
			DBG_LOG("[Steamworks] Refreshing global statistics.");

			m_global_stats_call = SteamUserStats()->RequestGlobalStats(global_stat_interval);
			m_global_stats_call_result.Set(m_global_stats_call, this, &Steamworks_OnlineAchievements::On_Find_Global_Stats);

			m_stat_timer = ticks;
		}
	}
}

void Steamworks_OnlineAchievements::On_Find_Global_Stats(GlobalStatsReceived_t* params, bool bIOFailure)
{
	if (bIOFailure || params->m_eResult != k_EResultOK) 
	{
		DBG_LOG("[Steamworks] Failed to download global stats.");
		return;
	}

	DBG_LOG("[Steamworks] Retrieved global statistics.");
}

void Steamworks_OnlineAchievements::Unlock(const char* id)
{
	m_queued_unlocks.push_back(id);
}

void Steamworks_OnlineAchievements::Reset(const char* id)
{
	m_queued_resets.push_back(id);
}

void Steamworks_OnlineAchievements::Set_Progress(const char* id, int progress, int max_progress)
{
	for (std::vector<Steamworks_AchievementProgress>::iterator iter = m_queued_progress.begin(); iter != m_queued_progress.end(); iter++)
	{
		Steamworks_AchievementProgress& prog = *iter;
		if (prog.id == id)
		{
			prog.progress = (float)progress;
			prog.max_progress = (float)max_progress;
			return;
		}
	}

	Steamworks_AchievementProgress prog;
	prog.as_int = true;
	prog.id = id;
	prog.progress = (float)progress;
	prog.max_progress = (float)max_progress;	
	m_queued_progress.push_back(prog);
}

void Steamworks_OnlineAchievements::Set_Progress(const char* id, float progress, float max_progress)
{
	for (std::vector<Steamworks_AchievementProgress>::iterator iter = m_queued_progress.begin(); iter != m_queued_progress.end(); iter++)
	{
		Steamworks_AchievementProgress& prog = *iter;
		if (prog.id == id)
		{
			prog.progress = progress;
			prog.max_progress = max_progress;
			return;
		}
	}

	Steamworks_AchievementProgress prog;
	prog.as_int = false;
	prog.id = id;
	prog.progress = progress;
	prog.max_progress = max_progress;	
	m_queued_progress.push_back(prog);
}

bool Steamworks_OnlineAchievements::Get_Global_Stat(const char* id, float& value, int aggregation_period)
{
	if (aggregation_period == 0)
	{
		double lifetime = 0;
		SteamUserStats()->GetGlobalStat(id, &lifetime);

		value = (float)lifetime;
		return true;
	}
	else
	{
		double totals[60];
		int32 count = SteamUserStats()->GetGlobalStatHistory(id, totals, aggregation_period);

		double agg = 0;
		for (int i = 0; i < count; i++)
		{
			agg += totals[i];
		}

		if (count > 0)
		{
			value = (float)agg;
		}

		return (count > 0);
	}
}

bool Steamworks_OnlineAchievements::Get_Global_Stat(const char* id, int& value, int aggregation_period)
{
	if (aggregation_period == 0)
	{
		int64 lifetime = 0;
		SteamUserStats()->GetGlobalStat(id, &lifetime);

		value = (int)lifetime;
		return true;
	}
	else
	{
		int64 totals[60];
		int32 count = SteamUserStats()->GetGlobalStatHistory(id, totals, aggregation_period);

		double agg = 0;
		for (int i = 0; i < count; i++)
		{
			agg += (double)totals[i];
		}

		if (count > 0)
		{
			value = (int)agg;
		}

		return (count > 0);
	}
}

void Steamworks_OnlineAchievements::Set_Global_Stat(const char* id, float value)
{
	for (std::vector<Steamworks_StatProgress>::iterator iter = m_queued_global_stats.begin(); iter != m_queued_global_stats.end(); iter++)
	{
		Steamworks_StatProgress& prog = *iter;
		if (prog.id == id)
		{
			prog.increment = value;
			return;
		}
	}

	Steamworks_StatProgress prog;
	prog.as_int = false;
	prog.id = id;
	prog.increment = value;
	prog.is_global = true;

	m_queued_global_stats.push_back(prog);
}

void Steamworks_OnlineAchievements::Set_Global_Stat(const char* id, int value)
{
	for (std::vector<Steamworks_StatProgress>::iterator iter = m_queued_global_stats.begin(); iter != m_queued_global_stats.end(); iter++)
	{
		Steamworks_StatProgress& prog = *iter;
		if (prog.id == id)
		{
			prog.increment = (float)value;
			return;
		}
	}

	Steamworks_StatProgress prog;
	prog.as_int = true;
	prog.id = id;
	prog.increment = (float)value;
	prog.is_global = true;

	m_queued_global_stats.push_back(prog);
}

void Steamworks_OnlineAchievements::Set_Stat(const char* id, float value)
{	
	for (std::vector<Steamworks_StatProgress>::iterator iter = m_queued_global_stats.begin(); iter != m_queued_global_stats.end(); iter++)
	{
		Steamworks_StatProgress& prog = *iter;
		if (prog.id == id)
		{
			prog.increment = (float)value;
			return;
		}
	}

	Steamworks_StatProgress prog;
	prog.as_int = false;
	prog.id = id;
	prog.increment = (float)value;
	prog.is_global = false;

	m_queued_global_stats.push_back(prog);
}

void Steamworks_OnlineAchievements::Set_Stat(const char* id, int value)
{
	for (std::vector<Steamworks_StatProgress>::iterator iter = m_queued_global_stats.begin(); iter != m_queued_global_stats.end(); iter++)
	{
		Steamworks_StatProgress& prog = *iter;
		if (prog.id == id)
		{
			prog.increment = (float)value;
			return;
		}
	}

	Steamworks_StatProgress prog;
	prog.as_int = true;
	prog.id = id;
	prog.increment = (float)value;
	prog.is_global = false;

	m_queued_global_stats.push_back(prog);
}

void Steamworks_OnlineAchievements::Get_Stat(const char* id, float& value)
{
	float steam_value = 0.0f;
	if (SteamUserStats()->GetStat(id, &steam_value))
	{
		value = steam_value;
	}
}

void Steamworks_OnlineAchievements::Get_Stat(const char* id, int& value)
{
	int steam_value = 0;
	if (SteamUserStats()->GetStat(id, &steam_value))
	{
		value = steam_value;
	}
}

void Steamworks_OnlineAchievements::Force_Sync()
{
	m_force_sync = true;
}