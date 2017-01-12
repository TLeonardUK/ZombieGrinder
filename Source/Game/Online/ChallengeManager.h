// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ONLINE_CHALLENGE_MANAGER_
#define _GAME_ONLINE_CHALLENGE_MANAGER_

#include "Engine/IO/BinaryStream.h"
#include "Engine/Engine/FrameTime.h"

#include "XScript/VirtualMachine/CVMObject.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Generic/Patterns/Singleton.h"

#include <ctime>

struct AtlasFrame;
struct Challenge;
struct ItemArchetype;

struct ChallengeTimeframe
{
	enum Type
	{
		Daily,
		Weekly,
		Monthly,

		COUNT
	};

	static char* Strings[ChallengeTimeframe::COUNT];
};

struct Challenge
{
public:
	std::string					id;
	std::string					name;
	std::string					description;
	ChallengeTimeframe::Type	timeframe;
	bool						requires_activation;

	float						progress;
	float						max_progress;

	bool						was_just_completed;

	float						last_submitted_score;

	bool						active;
	Challenge*					archetype;
	time_t						start_time;
	time_t						end_time;
	u64							time_remaining;
	int							leaderboard_target_score;
	int							seed;
	bool						complete;
	std::vector<ItemArchetype*>	rewards;

	ScriptEventListener*		event_listener;
	CVMGCRoot					script_object;

	bool						specific_date;
	int							start_day;
	int							start_month;

	bool						announced;

	bool						announce;
	std::string					announce_name;
	std::string					announce_description;
	std::string					announce_image;
};

struct ChallengeAnnouncement
{
public:
	std::string	name;
	std::string	description;
	std::string	image;
};

class ChallengeManager : public Singleton<ChallengeManager>
{
	MEMORY_ALLOCATOR(ChallengeManager, "Game");

private:
	bool m_init;
	bool m_first_load;

	int m_last_month;

	time_t m_daily_start_time;
	time_t m_daily_end_time;
	time_t m_weekly_start_time;
	time_t m_weekly_end_time;
	time_t m_monthly_start_time;
	time_t m_monthly_end_time;

	std::vector<Challenge> m_challenges;

	std::vector<ItemArchetype*> m_pending_rewards;

	Challenge m_active_challenges[ChallengeTimeframe::COUNT];

	bool m_starting_custom_map;
	Challenge* m_active_challenge_map_challenge;

private:
	void Calculate_Challenge_Times();
	void Tick_Active_Challenge(const FrameTime& time, Challenge& challenge, time_t start_time, time_t end_time);
	void Tick_Active(const FrameTime& time);

public:
	ChallengeManager();
	~ChallengeManager();

	Challenge* Get_Active_Challenge(ChallengeTimeframe::Type timeframe);

	void Activate_Challenge(ChallengeTimeframe::Type timeframe);

	bool Get_Announcement(ChallengeAnnouncement* output);

	void Start_Custom_Map(Challenge* challenge, std::string map, int seed);
	bool In_Custom_Map(Challenge* challenge);
	bool In_Custom_Map();

	ItemArchetype* Get_Next_Reward();

	void Submit_Rankings();

	bool Init();

	bool Serialize(BinaryStream* stream);
	bool Deserialize(BinaryStream* stream);

	void Tick(const FrameTime& time);

};

#endif

