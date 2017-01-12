// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_GameOver.h"
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Chat.h"
#include "Game/UI/Scenes/UIScene_SubMenu.h"
#include "Game/UI/Scenes/UIScene_Credits.h"
#include "Game/UI/Scenes/UIScene_Scoreboard.h"
#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/UIScene_InventoryDropDialog.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetUserState.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Profile/ProfileManager.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"
#include "Game/Online/RankingsManager.h"

#include "Engine/Online/OnlineRankings.h"

#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISimpleListView.h"
#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetUser.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Localise/Locale.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/Blocks/MapFileHeaderBlock.h"

#include "Generic/Math/Random.h"

#include "LibGeoIP/GeoIP.h"

UIScene_GameOver::UIScene_GameOver(std::vector<GameOverBonus>& bonuses, std::string title, std::string destination, bool show_black_bg)
	: m_bonuses(bonuses)
	, m_bgm_volume(1.0f)
	, m_bonus_display_index(0)
	, m_bonus_display_time(0.0f)
	, m_restart_timer(restart_countdown)
	, m_has_restarted_map(false)
	, m_destination(destination)
	, m_show_black_bg(show_black_bg)
{
	m_font_renderer = MarkupFontRenderer(GameEngine::Get()->Get_UIManager()->Get_Font(), false, true);

	m_is_pvp = Game::Get()->Get_Game_Mode()->Is_PVP();

	if (m_is_pvp)
	{
		Set_Layout("GameOverPVP");
	}
	else
	{
		Set_Layout("GameOver");
	}

	if (title != "")
	{
		Find_Element<UILabel*>("game_over_label")->Set_Value(title);
	}

	// If using full black bg then override the default alpha!
	if (show_black_bg)
	{
		Find_Element<UIPanel*>("background")->Set_Background_Color(Color::Black);
	}

	DBG_LOG("Starting game over with destination '%s'", m_destination.c_str());

	int total_score = 0;
	int total_coins = 0;
	int total_skill_points = 0;

	std::vector<NetUser*> users = GameNetManager::Get()->Get_Local_Net_Users();
	DBG_LOG("Iterating over %i local users.", users.size());
	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		DBG_LOG("User: 0x%08x", *iter);
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		int user_coins = user->Get_State()->Coins;

		// Apply coins gathered to profile.
		if (!Game::Get()->In_Restricted_Mode())
		{
			Profile* profile = ProfileManager::Get()->Get_Profile(user->Get_Online_User()->Get_Profile_Index());
			profile->Coins += user_coins;

			// Only show local users score on main screen.
			if (user->Get_Local_User_Index() == 0)
			{
				int initial_level = user->Get_Initial_Level();

				total_coins = user_coins;
				total_skill_points = (profile->Level - initial_level) * ProfileManager::Get()->Get_Skill_Points_Per_Level();
			}
		}
	}

	for (std::vector<GameOverBonus>::iterator iter = m_bonuses.begin(); iter != m_bonuses.end(); iter++)
	{
		GameOverBonus& b = *iter;
		total_score += b.value;
	}

	// Pity bonus for you!
	if (total_score <= 0)
	{
		GameOverBonus bonus;
		bonus.name = S("#game_over_bonus_pity");
		bonus.value = 1;
		bonus.scale = 1.0f;
		m_bonuses.push_back(bonus);		

		total_score += bonus.value;
	}

	// Add the "total" bonus
	GameOverBonus bonus;
	bonus.name = "";
	bonus.value = total_score;
	bonus.scale = 1.25f;
	m_bonuses.push_back(bonus);

	// Add spacer.
	bonus.name = "";
	bonus.value = 0;
	bonus.scale = 1.0f;
	m_bonuses.push_back(bonus);

	// Add the "earned x gold / skills" bonuses.
//	if (total_coins > 0)
	{
		bonus.name = S("#game_over_coins_earnt");
		bonus.value_format = S("#game_over_coins_earnt_value_format");
		bonus.value = total_coins;
		bonus.scale = 1.0f;
		m_bonuses.push_back(bonus);
	}

