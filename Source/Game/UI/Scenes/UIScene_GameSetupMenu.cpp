// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_GameSetupMenu.h"
#include "Game/UI/Scenes/UIScene_FindGameSetup.h"
#include "Game/UI/Scenes/UIScene_FindGame.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_HostGameSetup.h"
#include "Game/UI/Scenes/UIScene_HostGame.h"
#include "Game/UI/Scenes/UIScene_ServerBrowser.h"
#include "Game/UI/Scenes/UIScene_Inventory.h"
#include "Game/UI/Scenes/UIScene_Achievements.h"
#include "Game/UI/Scenes/UIScene_Leaderboards.h"
#include "Game/UI/Scenes/UIScene_Statistics.h"
#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Game/UI/Scenes/UIScene_GlobalChat.h"
#include "Game/UI/Scenes/UIScene_InventoryDropDialog.h"
#include "Game/UI/Scenes/UIScene_ItemBrokenDialog.h"
#include "Game/UI/Scenes/UIScene_UnlockDialog.h"
#include "Game/UI/Scenes/UIScene_ChallengeAnnouncement.h"
#include "Game/UI/Scenes/UIScene_RanksAndStats.h"
#include "Game/UI/Scenes/UIScene_Moderation.h"

#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UIButton.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"

#include "Generic/Math/Random.h"

#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineUser.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIElement.h"
#include "Engine/UI/Elements/UILabel.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

#include "Game/Online/ChallengeManager.h"

#include "Game/Online/RankingsManager.h"
#include "Engine/Online/OnlineUser.h"
#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"
#include "Game/Profile/SkillManager.h"

#include "Game/Moderation/ReportManager.h"

#include "XScript/VirtualMachine/CVMBinary.h"

//#define OPT_CYCLE_CHALLENGES 1

UIScene_GameSetupMenu::UIScene_GameSetupMenu()
{
	m_cycle_challenge_time = 0.0f;
	m_direction = 0;
	m_direction_change_timer = 0.0f;
	m_item_select_time = 0.0f;
	m_info_alpha = 0.0f;
	m_initial_seek = false;

	Set_Layout("game_setup_menu");

	m_draw_item_delegate = new Delegate<UIScene_GameSetupMenu, UIPanelDrawItem>(this, &UIScene_GameSetupMenu::On_Draw_Item);
	m_get_item_delegate = new Delegate<UIScene_GameSetupMenu, UIListViewGetItemData>(this, &UIScene_GameSetupMenu::On_Get_Item_Data);

	Find_Element<UIPanel*>("challenge_reward_1_bg")->On_Draw += m_draw_item_delegate;
	Find_Element<UIPanel*>("challenge_reward_2_bg")->On_Draw += m_draw_item_delegate;
	Find_Element<UIPanel*>("challenge_reward_3_bg")->On_Draw += m_draw_item_delegate;
	Find_Element<UIPanel*>("challenge_reward_4_bg")->On_Draw += m_draw_item_delegate;

	UIListView* listview = Find_Element<UIListView*>("challenge_ranking_box");
	listview->Clear_Columns();
	listview->Add_Column("", 0.1f, false, true);
	listview->Add_Column(S("#menu_leaderboards_rank_short_column"), 0.15f);
	listview->Add_Column(S("#menu_leaderboards_name_column"), 0.45f, true, true);
	listview->Add_Column(S("#menu_leaderboards_score_column"), 0.30f);
	listview->On_Get_Item_Data += m_get_item_delegate;
	listview->Set_Item_Height(10.0f);
}

UIScene_GameSetupMenu::~UIScene_GameSetupMenu()
{
	SAFE_DELETE(m_draw_item_delegate);
}

const char* UIScene_GameSetupMenu::Get_Name()
{
	return "UIScene_GameSetupMenu";
}

bool UIScene_GameSetupMenu::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_GameSetupMenu::Should_Display_Cursor()
{
	return true;
}

bool UIScene_GameSetupMenu::Is_Focusable()
{
	return true;
}

