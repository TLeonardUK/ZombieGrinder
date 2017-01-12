// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Online/ChallengeManager.h"
#include "Game/Online/StatisticsManager.h"
#include "Game/Online/RankingsManager.h"

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineMatching.h"

#include "Engine/Localise/Locale.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Game/Profile/ItemManager.h"

#include "Generic/Math/Random.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Game/UI/Scenes/UIScene_StartingLocalServer.h"
#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"
#include "Engine/UI/UIManager.h"

#include "Game/Runner/Game.h"
 
//#define FORCE_CHALLENGE_TYPE "CHALLENGE_MAX_WAVES"

char* ChallengeTimeframe::Strings[ChallengeTimeframe::COUNT] = {
	"Daily",
	"Weekly",
	"Monthly",
};

ChallengeManager::ChallengeManager()
	: m_init(false)
	, m_first_load(false)
	, m_starting_custom_map(false)
	, m_active_challenge_map_challenge(NULL)
{
	for (int i = 0; i < ChallengeTimeframe::COUNT; i++)
	{
		m_active_challenges[i].active = false;
		m_active_challenges[i].timeframe = (ChallengeTimeframe::Type)i;
		m_active_challenges[i].event_listener = NULL;
	}
}

ChallengeManager::~ChallengeManager()
{
	for (int i = 0; i < ChallengeTimeframe::COUNT; i++)
	{
		SAFE_DELETE(m_active_challenges[i].event_listener);
	}
}