//	if (total_skill_points > 0)
	{
		bonus.name = S("#game_over_skill_points_earnt");
		bonus.value_format = S("#game_over_skill_points_earnt_value_format");
		bonus.value = total_skill_points;
		bonus.scale = 1.0f;
		m_bonuses.push_back(bonus);
	}

	// Setup list view.
	m_get_item_delegate = new Delegate<UIScene_GameOver, UIListViewGetItemData>(this, &UIScene_GameOver::On_Get_Item_Data);

	if (!m_is_pvp)
	{
		UIListView* listview = Find_Element<UIListView*>("leaderboard_entry_listview");
		listview->Clear_Columns();
		listview->Add_Column("", 0.1f, false, true);
		listview->Add_Column(S("#menu_leaderboards_rank_column"), 0.15f);
		listview->Add_Column(S("#menu_leaderboards_name_column"), 0.45f, true, true);
		listview->Add_Column(S("#menu_leaderboards_score_column"), 0.30f);

		listview->On_Get_Item_Data += m_get_item_delegate;

		if (OnlineRankings::Try_Get() != NULL)
		{
			// Grab map leaderboard.
			m_leaderboard = RankingsManager::Get()->Get_Map_Leaderboard(Game::Get()->Get_Map()->Get_Map_Header().Short_Name.c_str());
	
			// Post our leaderboard entry.
			if (m_leaderboard != NULL)
			{
				RankingsManager::Get()->Submit(m_leaderboard, total_score);
			}
		}

		Refresh_Board();
	}
	else
	{		
		UISimpleListView* list_view = Find_Element<UISimpleListView*>("leaderboard_entry_listview");

		if (!NetManager::Get()->Is_Dedicated_Server())
		{
			UIScene_Scoreboard::Update_Scoreboard(list_view);
		}
	}

	GameNetManager::Get()->Update_User_Scoreboard_Ranks();

	// Update game over score with final.
	DBG_LOG("Game over score, score to persist to next level is %i", total_score);
	Game::Get()->Get_Game_Mode()->Set_Game_Over_Score(total_score);

	// Fire game over state.
	ScriptEventListener::Fire_On_Game_Over();

	// Save profiles.
	Game::Get()->Queue_Save();

	// Check for item drops!
	Game::Get()->Check_Inventory_Drops();
}

UIScene_GameOver::~UIScene_GameOver()
{
}

void UIScene_GameOver::Refresh_Board()
{
	UIListView* listview = Find_Element<UIListView*>("leaderboard_entry_listview");
	listview->Set_Faked_Item_Count(refresh_range);
	listview->Set_Selected_Item_Index(0);

	m_initial_seek = false;

	if (OnlineRankings::Try_Get() != NULL)
	{
		RankingsManager* manager = RankingsManager::Get();
		if (m_leaderboard != NULL)
		{
			manager->Clear_Board(m_leaderboard);
			manager->Get_Own_Entry(m_leaderboard);
		}
	}
}

bool UIScene_GameOver::Should_Tick_When_Not_Top()
{
	return true;
}

const char* UIScene_GameOver::Get_Name()
{
	return "UIScene_GameOver";
}

bool UIScene_GameOver::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_GameOver::Should_Render_Lower_Scenes_Background()
{
	return true;
}

bool UIScene_GameOver::Is_Focusable()
{
	return false;
}

bool UIScene_GameOver::Should_Display_Cursor()
{
	return false;
}

void UIScene_GameOver::Enter(UIManager* manager)
{
	AudioRenderer::Get()->Set_BGM_Volume_Multiplier(0.0f);
}	

void UIScene_GameOver::Exit(UIManager* manager)
{
	AudioRenderer::Get()->Set_BGM_Volume_Multiplier(1.0f);
}	