UIScene* UIScene_GameSetupMenu::Get_Background(UIManager* manager)
{
	return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_GameSetupMenu::Enter(UIManager* manager)
{
	Update_Moderate();

	Game::Get()->Check_Inventory_Drops();
	ChallengeManager::Get()->Submit_Rankings();
	Show_Challenge(ChallengeTimeframe::Daily);
}	

void UIScene_GameSetupMenu::Update_Moderate()
{
	// Do we have stuff we can moderate?
	ReportManager* report_manager = ReportManager::Get();
	UIButton* moderate_button = Find_Element<UIButton*>("moderate_button");
	UIButton* up_element = Find_Element<UIButton*>("ranking_button");
	UIButton* down_element = Find_Element<UIButton*>("back_button");

	bool bModerateEnabled = report_manager->Has_Queue() && report_manager->Get_Queue().size() > 0;
	moderate_button->Set_Enabled(bModerateEnabled);
	moderate_button->Set_Visible(bModerateEnabled);

	up_element->Set_Down_Element(bModerateEnabled ? moderate_button : down_element);
	down_element->Set_Up_Element(bModerateEnabled ? moderate_button : up_element);
}

void UIScene_GameSetupMenu::Show_Challenge(ChallengeTimeframe::Type timeframe)
{
	m_challenge_timeframe = timeframe;

	m_challenge_leaderboard = RankingsManager::Get()->Get_Or_Create_Leaderboard(StringHelper::Format("%s_Challenge", ChallengeTimeframe::Strings[(int)timeframe]).c_str());
	m_initial_seek = false;

	m_initial_rank_refresh = false;
	m_our_user_rank = false;
	m_total_ranks.clear();

	RankingsManager* manager = RankingsManager::Get();
	manager->Clear_Board(m_challenge_leaderboard);
	manager->Refresh_Entries(m_challenge_leaderboard, 1, TOTAL_RANKS_TO_SHOW, false);

	Update_Challenge_State();
}

void UIScene_GameSetupMenu::Update_Challenge_State()
{
	Challenge* challenge = ChallengeManager::Get()->Get_Active_Challenge(m_challenge_timeframe);
	if (challenge->active)
	{
		Find_Element<UILabel*>("challenge_title_label")->Set_Value(challenge->name);
		Find_Element<UILabel*>("challenge_description_label")->Set_Value(challenge->description);

		if (!challenge->archetype->requires_activation)
		{
			Find_Element<UIButton*>("challenge_begin_button")->Set_Visible(false);
			Find_Element<UIButton*>("challenge_begin_button")->Set_Enabled(false);
			Find_Element<UIButton*>("challenge_next_button")->Set_Left_Element(Find_Element<UIElement*>("leaderboards_button"));
			Find_Element<UIButton*>("challenge_next_button")->Set_Up_Element(Find_Element<UIElement*>("leaderboards_button"));
		}
		else
		{
			Find_Element<UIButton*>("challenge_begin_button")->Set_Visible(true);
			Find_Element<UIButton*>("challenge_begin_button")->Set_Enabled(true);
			Find_Element<UIButton*>("challenge_next_button")->Set_Left_Element(Find_Element<UIElement*>("challenge_begin_button"));
			Find_Element<UIButton*>("challenge_next_button")->Set_Up_Element(Find_Element<UIElement*>("challenge_begin_button"));
		}

		Find_Element<UIPanel*>("challenge_reward_1_bg")->Set_Visible(challenge->rewards.size() > 0);
		Find_Element<UIPanel*>("challenge_reward_2_bg")->Set_Visible(challenge->rewards.size() > 1);
		Find_Element<UIPanel*>("challenge_reward_3_bg")->Set_Visible(challenge->rewards.size() > 2);
		Find_Element<UIPanel*>("challenge_reward_4_bg")->Set_Visible(challenge->rewards.size() > 3);

		u64 days = (((challenge->time_remaining / 60) / 60) / 24);
		u64 hours = (((challenge->time_remaining / 60) / 60) % 24);
		u64 minutes = ((challenge->time_remaining / 60) % 60);
		u64 seconds = (challenge->time_remaining % 60);
		Find_Element<UILabel*>("challenge_time_remaining_label")->Set_Value(SF("#menu_game_setup_challenge_time_remaining", days, hours, minutes, seconds));

		UIProgressBar* prog_bar = Find_Element<UIProgressBar*>("challenge_progress_bar");
		if (challenge->progress >= challenge->max_progress)
		{
			prog_bar->Set_Progress(1.0f);
			prog_bar->Set_Foreground_Color(Color::Green);
			prog_bar->Set_Value(S("#menu_game_setup_challenge_completed"));
		}
		else
		{
			prog_bar->Set_Progress(challenge->progress / challenge->max_progress);
			prog_bar->Set_Foreground_Color(Color::Red);
			prog_bar->Set_Value("");
		}
	}
}

void UIScene_GameSetupMenu::Exit(UIManager* manager)
{
}	

void UIScene_GameSetupMenu::Update_Leaderboard()
{
	RankingsManager* rankings = RankingsManager::Get();
	UIListView* listview = Find_Element<UIListView*>("challenge_ranking_box");

	Challenge* challenge = ChallengeManager::Get()->Get_Active_Challenge(m_challenge_timeframe);

	std::string our_username = OnlinePlatform::Get()->Get_Initial_User()->Get_Username();

#ifdef OPT_CYCLE_CHALLENGES
	listview->Set_Faked_Item_Count(5);
#else
	// Grab the initial list.
	if (!m_initial_rank_refresh)
	{
		if (m_challenge_leaderboard->total_entries >= 0)
		{
			// TODO: This is broken, what happens if there are more than TOTAL_RANKS_TO_SHOW, we will end up with random scores
			//		 dissappearing as they all use the same ranking score. Need to keep requesting until we run out of scores of the right value.

			for (int i = 1; i <= TOTAL_RANKS_TO_SHOW; i++)
			{
				LeaderboardEntry* entry = RankingsManager::Get()->Get_Board_Entry(m_challenge_leaderboard, i);
				if (entry->score == challenge->leaderboard_target_score)
				{
					LeaderboardEntry final_entry = *entry;
					final_entry.score = final_entry.real_score;
					m_total_ranks.push_back(final_entry);
				}
			}

			std::sort(m_total_ranks.begin(), m_total_ranks.end(), &LeaderboardEntry::Sort_By_Score_Predicate);

			for (int i = 0; i < (int)m_total_ranks.size(); i++)
			{
				LeaderboardEntry& final_entry = m_total_ranks[i];
				final_entry.rank = i + 1;
				if (final_entry.name == our_username)
				{
					m_our_user_rank = final_entry.rank;
				}
			}

			listview->Set_Faked_Item_Count(Max(1, m_total_ranks.size()));
			if (m_our_user_rank >= 0)
			{
				listview->Set_Selected_Item_Index(m_our_user_rank - 1);
				listview->Scroll_To(m_our_user_rank - 1);
			}
			else
			{
				listview->Set_Selected_Item_Index(0);
				listview->Scroll_To(0);
			}

			m_initial_rank_refresh = true;
		}
	}
#endif
}

bool UIScene_GameSetupMenu::Get_Next_Broken_Item(Item*& outItem, Profile*& outProfile, int& startIndex)
{
	outItem = NULL;
	outProfile = NULL;

	ProfileManager* manager = ProfileManager::Get();

	int index = 0;

	for (int i = 0; i < OnlinePlatform::Get()->Get_Local_User_Count(); i++, index++)
	{
		OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);
		Profile* profile = manager->Get_Profile(user->Get_Profile_Index());

		std::vector<Item*> items = profile->Get_Items();
		for (unsigned int j = 0; j < items.size(); j++, index++)
		{
			Item* item = items[j];
			if (index >= startIndex)
			{
				if (item->Is_Broken() && item->equip_slot != -1)
				{
					DBG_LOG("Item '%s' broke for user '%s'.", item->Get_Display_Name().c_str(), profile->Name.c_str());

					outItem = item;
					outProfile = profile;

					startIndex = index + 1;
				
					return true;
				}
			}
		}
	}

	return false;
}