bool ChallengeManager::Init()
{
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading challenges ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* base_symbol = vm->Find_Class("Challenge");
	DBG_ASSERT(base_symbol != NULL);

	std::vector<CVMLinkedSymbol*> extended_classes = vm->Find_Derived_Classes(base_symbol);

	m_challenges.resize(extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	int index = 0;
	for (std::vector<CVMLinkedSymbol*>::iterator iter = extended_classes.begin(); iter != extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* achievement_class = *iter;
		DBG_LOG("Loading Challenge Class: %s", achievement_class->symbol->name);

		Challenge& achievement = m_challenges.at(index);
		achievement.specific_date = false;
		achievement.announce = false;
		achievement.script_object = vm->New_Object(achievement_class, true, &achievement);
		vm->Set_Default_State(achievement.script_object);
	}

	m_init = true;
	return true;
}

Challenge* ChallengeManager::Get_Active_Challenge(ChallengeTimeframe::Type timeframe)
{
	return &m_active_challenges[timeframe];
}

ItemArchetype* ChallengeManager::Get_Next_Reward()
{
	if (m_pending_rewards.size() == 0)
	{
		return NULL;
	}

	ItemArchetype* item = m_pending_rewards.back();
	m_pending_rewards.pop_back();

	return item;
}

void ChallengeManager::Tick(const FrameTime& time)
{
	if (!m_init)
		return;

	if (!StatisticsManager::Get()->Is_Init())
		return;

	// Calculate challenge intervals.
	Calculate_Challenge_Times();

	// New challenges?
	Tick_Active_Challenge(time, m_active_challenges[(int)ChallengeTimeframe::Daily], m_daily_start_time, m_daily_end_time);
	Tick_Active_Challenge(time, m_active_challenges[(int)ChallengeTimeframe::Weekly], m_weekly_start_time, m_weekly_end_time);
	Tick_Active_Challenge(time, m_active_challenges[(int)ChallengeTimeframe::Monthly], m_monthly_start_time, m_monthly_end_time);

	// Starting custom map?
	UIManager* ui_manager = GameEngine::Get()->Get_UIManager();
	if (m_starting_custom_map)
	{
		if (ui_manager->Get_Scene_By_Type<UIScene_StartingLocalServer*>() == NULL && !ui_manager->Transition_In_Progress() && !ui_manager->Pending_Actions())
		{
			m_starting_custom_map = false;

			// Are we connected to a server now?
			OnlineMatching_Server* server = OnlineMatching::Get()->Get_Lobby_Server();
			if (server != NULL)
			{
				DBG_LOG("Lobby is now connected to a server, connecting to server ...");
				ui_manager->Go(UIAction::Push(new UIScene_ConnectToServer(*server), new UIFadeInTransition()));
				return;
			}
			else
			{
				DBG_LOG("No server to join, da fuq?!");
				if (OnlineMatching::Get()->Get_State() == OnlineMatching_State::In_Lobby)
				{
					OnlineMatching::Get()->Leave_Lobby();
				}
				m_active_challenge_map_challenge = NULL;
			}
		}
	}
	// In a custom map?
	else if (m_active_challenge_map_challenge != NULL)
	{
		// Server changed? No longer in challenge map.
		if (ui_manager->Get_Scene_By_Type<UIScene_StartingLocalServer*>() == NULL && 
			ui_manager->Get_Scene_By_Type<UIScene_ConnectToServer*>() == NULL &&
			ui_manager->Get_Scene_By_Type<UIScene_Game*>() == NULL &&
			ui_manager->Get_Scene_By_Type<UIScene_MapLoading*>() == NULL && 
			!ui_manager->Transition_In_Progress() && !ui_manager->Pending_Actions())
		{
			DBG_LOG("User is not in game, assuming they have left challenge map.");
			m_active_challenge_map_challenge = NULL;
		}
	}
}

void ChallengeManager::Activate_Challenge(ChallengeTimeframe::Type timeframe)
{
	Challenge* challenge = &m_active_challenges[(int)timeframe];

	CVMLinkedSymbol* class_symbol = challenge->script_object.Get().Get()->Get_Symbol();
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* setup_func = vm->Find_Function(class_symbol, "On_Activate", 0);
	if (setup_func)
	{
		vm->Invoke(setup_func, challenge->script_object);
	}
}

void ChallengeManager::Start_Custom_Map(Challenge* challenge, std::string map, int seed)
{
	DBG_LOG("Starting custom challenge map: challenge=%s, map=%s, seed=0x%08x", ChallengeTimeframe::Strings[(int)challenge->timeframe], map.c_str(), seed);

	if (OnlinePlatform::Get()->Get_Local_User_Count() > 1)
	{
		GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_Dialog(S("#challenge_to_many_local_users")), new UIFadeInTransition()));
		return;
	}

	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_Short_Name(map.c_str());
	DBG_ASSERT(handle != NULL);

	// Create offline lobby.
	OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
	settings.MapGUID = handle->Get()->Get_Header()->GUID;
	settings.MapWorkshopID = 0;
	settings.MaxPlayers = 1;
	settings.Private = true;
	settings.Secure = false;
	settings.Local = true;
	settings.IsOffline = true;
	OnlineMatching::Get()->Set_Lobby_Settings(settings);
	OnlineMatching::Get()->Create_Lobby(settings);

	DBG_ASSERT(OnlineMatching::Get()->Get_State() == OnlineMatching_State::In_Lobby);

	// Start local server.
	GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_StartingLocalServer(false), new UIFadeInTransition()));

	m_active_challenge_map_challenge = challenge;
	m_starting_custom_map = true;
}

bool ChallengeManager::In_Custom_Map(Challenge* challenge)
{
	return (m_active_challenge_map_challenge == challenge);
}

bool ChallengeManager::In_Custom_Map()
{
	return (m_active_challenge_map_challenge != NULL);
}