void UIScene_GameOver::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	bool is_topmost	= (manager->Get_Top_Scene_Index() == scene_index);

	if (is_topmost == true)
	{
		UIScene_Game* game_scene = manager->Get_Scene_By_Type<UIScene_Game*>();

		// Open the main sub-menu.
		if ((Input::Get()->Was_Pressed_By_Any(OutputBindings::GUI_Back) ||
			Input::Get()->Was_Pressed_By_Any(OutputBindings::Escape)))
		{
			manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
			manager->Go(UIAction::Push(new UIScene_SubMenu(), new UISlideInTransition()));
			return;
		}

		// Open the chat menu.
		if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Chat))
		{
			manager->Play_UI_Sound(UISoundType::Small_Click);
			manager->Go(UIAction::Push(new UIScene_Chat(game_scene, false), new UIFadeInTransition()));
			return;
		}

		// Open the scoreboard.
		if (Input::Get()->Was_Pressed_By_Any(OutputBindings::Scoreboard))
		{
			manager->Play_UI_Sound(UISoundType::Small_Click);
			manager->Go(UIAction::Push(new UIScene_Scoreboard(), new UIFadeInTransition()));
			return;
		}
	}

	// Show inventory item.
	/*if (manager->Get_Scene_By_Type<UIScene_InventoryDropDialog*>() == NULL)
	{
		OnlineInventoryItem item;
		if (Game::Get()->Get_Next_Inventory_Drop(item))
		{
			manager->Play_UI_Sound(UISoundType::Enter_SubMenu);
			manager->Go(UIAction::Push(new UIScene_InventoryDropDialog(item), new UIFadeInTransition()));
			return;
		}
	}*/

	// Restart text.
	bool any_in_clients_cutscenes = GameNetManager::Get()->Any_Users_In_Cutscene();
	if (any_in_clients_cutscenes)
	{
		Find_Element<UILabel*>("restart_label")->Set_Value(S("#game_over_waiting_for_cutscenes"));
	}
	else
	{
		m_restart_timer = Max(0.0f, m_restart_timer - time.Get_Frame_Time());
		if (m_destination != "")
		{
			Find_Element<UILabel*>("restart_label")->Set_Value(SF("#game_over_continue_text", (int)ceilf(m_restart_timer / 1024)));
		}
		else
		{
			Find_Element<UILabel*>("restart_label")->Set_Value(SF("#game_over_restart_text", (int)ceilf(m_restart_timer / 1024)));
		}
	}

	if (NetManager::Get()->Server() != NULL)
	{
		if (m_restart_timer <= 0 && m_has_restarted_map == false)
		{
			if (m_destination == "next_dungeon_level")
			{
				Game::Get()->Get_Game_Mode()->Persist_State();

				NetManager::Get()->Server()->Force_Map_Change(
					NetManager::Get()->Get_Current_Map_GUID().c_str(), 
					NetManager::Get()->Get_Current_Map_Workshop_ID(), 
					Random::Static_Next(),
					NetManager::Get()->Get_Current_Map_Dungeon_Level() + 1);
			}
			else if (m_destination == "credits")
			{
				// If local-hosting, go to credits.
				if (NetManager::Get()->Client() != NULL)
				{
					DBG_LOG("Campaign finished, heading to credits.");

					NetManager::Get()->Leave_Network_Game();

					manager->Go(UIAction::Pop(NULL, "UIScene_MainMenu"));
					manager->Go(UIAction::Push(new UIScene_Credits()));
				}
			}
			else if (m_destination != "")
			{
				MapFileHandle* map_handle = ResourceFactory::Get()->Get_Map_File_By_Short_Name(m_destination.c_str());
				if (map_handle != NULL)
				{
					NetManager::Get()->Server()->Force_Map_Change(
						map_handle->Get()->Get_Header()->GUID.c_str(), 
						map_handle->Get()->Get_Header()->Workshop_ID, 
						Random::Static_Next(),
						0);
				}
				else
				{
					NetManager::Get()->Server()->Restart_Map();
				}
			}
			else
			{
				NetManager::Get()->Server()->Restart_Map();
			}

			m_has_restarted_map = true;

			if (NetManager::Get()->Is_Dedicated_Server())
			{
				manager->Go(UIAction::Pop(NULL, "UIScene_Server"));
			}

			return;
		}
	}
	else
	{
		if (m_restart_timer <= 0 && m_has_restarted_map == false)
		{
			if (NetManager::Get()->Is_Dedicated_Server())
			{
				NetManager::Get()->Server()->Restart_Map();

				manager->Go(UIAction::Pop(NULL, "UIScene_Server"));
			}
			else if (m_destination == "credits")
			{
				DBG_LOG("Campaign finished, heading to credits.");

				NetManager::Get()->Leave_Network_Game();

				manager->Go(UIAction::Pop(NULL, "UIScene_MainMenu"));
				manager->Go(UIAction::Push(new UIScene_Credits()));
			}

			m_has_restarted_map = true;
			return;
		}
	}

	// Make sure we have our own entry before we try to refresh 
	// other entries.
	if (!m_is_pvp && m_leaderboard != NULL && OnlineRankings::Try_Get() != NULL)
	{
		UIListView* listview = Find_Element<UIListView*>("leaderboard_entry_listview");
		RankingsManager* rankings = RankingsManager::Get();

		LeaderboardEntry* own_entry = rankings->Get_Own_Entry(m_leaderboard);
		if (own_entry != NULL && own_entry->status == LeaderboardEntryStatus::Loaded)
		{
			// Seek to our entry in the leaderboard.
			if (!m_initial_seek)
			{
				// No rank.
				if (own_entry->rank <= 0)
				{
					// Get the first rankings.
					rankings->Refresh_Entries(m_leaderboard, 1, refresh_range * 2, false);
				}
				else
				{
					// Zoom scroll so our rank is at the top.				
					listview->Set_Faked_Item_Count(own_entry->rank + (refresh_range * 2));
					listview->Set_Selected_Item_Index(own_entry->rank - 1);
					listview->Scroll_To(own_entry->rank - 1);

					// Get all the entries around our current one.
					rankings->Refresh_Entries(m_leaderboard, own_entry->rank, refresh_range * 2, false);
				}

				m_initial_seek = true;
			}

			// Refresh the areas we are viewing.
			else
			{
				// Work out range. If we have found the end then set range to 0-max otherwise set it to 0-total_downloaded + refresh_range
				int total_entries = m_leaderboard->total_entries;
				if (total_entries < 0)
				{
					total_entries = m_leaderboard->highest_entry + refresh_range;
				}
				listview->Set_Faked_Item_Count(Max(1, total_entries)); // We always need 1 entry to show the "no entries" text.

				// If unloaded ranks are in view, get loading!
				int scroll_position = listview->Get_Scroll_Offset();
				if (total_entries > 0)
				{
					int start_offset = -1;
					int end_offset = -1;

					for (int i = scroll_position + 1; i < (scroll_position + refresh_range) + 1; i++)
					{
						if (total_entries >= 0)
						{
							if (i >= total_entries)
							{
								break;
							}
						}

						LeaderboardEntry* entry = rankings->Get_Board_Entry(m_leaderboard, i);
						if (entry->status == LeaderboardEntryStatus::Unloaded)
						{
							if (start_offset == -1)
							{
								start_offset = i;
								end_offset = i;

								// Load future ones as well.
								if (end_offset == (scroll_position + refresh_range))
								{
									end_offset += refresh_range;
								}
							}
							else
							{
								end_offset = i;
							}

						}
					}

					if (start_offset >= 0 && !listview->Is_Scrolling())
					{
						DBG_LOG("Refreshing rankings entries %i-%i", start_offset, end_offset);
						rankings->Refresh_Entries(m_leaderboard, start_offset, (end_offset - start_offset) + 1, false);
					}
				}
			}
		}
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_GameOver::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene_Game* game_scene = manager->Get_Scene_By_Type<UIScene_Game*>();
	if (game_scene != NULL)
	{
		game_scene->Set_Overlay_Enabled(false);
	}

	UIScene::Draw(time, manager, scene_index);

	if (game_scene != NULL)
	{
		game_scene->Set_Overlay_Enabled(true);
	}

	if (!m_is_pvp)
	{
		// Draw the bonuses.
		UIElement* element = Find_Element<UIPanel*>("bonus_area");
		Rect2D bonus_area = element->Get_Screen_Box();

		Vector2 ui_scale = manager->Get_UI_Scale();

		float offset = 0.0f;
		float bonus_height = 10.0f * ui_scale.Y;

		int index = 0;

		if (m_bonus_display_index < (int)m_bonuses.size())
		{
			GameOverBonus& bonus = m_bonuses.at(m_bonus_display_index);

			m_bonus_display_time += time.Get_Frame_Time();
			if (m_bonus_display_time >= bonus_display_time * 2)
			{
				if (bonus.value != 0 || bonus.name != "")
				{
				//	manager->Play_UI_Sound(UISoundType::Thud);
				}
				m_bonus_display_time = 0.0f;
				m_bonus_display_index++;
			}
		}

		float bonus_anim_delta = Clamp(m_bonus_display_time / (float)bonus_display_time, 0.0f, 1.0f);

		for (std::vector<GameOverBonus>::iterator iter = m_bonuses.begin(); iter != m_bonuses.end() && index <= m_bonus_display_index; iter++, index++)
		{
			GameOverBonus& bonus = *iter;

			float height = bonus_height * bonus.scale;
			Rect2D area = Rect2D(bonus_area.X, bonus_area.Y + offset, bonus_area.Width, height);

			if (index == m_bonus_display_index)
			{
				float h_padding = (area.Width * 6) * (1.0f - bonus_anim_delta);
				float v_padding = (area.Height * 6) * (1.0f - bonus_anim_delta);

				area.X -= h_padding;
				area.Y -= v_padding;
				area.Width += (h_padding * 2);
				area.Height += (v_padding * 2);
			}

			if (bonus.name != "" || bonus.value != 0)
			{
				m_font_renderer.Draw_String(
					bonus.name.c_str(),
					area,
					bonus_height,
					Color::White,
					TextAlignment::Left,
					TextAlignment::ScaleToFit,
					ui_scale
				);

				std::string value_formatted = StringHelper::Format_Number((float)bonus.value);

				m_font_renderer.Draw_String(
					StringHelper::Format(bonus.value_format.c_str(), value_formatted.c_str()).c_str(),
					area,
					bonus_height,
					Color::White,
					TextAlignment::Right,
					TextAlignment::ScaleToFit,
					ui_scale
				);
			}

			offset += height;
		}
	}

	if (game_scene != NULL)
	{
		game_scene->Draw_Overlays(time, manager, scene_index);
	}
}