bool UIScene_GameSetupMenu::Get_Unlocked_States(Profile*& outProfile, std::vector<ItemArchetype*>& outItemUnlocks, std::vector<SkillArchetype*>& outSkillUnlocks)
{
	outProfile = NULL;
	outItemUnlocks.clear();
	outSkillUnlocks.clear();

	ProfileManager* manager = ProfileManager::Get();

	for (int i = 0; i < OnlinePlatform::Get()->Get_Local_User_Count(); i++)
	{
		OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);
		Profile* profile = manager->Get_Profile(user->Get_Profile_Index());
		
		if (profile->Item_Unlock_Events.size() > 0 || profile->Skill_Unlock_Events.size() > 0)
		{
			outProfile = profile;
			outItemUnlocks = profile->Item_Unlock_Events;
			outSkillUnlocks = profile->Skill_Unlock_Events;

			profile->Item_Unlock_Events.clear();
			profile->Skill_Unlock_Events.clear();

			return true;
		}
	}

	return false;
}

void UIScene_GameSetupMenu::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Update_Challenge_State();

	// Cycle challenges periodically.
	UIElement* focused_element = manager->Get_Focused_Element();
	if (!(focused_element->Get_Name() == "challenge_begin_button" ||
		  focused_element->Get_Name() == "challenge_next_button"  ||
		  focused_element->Get_Name() == "challenge_ranking_box"))
	{
		m_cycle_challenge_time += time.Get_Delta_Seconds();
#ifdef OPT_CYCLE_CHALLENGES
		if (m_cycle_challenge_time >= 0.6f)
#else
		if (m_cycle_challenge_time >= 10.0f)
#endif
		{
#ifdef OPT_CYCLE_CHALLENGES
			m_cycle_seed++;
#endif
			int idx = (int)m_challenge_timeframe + 1;
			if (idx >= ChallengeTimeframe::COUNT)
			{
				idx = 0;
			}
			Show_Challenge((ChallengeTimeframe::Type)idx);

			m_cycle_challenge_time = 0.0f;
		}
	}
	else
	{
		m_cycle_challenge_time = 0.0f;
	}

	Update_Leaderboard();

	// Update dat info box.
	Update_Info_Box(time, manager);

	// Do we have stuff we can moderate?
	Update_Moderate();

	// Show ban message.
	ReportManager* report_manager = ReportManager::Get();
	if (report_manager->Has_Notification() && manager->Get_Top_Scene_Index() == scene_index)
	{
		if (*GameOptions::ban_message_shown == false)
		{
			//ReportManager::Get()->Get_Notification();
			CheatReportQueueNotification notice = report_manager->Get_Notifications()[0];

			std::string cheat_types = "";
			int val = (int)notice.Category;

			if ((val & (int)CheatCategory::Exploit) != 0)
			{
				cheat_types += S("#ban_notice_exploit");
			}
			if ((val & (int)CheatCategory::Cheat) != 0)
			{
				if (cheat_types != "")
				{
					cheat_types += S("#ban_notice_type_seperator");
				}
				cheat_types += S("#ban_notice_cheat");
			}
			if ((val & (int)CheatCategory::Abuse) != 0)
			{
				if (cheat_types != "")
				{
					cheat_types += S("#ban_notice_type_seperator");
				}
				cheat_types += S("#ban_notice_abuse");
			}
			if ((val & (int)CheatCategory::Grief) != 0)
			{
				if (cheat_types != "")
				{
					cheat_types += S("#ban_notice_type_seperator");
				}
				cheat_types += S("#ban_notice_grief");
			}

			std::string message = "";
			if (notice.End_Time == "")
			{
				message = S("#ban_notice_message_indefinite");
				message = StringHelper::Format(message.c_str(),
					notice.Report_Time.c_str(),
					cheat_types.c_str()
					);
			}
			else
			{
				message = S("#ban_notice_message");
				message = StringHelper::Format(message.c_str(),
					notice.End_Time.c_str(),
					notice.Report_Time.c_str(),
					cheat_types.c_str()
					);
			}

			manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
			manager->Go(UIAction::Push(new UIScene_Dialog(message, S("#menu_continue"), true, false, "game_ban_warning"), new UIFadeInTransition()));

			*GameOptions::ban_message_shown = true;
			return;
		}
	}

	// Show inventory item.
	if (scene_index == manager->Get_Top_Scene_Index())
	{
		ChallengeManager* challenge_manager = ChallengeManager::Get();

		std::vector<UnlockDialogEntry> entries;

		if (manager->Get_Scene_By_Type<UIScene_UnlockDialog*>() == NULL)
		{
			ProfileManager::Get()->Check_Unlocks();

			// Inventory drops.
			OnlineInventoryItem item;
			while (Game::Get()->Get_Next_Inventory_Drop(item))
			{
				UnlockDialogEntry entry;
				entry.Type = UnlockDialogEntryType::Online_Item_Drop;
				entry.Online_Item = item;
				entries.push_back(entry);
			}

			// Challenge rewards.
			while (true)
			{
				ItemArchetype* archetype = challenge_manager->Get_Next_Reward();
				if (archetype != NULL)
				{
					int profile_index = OnlinePlatform::Get()->Get_Initial_User()->Get_Profile_Index();
					
					Profile* pro = ProfileManager::Get()->Get_Profile(profile_index);
					Item* item = pro->Add_Item(archetype);
					item->indestructable = false; // Challenge rewards are indestructable!

					UnlockDialogEntry entry;
					entry.Type = UnlockDialogEntryType::Challenge_Reward;
					entry.Item_Instance = item;
					entry.User_Profile = pro;
					entries.push_back(entry);
				}
				else
				{
					break;
				}
			}

			// Unlocked items and skills.
			Profile* profile = NULL;
			std::vector<ItemArchetype*> unlocked_items;
			std::vector<SkillArchetype*> unlocked_skills;

			if (Get_Unlocked_States(profile, unlocked_items, unlocked_skills))
			{
				for (std::vector<ItemArchetype*>::iterator iter = unlocked_items.begin(); iter != unlocked_items.end(); iter++)
				{
					UnlockDialogEntry entry;
					entry.Type = UnlockDialogEntryType::Item_Unlocked;
					entry.Archetype_For_Item = *iter;
					entry.User_Profile = profile;
					entries.push_back(entry);
				}
				for (std::vector<SkillArchetype*>::iterator iter = unlocked_skills.begin(); iter != unlocked_skills.end(); iter++)
				{
					UnlockDialogEntry entry;
					entry.Type = UnlockDialogEntryType::Skill_Unlocked;
					entry.Archetype_For_Skill = *iter;
					entry.User_Profile = profile;
					entries.push_back(entry);
				}
			}

			// Broken items.
			int start_index = 0;
			while (entries.size() < 1024)
			{
				Item* item = NULL;
				Profile* profile = NULL;

				if (Get_Next_Broken_Item(item, profile, start_index))
				{
					UnlockDialogEntry entry;
					entry.Type = UnlockDialogEntryType::Item_Broken;
					entry.Item_Instance = item;
					entry.User_Profile = profile;
					entries.push_back(entry);
				}
				else
				{
					break;
				}
			}

#if 0
			// DEBUG DEBUG

			entries.clear();

			ProfileManager* profile_manager = ProfileManager::Get();
			profile = profile_manager->Get_Profile(0);

			UnlockDialogEntry entry;

			entry.Type = UnlockDialogEntryType::Item_Broken;
			DBG_LOG("ITEM COUNT: %i", profile->Get_Items().size());
			entry.Item_Instance = profile->Get_Items()[0];
			entry.User_Profile = profile;
			entries.push_back(entry);

			/*
			entry.Type = UnlockDialogEntryType::Challenge_Reward;
			entry.Item = profile->Get_Items()[1];
			entry.Profile = profile;
			entries.push_back(entry);

			entry.Type = UnlockDialogEntryType::Item_Unlocked;
			entry.Archetype_For_Item = ItemManager::Get()->Find_Archetype("Item_Spaz");
			entry.Profile = profile;
			entries.push_back(entry);

			entry.Type = UnlockDialogEntryType::Skill_Unlocked;
			entry.Archetype_For_Skill = SkillManager::Get()->Find_Archetype("Skill_XP_Boost_2");
			entry.Profile = profile;
			entries.push_back(entry);

			entry.Type = UnlockDialogEntryType::Online_Item_Drop;
			entry.Online_Item.type_id = ItemManager::Get()->Find_Archetype("Item_Spaz")->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;
			entry.Profile = profile;
			entries.push_back(entry);

			entry.Type = UnlockDialogEntryType::Item_Unpacked;
			entry.Unpack_Event.amount = 1000;
			entry.Unpack_Event.type = ProfileUnpackEventType::Coins;
			entry.Profile = profile;
			entries.push_back(entry);

			entry.Type = UnlockDialogEntryType::Item_Unpacked;
			entry.Unpack_Event.amount = 1000;
			entry.Unpack_Event.type = ProfileUnpackEventType::XP;
			entry.Profile = profile;
			entries.push_back(entry);

			entry.Type = UnlockDialogEntryType::Item_Unpacked;
			entry.Unpack_Event.amount = 1;
			entry.Unpack_Event.type = ProfileUnpackEventType::Item;
			entry.Unpack_Event.item = ItemManager::Get()->Find_Archetype("Item_Spaz");
			entry.Profile = profile;
			entries.push_back(entry);
			*/

			// DEBUG DEBUG

#endif

			if (entries.size() > 0)
			{
				Game::Get()->Queue_Save();
				manager->Go(UIAction::Push(new UIScene_UnlockDialog(entries), new UIFadeInTransition()));
			}
		}

		// Show seasonal challenge announcements.
		if (scene_index == manager->Get_Top_Scene_Index())
		{
			if (manager->Get_Scene_By_Type<UIScene_ChallengeAnnouncement*>() == NULL)
			{
				ChallengeAnnouncement announcement;
				if (challenge_manager->Get_Announcement(&announcement))
				{
					manager->Go(UIAction::Push(new UIScene_ChallengeAnnouncement(announcement), new UIFadeInTransition()));
				}
			}
		}


		/*
		// Show challenge rewards
		if (manager->Get_Scene_By_Type<UIScene_InventoryDropDialog*>() == NULL)
		{
			OnlineInventoryItem item;
			if (Game::Get()->Get_Next_Inventory_Drop(item))
			{
				manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
				manager->Go(UIAction::Push(new UIScene_InventoryDropDialog(item), new UIFadeInTransition()));
				return;
			}
			else
			{
				ItemArchetype* archetype = challenge_manager->Get_Next_Reward();
				if (archetype != NULL)
				{
					int profile_index = OnlinePlatform::Get()->Get_Initial_User()->Get_Profile_Index();
					Item* item = ProfileManager::Get()->Get_Profile(profile_index)->Add_Item(archetype);
					item->indestructable = true; // Challenge rewards are indestructable!
					manager->Go(UIAction::Push(new UIScene_InventoryDropDialog(archetype), new UIFadeInTransition()));
					return;
				}
			}
		}

		// Show level unlocks.
		if (manager->Get_Scene_By_Type<UIScene_UnlockDialog*>() == NULL)
		{
			Profile* profile = NULL;
			std::vector<ItemArchetype*> unlocked_items;
			std::vector<SkillArchetype*> unlocked_skills;

			if (Get_Unlocked_States(profile, unlocked_items, unlocked_skills))
			{
				manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
				manager->Go(UIAction::Push(new UIScene_UnlockDialog(profile, unlocked_items, unlocked_skills), new UIFadeInTransition()));
				return;
			}
		}

		// Show broken item.
		if (manager->Get_Scene_By_Type<UIScene_ItemBrokenDialog*>() == NULL)
		{
			Item* item = NULL;
			Profile* profile = NULL;

			if (Get_Next_Broken_Item(item, profile))
			{
				manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
				manager->Go(UIAction::Push(new UIScene_ItemBrokenDialog(item, profile), new UIFadeInTransition()));
				return;			
			}
		}
		*/
	}

	// Auto load map.
	if (*GameOptions::editor == true || 
		*GameOptions::map != "" ||
		*EngineOptions::connect != 0)
	{
		std::string map = *GameOptions::map;
		bool open_editor = *GameOptions::editor;
		u64 connect_id = *EngineOptions::connect;

		// Disable options so we don't get circular loops if we get any errors.
		*GameOptions::editor = false;
		*GameOptions::map = "";

		MapFileHandle* map_file = ResourceFactory::Get()->Get_Map_File_By_Short_Name(map.c_str());
		if (map_file == NULL)
		{
			map = *EngineOptions::server_safe_map_name;
			map_file = ResourceFactory::Get()->Get_Map_File_By_Short_Name(map.c_str());

			if (map_file == NULL)
			{
				DBG_LOG("Failed to load map, map passed on command line '%s', dosen't exist.", map.c_str());
				manager->Go(UIAction::Pop(new UISlideInTransition()));
				return;
			}
		}

		// TODO: Needs abstracting.
		if (connect_id != 0)
		{			
			Steamworks_OnlinePlatform* client = static_cast<Steamworks_OnlinePlatform*>(OnlinePlatform::Get());
			Steamworks_OnlineUser* user = client->Get_User_By_SteamID(connect_id);
			if (user == NULL)
			{
				user = client->Register_Remote_User(connect_id);
			}

			OnlineMatching_Server server;
			server.Ping				= 0;
			server.Name				= "Connect";
			server.Private			= false;
			server.Secure			= false;
			server.PlayerCount		= 0;
			server.MaxPlayerCount	= 8;
			server.Map				= "";
			server.Address			= NetAddress(0, 0);
			server.User				= user;

			manager->Go(UIAction::Push(new UIScene_ConnectToServer(server), new UIFadeInTransition()));
		}
		else
		{
			manager->Go(UIAction::Push(new UIScene_HostGame(map_file, 16, true, false, true, true, open_editor), new UISlideInTransition()));
		}
		return;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_GameSetupMenu::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_direction_change_timer += time.Get_Frame_Time();
	if (m_direction_change_timer >= DIRECTION_CHANGE_INTERVAL)
	{
		m_direction = (m_direction + 1) % 8;
		m_direction_change_timer = 0.0f;
	}

	UIScene::Draw(time, manager, scene_index);
}