void ChallengeManager::Tick_Active_Challenge(const FrameTime& time, Challenge& challenge, time_t start_time, time_t end_time)
{	
	// Challenge changed?
	if (!challenge.active || challenge.start_time != start_time || challenge.end_time != end_time)
	{
		if (challenge.active)
		{
			SAFE_DELETE(challenge.event_listener);
			challenge.script_object = NULL;
		}

		static u64 score_base_time = 1400000000;

		challenge.active = true;
		challenge.archetype = NULL;
		challenge.end_time = end_time;
		challenge.start_time = start_time;
		challenge.seed = (int)(((u64)challenge.start_time - score_base_time) % 100000000);
		challenge.progress = 0.0f;
		challenge.announced = false;
		challenge.announce = false;
		challenge.complete = false;
		challenge.was_just_completed = false;
		challenge.last_submitted_score = 0.0f;
		challenge.leaderboard_target_score = (((int)((u64)challenge.start_time - score_base_time)) / 60) / 60; // Hourly values.

		Random challenge_rand(challenge.seed);
		
		time_t current_time = (time_t)OnlinePlatform::Get()->GetServerTime();
		struct tm current_time_tm = *gmtime(&current_time);
		struct tm start_time_tm = *gmtime(&start_time);
		struct tm end_time_tm = *gmtime(&end_time);

		// Select archetype.
		std::vector<Challenge*> challenges;
		for (unsigned int i = 0; i < m_challenges.size(); i++)
		{
#ifdef FORCE_CHALLENGE_TYPE
			if (m_challenges[i].id == FORCE_CHALLENGE_TYPE)
#else
			if (m_challenges[i].timeframe == challenge.timeframe)
#endif
			{
#ifndef FORCE_CHALLENGE_TYPE
				if (m_challenges[i].specific_date)
				{
					if (start_time_tm.tm_mon <= m_challenges[i].start_month - 1 &&
						end_time_tm.tm_mon >= m_challenges[i].start_month - 1 &&
						start_time_tm.tm_mday <= m_challenges[i].start_day &&
						end_time_tm.tm_mday >= m_challenges[i].start_day)
					{
						challenges.clear();
						challenges.push_back(&m_challenges[i]);
						break;
					}
				}
				else
#endif
				{
					challenges.push_back(&m_challenges[i]);
				}
			}
		}

		// No challenges for us :(
		if (challenges.size() == 0)
		{
			challenge.active = false;
			return;
		}

		challenge.archetype = challenges[challenge_rand.Next() % challenges.size()];

		CVMLinkedSymbol* class_symbol = challenge.archetype->script_object.Get().Get()->Get_Symbol();
		EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
		CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

		challenge.script_object = vm->New_Object(class_symbol, true, &challenge);
		vm->Set_Default_State(challenge.script_object);

		challenge.event_listener = new ScriptEventListener(vm->Get_Static_Context(), challenge.script_object);

		// Debug code!
		char start_time_str[48];
		char endtime_str[48];
		strftime(start_time_str, 1024, "%c", &start_time_tm);
		strftime(endtime_str, 1024, "%c", &end_time_tm);

		DBG_LOG("[Challenges] New challenge (timeframe=%s): Archetype=%s Start=%s End=%s",
			ChallengeTimeframe::Strings[(int)challenge.timeframe],
			challenge.archetype->id.c_str(),
			start_time_str,
			endtime_str);

		// Generate rewards.
		int original_seed = Random::Get_Static_Seed();
		Random::Seed_Static(challenge_rand.Next());

		CVMLinkedSymbol* setup_func = vm->Find_Function(class_symbol, "Seeded_Setup", 0);
		if (setup_func)
		{
			vm->Invoke(setup_func, challenge.script_object);
		}

		CVMLinkedSymbol* generate_func = vm->Find_Function(class_symbol, "Get_Rewards", 0);
		if (generate_func)
		{
			vm->Invoke(generate_func, challenge.script_object);

			CVMValue reward_array;
			vm->Get_Return_Value(reward_array);
			CVMObject* reward_array_obj = reward_array.object_value.Get();

			challenge.rewards.clear();

			for (int i = 0; i < reward_array_obj->Slot_Count(); i++)
			{
				CVMObject* item_archetype_obj = reward_array_obj->Get_Slot(i).object_value.Get();
				ItemArchetype* archetype = reinterpret_cast<ItemArchetype*>(item_archetype_obj->Get_Meta_Data());
				DBG_LOG("Reward[%i] %s", i, archetype->Get_Display_Name(false, 0).c_str());
				challenge.rewards.push_back(archetype);
			}
		}

		Random::Seed_Static(original_seed);
	}

	if (challenge.active)
	{
		u64 current_time = OnlinePlatform::Get()->GetServerTime();
		challenge.time_remaining = Max(0, challenge.end_time - current_time);

		// If in restricted mode we cannot progress.
		if (Game::Get()->In_Restricted_Mode())
		{
			challenge.progress = 0.0f;
		}

		// Have they completed the challenge? Time to give them $$$?
		if (challenge.complete == false && 
			challenge.progress >= challenge.max_progress)
		{
			// Give user rewards.
			for (int i = 0; i < (int)challenge.rewards.size(); i++)
			{
				m_pending_rewards.push_back(challenge.rewards[i]);
			}

			challenge.complete = true;
			challenge.was_just_completed = true;
		}

		/*DBG_ONSCREEN_LOG(StringHelper::Hash("Challenge") + (int)challenge.timeframe, Color::White, 1.0f, "Timeframe=%s, Archetype=%s, Progress=%.1f, Remaining=%i days, %i hours, %i minutes, %i seconds",
			ChallengeTimeframe::Strings[(int)challenge.timeframe],
			challenge.archetype->name.c_str(),
			challenge.progress,
			(int)((challenge.time_remaining / 60) / 60) / 24,
			(int)((challenge.time_remaining / 60) / 60) % 24,
			(int)(challenge.time_remaining / 60) % 60,
			(int)challenge.time_remaining % 60
		);*/
	}
}