void UIScene_GameOver::On_Get_Item_Data(UIListViewGetItemData* data)
{
	data->item->Values.resize(4);

	if (m_leaderboard == NULL || OnlineRankings::Try_Get() == NULL)
	{
		data->item->Values.at(0) = "";
		data->item->Values.at(1) = "";
		data->item->Values.at(2) = S("#menu_leaderboards_no_entries_text");
		data->item->Values.at(3) = "";
		return;
	}

	LeaderboardEntry* own_entry = RankingsManager::Get()->Get_Own_Entry(m_leaderboard);
	LeaderboardEntry* entry = RankingsManager::Get()->Get_Board_Entry(m_leaderboard, data->index + 1);

	if (m_leaderboard->total_entries == 0)
	{
		data->item->Values.at(0) = "";
		data->item->Values.at(1) = "";
		data->item->Values.at(2) = S("#menu_leaderboards_no_entries_text");
		data->item->Values.at(3) = "";
	}
	else if (entry->status == LeaderboardEntryStatus::Loaded)
	{
		const char* flag_id = GeoIP_code_by_id(entry->country);
		if (flag_id != NULL)
			data->item->Values.at(0) = StringHelper::Lowercase(StringHelper::Format("[img=flag_%s,1,1,0,0]", flag_id).c_str());
		else
			data->item->Values.at(0) = "";

		data->item->Values.at(1) = StringHelper::Format("%i", entry->rank);

		std::string name = MarkupFontRenderer::Escape(entry->name);

		if (entry->rank == own_entry->rank &&
			entry->name == own_entry->name)
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