void UIScene_GameSetupMenu::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
			/*
			if (e.Source->Get_Name() == "find_game_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_find_game_description"));
			}
			else if (e.Source->Get_Name() == "quick_game_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_quick_game_description"));
			}
			else if (e.Source->Get_Name() == "host_game_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_host_game_description"));
			}
			else if (e.Source->Get_Name() == "server_browser_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_browse_servers_description"));
			}
			else if (e.Source->Get_Name() == "inventory_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_inventory_description"));
			}
			else if (e.Source->Get_Name() == "global_chat_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_global_chat_description"));
			}
			else if (e.Source->Get_Name() == "achievements_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_achievements_description"));
			}
			else if (e.Source->Get_Name() == "leaderboards_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_leaderboards_description"));
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				Find_Element<UILabel*>("description_label")->Set_Value(S("#menu_game_setup_back_description"));
			}
			*/
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "find_game_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_FindGameSetup(), new UIFadeInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "quick_game_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_FindGame(NULL, true, true), new UIFadeInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "host_game_button")
			{
				manager->Go(UIAction::Push(new UIScene_HostGameSetup(), new UIFadeInTransition()));
			}
			else if (e.Source->Get_Name() == "server_browser_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_ServerBrowser(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "moderate_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_Moderation(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "inventory_button")
			{
				OnlineUser* user = e.Source_User;
				if (user != NULL)
				{
					Profile* profile = ProfileManager::Get()->Get_Profile(user->Get_Profile_Index());
					if (profile != NULL)
					{
						manager->Go(UIAction::Push(new UIScene_Inventory(profile, false), new UISlideInTransition()));		
					}
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#no_user_assigned_to_input_device")), new UIFadeInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "achievements_button")
			{
				manager->Go(UIAction::Push(new UIScene_Achievements(), new UISlideInTransition()));
			}
			else if (e.Source->Get_Name() == "global_chat_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_GlobalChat(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "leaderboards_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_Leaderboards(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "ranking_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_RanksAndStats(), new UISlideInTransition()));
				}
			}
			else if (e.Source->Get_Name() == "statistics_button")
			{
				if (!OnlinePlatform::Get()->Is_Online())
				{
					manager->Go(UIAction::Push(new UIScene_Dialog(S("#online_platform_feature_not_available_offline")), new UIFadeInTransition()));
				}
				else
				{
					manager->Go(UIAction::Push(new UIScene_Statistics(), new UISlideInTransition()));		
				}
			}
			else if (e.Source->Get_Name() == "challenge_begin_button")
			{
				ChallengeManager::Get()->Activate_Challenge(m_challenge_timeframe);
			}
			else if (e.Source->Get_Name() == "challenge_next_button")
			{
				int idx = (int)m_challenge_timeframe + 1;
				if (idx >= ChallengeTimeframe::COUNT)
				{
					idx = 0;
				}
				Show_Challenge((ChallengeTimeframe::Type)idx);
			}
			else if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}

void UIScene_GameSetupMenu::On_Draw_Item(UIPanelDrawItem* data)
{
	unsigned int index = 0;
	if (data->view->Get_Name() == "challenge_reward_1_bg")
	{
		index = 0;
	}
	else if (data->view->Get_Name() == "challenge_reward_2_bg")
	{
		index = 1;
	}
	else if (data->view->Get_Name() == "challenge_reward_3_bg")
	{
		index = 2;
	}
	else if (data->view->Get_Name() == "challenge_reward_4_bg")
	{
		index = 3;
	}

	Challenge* challenge = ChallengeManager::Get()->Get_Active_Challenge(m_challenge_timeframe);
	if (!challenge->active || challenge->rewards.size() <= index)
	{
		return;
	}

	Vector2 ui_scale = data->ui_scale;
	ui_scale.X *= 0.60f;
	ui_scale.Y *= 0.60f;

	ItemArchetype* item = challenge->rewards[index];

	Vector2 position = Vector2
	(
		data->view_bounds.X + (data->view_bounds.Width * 0.5f) - (item->icon_offset.X * ui_scale.X),
		data->view_bounds.Y + (data->view_bounds.Height * 0.5f) - (item->icon_offset.Y * ui_scale.Y)
	);

	AtlasRenderer atlas_renderer(NULL);
	if (item->is_icon_direction_based && item->item_slot != 1)
	{
		if (item->is_tintable)
		{
			atlas_renderer.Draw_Frame(item->icon_animations[m_direction]->Frames[0], position, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
			atlas_renderer.Draw_Frame(item->icon_tint_animations[m_direction]->Frames[0], position, 0.0f, item->default_tint, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
		else
		{
			atlas_renderer.Draw_Frame(item->icon_animations[m_direction]->Frames[0], position, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
	}
	else
	{
		if (item->is_tintable)
		{
			atlas_renderer.Draw_Frame(item->icon_animations[0]->Frames[0], position, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
			atlas_renderer.Draw_Frame(item->icon_tint_animations[0]->Frames[0], position, 0.0f, item->default_tint, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
		else
		{
			atlas_renderer.Draw_Frame(item->icon_animations[0]->Frames[0], position, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale);
		}
	}
}

void UIScene_GameSetupMenu::Update_Info_Box(const FrameTime& time, UIManager* manager)
{
	Vector2 ui_scale = manager->Get_UI_Scale();

	UIPanel* info_panel = Find_Element<UIPanel*>("info_panel");
	UILabel* info_name_label = Find_Element<UILabel*>("info_name_label");
	UILabel* info_price_label = Find_Element<UILabel*>("info_price_label");
	UILabel* info_description_label = Find_Element<UILabel*>("info_description_label");

	ProfileManager* profile_manager = ProfileManager::Get();

	UIPanel* reward_1_bg = Find_Element<UIPanel*>("challenge_reward_1_bg");
	UIPanel* reward_2_bg = Find_Element<UIPanel*>("challenge_reward_2_bg");
	UIPanel* reward_3_bg = Find_Element<UIPanel*>("challenge_reward_3_bg");
	UIPanel* reward_4_bg = Find_Element<UIPanel*>("challenge_reward_4_bg");

	Vector2 mouse_pos = Input::Get()->Get_Mouse_State()->Get_Position();

	int item_index = -1;
	Rect2D item_box;

	if (reward_1_bg->Get_Screen_Box().Intersects(mouse_pos))
	{
		item_index = 0;
		item_box = reward_1_bg->Get_Screen_Box();
	}
	else if (reward_2_bg->Get_Screen_Box().Intersects(mouse_pos))
	{
		item_index = 1;
		item_box = reward_2_bg->Get_Screen_Box();
	}
	else if (reward_3_bg->Get_Screen_Box().Intersects(mouse_pos))
	{
		item_index = 2;
		item_box = reward_3_bg->Get_Screen_Box();
	}
	else if (reward_4_bg->Get_Screen_Box().Intersects(mouse_pos))
	{
		item_index = 3;
		item_box = reward_4_bg->Get_Screen_Box();
	}
	
	if (item_index == -1)
	{
		m_item_select_time = 0.0f;
	}
	else
	{
		m_item_select_time += time.Get_Delta_Seconds();
	}

	Challenge* challenge = ChallengeManager::Get()->Get_Active_Challenge(m_challenge_timeframe);

	// Show info dialog if we hover long enough.
	if (item_index >= 0 && m_item_select_time * 1000 >= INFO_FADE_IN_DELAY && item_index < (int)challenge->rewards.size())
	{
		m_info_alpha = Math::Lerp(m_info_alpha, 1.0f, 0.05f);
	}
	else
	{
		m_info_alpha = 0.0f;
	}

	// Position info next to the info box.
	float screen_width = (float)GfxDisplay::Get()->Get_Width();
	float screen_height = (float)GfxDisplay::Get()->Get_Height();

	Rect2D info_box = info_panel->Get_Screen_Box();

	bool left = (item_box.X + (item_box.Width * 0.5f)) <= (screen_width * 0.5f);
	bool top = (item_box.Y + (item_box.Height * 0.5f)) <= (screen_height * 0.5f);

	float spacing = 5.0f * ui_scale.Y;

	// Top-Left
	if (left == true && top == true)
	{
		info_panel->Reposition(Rect2D(item_box.X, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
	}
	// Top-Right
	else if (left == false && top == true)
	{
		info_panel->Reposition(Rect2D(item_box.X + item_box.Width - info_box.Width, item_box.Y + item_box.Height + spacing, info_box.Width, info_box.Height));
	}
	// Bottom-Left
	else if (left == true && top == false)
	{
		info_panel->Reposition(Rect2D(item_box.X, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
	}
	// Bottom-Right
	else if (left == false && top == false)
	{
		info_panel->Reposition(Rect2D(item_box.X + item_box.Width - info_box.Width, (item_box.Y - info_box.Height) - spacing, info_box.Width, info_box.Height));
	}

	info_panel->Set_Frame_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_name_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));
	info_description_label->Set_Color(Color(255.0f, 255.0f, 255.0f, 220.0f * m_info_alpha));

	// Show info.
	if (challenge->active && item_index >= 0 && item_index < (int)challenge->rewards.size())
	{
		ItemArchetype* archetype = challenge->rewards.at(item_index);
		info_name_label->Set_Value(archetype->Get_Fragile_Display_Name());

		std::string description = archetype->Get_Description(false);
		info_description_label->Set_Value(description);
	}
}

void UIScene_GameSetupMenu::On_Get_Item_Data(UIListViewGetItemData* data)
{
	data->item->Values.resize(4);

#ifdef OPT_CYCLE_CHALLENGES
	static const char* RandomFlags[] = {
		"en",
		"de",
		"fr",
		"it",
		"ca",
		"gb",
		"jp"
	};
	static int RandomFlagsCount = sizeof(RandomFlags) / sizeof(*RandomFlags);
	static const char* RandomNames[] = {
		"Infinitus",
		"Jordizzle",
		"TripCharge",
		"TacoKnight",
		"NightHawk13",
		"Wizard",
	};
	static int RandomNamesCount = sizeof(RandomNames) / sizeof(*RandomNames);

	Random rnd(m_cycle_seed + data->index);
	data->item->Values.at(0) = StringHelper::Lowercase(StringHelper::Format("[img=flag_%s,0.6,0.6,0,1.5]", RandomFlags[rnd.Next(0, RandomFlagsCount)]).c_str());
	data->item->Values.at(1) = StringHelper::Format("%i", data->index + 1);
	data->item->Values.at(2) = RandomNames[rnd.Next(0, RandomNamesCount)];
	data->item->Values.at(3) = StringHelper::Format_Number((float)(data->index * 1000) + rnd.Next(1, 800));
	return;
#endif

	if (m_total_ranks.size() == 0 && m_initial_rank_refresh)
	{
		data->item->Values.at(0) = "";
		data->item->Values.at(1) = "";
		data->item->Values.at(2) = S("#menu_leaderboards_no_entries_short_text");
		data->item->Values.at(3) = "";
	}
	else
	{
		LeaderboardEntry* entry = (data->index >= (int)m_total_ranks.size()) ? NULL : &m_total_ranks[data->index];
		if (data->index < (int)m_total_ranks.size() &&
			entry->status == LeaderboardEntryStatus::Loaded)
		{
			const char* flag_id = GeoIP_code_by_id(entry->country);
			if (flag_id != NULL)
				data->item->Values.at(0) = StringHelper::Lowercase(StringHelper::Format("[img=flag_%s,0.6,0.6,0,1.5]", flag_id).c_str());
			else
				data->item->Values.at(0) = "";

			data->item->Values.at(1) = StringHelper::Format("%i", entry->rank);

			std::string name = MarkupFontRenderer::Escape(entry->name);

			if (data->index + 1 == m_our_user_rank)
			{
				data->item->Values.at(2) = StringHelper::Format("[c=0,200,200,255]%s[/c]", entry->name.c_str());
			}
			else
			{
				data->item->Values.at(2) = name;
			}

			data->item->Values.at(3) = StringHelper::Format_Number((float)entry->score);
		}
		else
		{
			data->item->Values.at(0) = "";
			data->item->Values.at(1) = "";
			data->item->Values.at(2) = S("#menu_leaderboards_loading_text");
			data->item->Values.at(3) = "";
		}
	}
}