bool ChallengeManager::Get_Announcement(ChallengeAnnouncement* output)
{
	for (int i = 0; i < ChallengeTimeframe::COUNT; i++)
	{
		Challenge* challenge = &m_active_challenges[i];
		if ( challenge->active && 
			 challenge->announce &&
			!challenge->announced &&
			!challenge->complete)
		{
			output->name = challenge->announce_name;
			output->description = challenge->announce_description;
			output->image = challenge->announce_image;

			challenge->announced = true;

			return true;
		}
	}
	return false;
}

void ChallengeManager::Calculate_Challenge_Times()
{
	static int DaysPerMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// Base everything on server time to ensure nobody fucks with the clock :)
	time_t time = (time_t)OnlinePlatform::Get()->GetServerTime();
	struct tm real_time = *gmtime(&time);

	// Find start of the month.
	struct tm monthly_start = real_time;
	monthly_start.tm_hour = 0;
	monthly_start.tm_min = 0;
	monthly_start.tm_sec = 0;
	monthly_start.tm_mday = 1;

	// Find end of the month.
	struct tm monthly_end = monthly_start;
	monthly_end.tm_hour = 23;
	monthly_end.tm_min = 59;
	monthly_end.tm_sec = 59;
	if (monthly_end.tm_mon == 11)
	{
		monthly_end.tm_mon = 0;
		monthly_end.tm_year = monthly_start.tm_year + 1;
	}
	else
	{
		monthly_end.tm_mon++;
	}

	time_t last_day = mktime(&monthly_end);
	last_day -= 86400; // 1 day.
	monthly_end = *gmtime(&last_day);

	// Find start of the week.
	struct tm weekly_start = real_time;
	weekly_start.tm_hour = 0;
	weekly_start.tm_min = 0;
	weekly_start.tm_sec = 0;
	int days_off = weekly_start.tm_wday - 1;
	weekly_start.tm_wday = 1;
	weekly_start.tm_mday -= days_off;
	if (weekly_start.tm_mday < 0)
	{
		if (weekly_start.tm_mon == 0)
		{
			weekly_start.tm_year--;
			weekly_start.tm_mon = 11;
			weekly_start.tm_mday = DaysPerMonth[11] - 1;
		}
		else
		{
			weekly_start.tm_mon--;
			weekly_start.tm_mday = (DaysPerMonth[weekly_start.tm_mon] - 1) + weekly_start.tm_mday;
		}
	}

	// Find end of the week.
	struct tm weekly_end = real_time;
	weekly_end.tm_hour = 23;
	weekly_end.tm_min = 59;
	weekly_end.tm_sec = 59;
	days_off = 7 - weekly_end.tm_wday;
	weekly_end.tm_wday = 0;
	weekly_end.tm_mday += days_off;
	if (weekly_end.tm_mday >= DaysPerMonth[weekly_end.tm_mon])
	{
		if (weekly_end.tm_mon == 11)
		{
			weekly_end.tm_year++;
			weekly_end.tm_mon = 0;
			weekly_end.tm_mday = 0;
		}
		else
		{
			weekly_end.tm_mon++;
			weekly_end.tm_mday = 0;
		}
	}

	// Find start of the day.
	struct tm daily_start = real_time;
	daily_start.tm_hour = 0;
	daily_start.tm_min = 0;
	daily_start.tm_sec = 0;

	// Find end of the day.
	struct tm daily_end = real_time;
	daily_end.tm_hour = 23;
	daily_end.tm_min = 59;
	daily_end.tm_sec = 59;

	m_daily_start_time = mktime(&daily_start);
	m_daily_end_time = mktime(&daily_end);
	m_weekly_start_time = mktime(&weekly_start);
	m_weekly_end_time = mktime(&weekly_end);
	m_monthly_start_time = mktime(&monthly_start);
	m_monthly_end_time = mktime(&monthly_end);
}

void ChallengeManager::Tick_Active(const FrameTime& time)
{
	// Calculate challenge intervals.
	Calculate_Challenge_Times();

	// New challenges?
	Tick_Active_Challenge(time, m_active_challenges[(int)ChallengeTimeframe::Daily], m_daily_start_time, m_daily_end_time);
	Tick_Active_Challenge(time, m_active_challenges[(int)ChallengeTimeframe::Weekly], m_weekly_start_time, m_weekly_end_time);
	Tick_Active_Challenge(time, m_active_challenges[(int)ChallengeTimeframe::Monthly], m_monthly_start_time, m_monthly_end_time);
}

void ChallengeManager::Submit_Rankings()
{
	RankingsManager* rankings = RankingsManager::Get();
	for (int i = 0; i < ChallengeTimeframe::COUNT; i++)
	{
		Challenge* challenge = &m_active_challenges[i];
		if (challenge->last_submitted_score < challenge->progress &&
			challenge->progress >= challenge->max_progress)
		{
			Leaderboard* board = RankingsManager::Get()->Get_Or_Create_Leaderboard(StringHelper::Format("%s_Challenge", ChallengeTimeframe::Strings[i]).c_str());
			rankings->Submit(board, challenge->leaderboard_target_score, (int)challenge->progress);
			challenge->last_submitted_score = challenge->progress;
		}
	}
}

bool ChallengeManager::Serialize(BinaryStream* stream)
{
	DBG_ASSERT(m_init);

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	// Save active challenge states.
	for (int i = 0; i < ChallengeTimeframe::COUNT; i++)
	{
		Challenge* challenge = &m_active_challenges[i];
		stream->Write<int>(challenge->active ? 1 : 0);
		stream->Write<float>(challenge->progress);
		stream->Write<int>(challenge->complete ? 1 : 0);
		stream->Write<int>(challenge->leaderboard_target_score);
		stream->Write<int>(challenge->script_object.Get().Get()->Get_Symbol()->symbol->unique_id);
	}

	// Save pending rewards.
	stream->Write<int>((int)m_pending_rewards.size());
	for (int i = 0; i < (int)m_pending_rewards.size(); i++)
	{
		ItemArchetype* item = m_pending_rewards[i];
		stream->Write<int>(item->script_object.Get().Get()->Get_Symbol()->symbol->unique_id);
	}

	return true;
}

bool ChallengeManager::Deserialize(BinaryStream* stream)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	// Ensure we have active states to deserialise to.
	Tick_Active(*GameEngine::Get()->Get_Time());

	// Load active challenge state.
	for (int i = 0; i < ChallengeTimeframe::COUNT; i++)
	{
		Challenge* challenge = &m_active_challenges[i];
		bool active = stream->Read<int>() != 0;
		float progress = stream->Read<float>();
		bool complete = stream->Read<int>() != 0;
		int leaderboard_target_score = stream->Read<int>();
		int unique_id = stream->Read<int>();

		// Only apply these settings if we are still on the same challenge.
		if (challenge->active == active &&
			challenge->leaderboard_target_score == leaderboard_target_score &&
			challenge->script_object.Get().Get()->Get_Symbol()->symbol->unique_id == unique_id)
		{
			challenge->progress = progress;
			challenge->complete = complete;
		}
	}

	// Load pending rewards.
	m_pending_rewards.clear();

	int count = stream->Read<int>();
	for (int i = 0; i < count; i++)
	{
		int unique_id = stream->Read<int>();
		ItemArchetype* archetype = ItemManager::Get()->Find_Archetype(unique_id); 
		if (archetype)
		{
			m_pending_rewards.push_back(archetype);
		}
	}

	m_first_load = true;

	return true;
}