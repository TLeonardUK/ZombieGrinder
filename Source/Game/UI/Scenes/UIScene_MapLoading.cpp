// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_MapLoading.h"
#include "Game/UI/Scenes/UIScene_MainMenu.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_OpenWorkshopDialog.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_LoadingBackground.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeOutToNewTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"
#include "Engine/Network/Packets/EnginePackets.h"

#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/ActorReplicator.h"
#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Online/OnlineMods.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/EngineOptions.h"

#include "Engine/Particles/ParticleManager.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Game/UI/Scenes/UIScene_Video.h"
#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Server.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"

#include "Game/Game/EditorManager.h"
//#include "Game/UI/Scenes/UIScene_SetupServer.h"

#include "Engine/Online/OnlineAchievements.h"

#include "Engine/Tasks/TaskManager.h"

#include "Engine/Localise/Locale.h"

#include "Game/Runner/Game.h"

UIScene_MapLoading::UIScene_MapLoading()
	: m_set_style(false)
	, m_tip_timer(0.0f)
	, m_loading_state(MapLoadingState::WaitingForServer)
	, m_map_handle(NULL)
	, m_load_task(NULL)
	, m_load_task_id(-1)
	, m_bgm_pushed(false)
	, m_load_success(false)
	, m_duration_timer(0.0f)
{
	Set_Layout("map_loading");	
}

const char* UIScene_MapLoading::Get_Name()
{
	return "UIScene_MapLoading";
}

bool UIScene_MapLoading::Can_Accept_Invite()
{
	return false;
}

bool UIScene_MapLoading::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_MapLoading::Should_Render_Lower_Scenes_Background()
{
	return false;
}

void UIScene_MapLoading::Refresh(UIManager* manager)
{
	// Load all tips
	m_tip_count = 0;
	while (true)
	{
		char name[32];
		sprintf(name, "#tip_%i", m_tip_count);

		const char* tagline = Locale::Get()->Get_String(name, true);
		if (tagline != NULL)
		{
			m_tips.push_back(tagline);
			m_tip_count++;
		}
		else
		{
			break;
		}
	}

	// No tips defined? Put a dummy one in so we know.
	if (m_tips.size() <= 0)
	{
		m_tips.push_back("< !! NO TIPS DEFINED !! >");
		m_tip_count++;
	}

	// Refresh base stuff.
	UIScene::Refresh(manager);
}

void UIScene_MapLoading::Next_Tip()
{
	m_tip = m_tips.at(Random::Static_Next(0, m_tip_count));
	m_tip_timer = 0.0f;
}

void UIScene_MapLoading::Set_Map_Details(const char* name, const char* description)
{
	UILabel* label = Find_Element<UILabel*>("map_name_text");
	if (label != NULL)
	{
		label->Set_Value(name);
	}

	label = Find_Element<UILabel*>("map_description_text");
	if (label != NULL)
	{
		label->Set_Value(description);
	}
}

void UIScene_MapLoading::Set_Progress(const char* text, float progress)
{
	UIProgressBar* bar = Find_Element<UIProgressBar*>("progress_bar");
	if (bar != NULL)
	{
		bar->Set_Progress(progress);
	}

	UILabel* label = Find_Element<UILabel*>("loading_text");
	if (label != NULL)
	{
		label->Set_Value(text);
	}

	m_progress_text = text;
}

UIScene* UIScene_MapLoading::Get_Background(UIManager* manager)
{
	UIElement* char_position = Find_Element<UIElement*>("bg_character_position");
	UIElement* logo_position = Find_Element<UIElement*>("bg_logo_position");

	UIScene_LoadingBackground* bg = static_cast<UIScene_LoadingBackground*>(manager->Retrieve_Persistent_Scene("UIScene_LoadingBackground"));

	if (char_position != NULL)
		bg->Set_Character_Position(char_position->Get_Screen_Box().X, char_position->Get_Screen_Box().Y);
	else
		bg->Set_Character_Position(0, 0);

	if (logo_position != NULL)
		bg->Set_Logo_Position(logo_position->Get_Screen_Box().X, logo_position->Get_Screen_Box().Y);
	else
		bg->Set_Logo_Position(0, 0);

	if (m_set_style == false)
	{
		bg->Set_Style_Index(Random::Static_Next(0, UIScene_LoadingBackground::MAX_STYLES));
	}
	m_set_style = true;

	return bg;
}	

void UIScene_MapLoading::Enter(UIManager* manager)
{
	Next_Tip();

	Set_Progress(S("#map_load_waiting_for_server"), 0.2f);
	Set_Map_Details("", "");

	Game::Get()->Set_Update_Game(false);
	Game::Get()->Set_Loading(true);
	GameEngine::Get()->Get_VM()->Set_MultiThreaded(true);
}	

void UIScene_MapLoading::Exit(UIManager* manager)
{
	// Cancel and destroy the loading task.
	if (m_load_task != NULL)
	{
		m_load_task->Cancel();
		TaskManager::Get()->Wait_For(m_load_task_id);
		SAFE_DELETE(m_load_task);
	}

	// Pop off the BGM if we were not successful.
	if (m_bgm_pushed == true && m_load_success == false)
	{
		GameEngine::Get()->Pop_Map_BGM();
	}

	if (m_load_success)
	{
		Game::Get()->Set_Update_Game(true);
	}

	// Force sync stats.
	Game::Get()->Set_Loading(false);
	GameEngine::Get()->Get_VM()->Set_MultiThreaded(false);
	Game::Get()->Queue_Save();

	if (OnlineAchievements::Try_Get() != NULL)
	{
		OnlineAchievements::Get()->Force_Sync();
	}
}	

bool UIScene_MapLoading::Update_Loading(const FrameTime& time, UIManager* uimanager)
{
	NetManager* manager = NetManager::Get();
	GameNetClient* client = static_cast<GameNetClient*>(manager->Client());
	GameNetServer* server = static_cast<GameNetServer*>(manager->Server());

	m_duration_timer += time.Get_Frame_Time();

	bool bLoadComplete = (m_load_task == NULL || m_load_task->Is_Completed());

	// If a load is pending, we always go straight back to the start ;_;	
	if (manager->Is_Map_Load_Pending() && bLoadComplete)
	{
		DBG_LOG("Encountered pending load. Starting loading process.");

		SAFE_DELETE(m_load_task);
		bLoadComplete = false;

		manager->Clear_Map_Load_Pending();

		std::string map_guid = manager->Get_Current_Map_GUID();
		u64 map_workshop_id = manager->Get_Current_Map_Workshop_ID();

		// TODO: Implement map-downloading from steam workshop?

		// Do we have this map?
		m_map_handle = ResourceFactory::Get()->Get_Map_File_By_GUID(map_guid.c_str());
		if (m_map_handle == NULL)
		{
			// If we are a client, back to the title screen with an error.
			if (server == NULL)
			{
				GameNetManager::Get()->Leave_Network_Game();

				if (GameEngine::Get()->Get_UIManager()->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
					GameEngine::Get()->Get_UIManager()->Go(UIAction::Pop(NULL, "UIScene_GameSetupMenu"));
				else
					GameEngine::Get()->Get_UIManager()->Go(UIAction::Pop(NULL, "UIScene_PlayerLogin"));

				if (map_workshop_id != 0)
				{
					GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_OpenWorkshopDialog("#map_load_error_unknown_map_workshop", map_workshop_id), new UIFadeInTransition()));
				}
				else
				{
					GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_Dialog("#map_load_error_unknown_map"), new UIFadeInTransition()));
				}
			}

			// If we are a server, SHIT, something has gone seriously wrong
			// we should never be able to get into this state. Best bet, revert
			// to default map, send change-map message to all clients.
			else
			{
				std::string new_map_name = *EngineOptions::server_safe_map_name;
				MapFileHandle* mapHandle = ResourceFactory::Get()->Get_Map_File_By_Short_Name(new_map_name.c_str());
				DBG_ASSERT(mapHandle != NULL);

				server->Force_Map_Change(
					mapHandle->Get()->Get_Header()->GUID.c_str(), 
					mapHandle->Get()->Get_Header()->Workshop_ID, 
					Random::Static_Next(),
					0);
			}

			return false;
		}
		else
		{
			DBG_LOG("Starting load of map '%s'.", map_guid.c_str());
			m_loading_state = MapLoadingState::LoadingMap;

			// Unload the old map.
			GameEngine::Get()->Restart_Map();
			ActorReplicator::Get()->Restart();
			ParticleManager::Get()->Reset();
			CollisionManager::Get()->Reset();
			GameEngine::Get()->Get_Scene()->Unload();
			GameEngine::Get()->Get_VM()->Get_GC()->Collect();

			// Play music.
			SoundHandle* handle = m_map_handle->Get()->Get_Header()->Music;
			GameEngine::Get()->Push_Map_BGM(handle);
			m_bgm_pushed = true;

			// Load map.
			m_load_task = new MapLoadTask(Game::Get()->Get_Map(), m_map_handle);
			m_load_task_id = TaskManager::Get()->Add_Task(m_load_task);
			TaskManager::Get()->Queue_Task(m_load_task_id);

			// Update restricted mode based on map state.
			bool isRestricted = false;
			PackageFile* package = m_map_handle->Get()->Get_Package();
			if (package == NULL)
			{
				OnlineSubscribedMod* mod = m_map_handle->Get()->Get_Online_Mod();
				if (mod != NULL && !mod->IsWhitelisted)
				{
					isRestricted = true;
				}
			}
			else
			{
				if (!package->Is_Signed() && !package->Is_Whitelisted())
				{
					isRestricted = true;
				}
			}
			Game::Get()->Set_Map_Restricted_Mode(isRestricted);

			if (client != NULL)
			{
				client->Set_Game_State(GameNetClient_GameState::Loading);
			}
		}
	}

	switch (m_loading_state)
	{
	case MapLoadingState::WaitingForServer:
		{
			Set_Progress(S("#map_load_waiting_for_server"), 0.0f);
			break;
		}
	case MapLoadingState::LoadingMap:
		{
			Set_Progress(S("#map_load_loading_map"), 0.2f);
			Set_Map_Details(StringHelper::Uppercase(S(m_map_handle->Get()->Get_Header()->Long_Name.c_str())).c_str(), 
													m_map_handle->Get()->Get_Header()->Loading_Slogan.c_str());

			if (bLoadComplete && m_load_task != NULL)
			{
				bool success = m_load_task->Was_Success();
				SAFE_DELETE(m_load_task);

				if (!success)
				{					
					GameNetManager::Get()->Leave_Network_Game();

					if (GameEngine::Get()->Get_UIManager()->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
						GameEngine::Get()->Get_UIManager()->Go(UIAction::Pop(NULL, "UIScene_GameSetupMenu"));
					else
						GameEngine::Get()->Get_UIManager()->Go(UIAction::Pop(NULL, "UIScene_PlayerLogin"));


					GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_Dialog("#map_load_error_failed_load"), new UIFadeInTransition()));
					return false;
				}

				if (client != NULL)
				{
					DBG_LOG("Finished loading map, sending client info.");

					m_loading_state = MapLoadingState::SendingClientInfo;
					client->Set_Game_State(GameNetClient_GameState::SendingClientInfo);
				}
				else
				{
					DBG_LOG("Finished loading map, heading back to server uiscene.");					

					m_load_success = true;

					Game::Get()->Finalize_Map_Load();

					ScriptEventListener::Fire_On_Map_Load();

					GameNetManager::Get()->Game_Server()->Map_Loaded();

					//EditorManager::Get()->Store_Map_State();

					uimanager->Go(UIAction::Pop(NULL, "UIScene_Server"));
					return false;
				}
			}

			break;
		}
	case MapLoadingState::SendingClientInfo:
		{
			Set_Progress(S("#map_load_sending_client_info"), 0.6f);

			client->Send_Profile_Update();

			if (true) // Don't actually have to wait for anything, just send and progress to sync?
			{			
				DBG_LOG("Finished sending client info, syncing with server.");

				m_loading_state = MapLoadingState::SyncWithServer;	
				client->Set_Game_State(GameNetClient_GameState::SyncingWithServer);
			}

			break;
		}
	case MapLoadingState::SyncWithServer:
		{
			Set_Progress(S("#map_load_sync_with_server"), 0.8f);
			
			// Tick actor replicator which needs to spawn objects to complete sync.	
			ActorReplicator::Get()->Tick(time);

			if (ActorReplicator::Get()->Has_Recieved_Initial_Update() || 
				GameNetManager::Get()->Server() != NULL) 
			{			
				DBG_LOG("Finished syncing with server, waiting for start.");

				Game::Get()->Finalize_Map_Load();
				//EditorManager::Get()->Store_Map_State();

				m_loading_state = MapLoadingState::WaitingForStart;	
				client->Set_Game_State(GameNetClient_GameState::WaitingForStart);
			}

			break;
		}
	case MapLoadingState::WaitingForStart:
		{
			Set_Progress(S("#map_load_sync_with_server"), 1.0f);

			if (true) // TODO: We don't do any waiting for start atm.
			{	
				DBG_LOG("Finished loading, entering game.");
				
				m_loading_state = MapLoadingState::Starting;	
			}

			break;
		}
	case MapLoadingState::Starting:
		{
			// Artificial delay so we don't just have a flicking loading screen.
			if (m_duration_timer >= minimum_display_time)
			{
#ifndef OPT_DISABLE_CUTSCENES				
				VideoHandle* handle = NULL;
				if (m_map_handle->Get()->Get_Header()->Intro_Cutscene != "" && !GameNetManager::Get()->Get_Server_State().In_Editor_Mode && !NetManager::Get()->Is_Dedicated_Server())
				{
					handle = ResourceFactory::Get()->Get_Video(m_map_handle->Get()->Get_Header()->Intro_Cutscene.c_str());
				}
#endif

				m_load_success = true;

#ifndef OPT_DISABLE_CUTSCENES				
				client->Set_Game_State(handle != NULL ? GameNetClient_GameState::InCutscene : GameNetClient_GameState::InGame);
#else
				client->Set_Game_State(GameNetClient_GameState::InGame);		
#endif

				ScriptEventListener::Fire_On_Map_Load();

				if (GameNetManager::Get()->Server() != NULL)
				{
					GameNetManager::Get()->Game_Server()->Map_Loaded();
				}

#ifndef OPT_DISABLE_CUTSCENES				
				if (m_map_handle->Get()->Get_Header()->Intro_Cutscene != "" && !GameNetManager::Get()->Get_Server_State().In_Editor_Mode && !NetManager::Get()->Is_Dedicated_Server())
				{
					if (handle)
					{
						UIAction next_action = UIAction::Replace(new UIScene_Game(), new UIFadeOutToNewTransition());
						uimanager->Go(UIAction::Replace(new UIScene_Video(handle, next_action), new UIFadeOutToNewTransition()));
					}
					else
					{
						DBG_LOG("Failed to find outro video: %s", m_map_handle->Get()->Get_Header()->Intro_Cutscene.c_str());
						uimanager->Go(UIAction::Replace(new UIScene_Game(), new UIFadeOutToNewTransition()));
					}
				}
				else
#endif
				{
					uimanager->Go(UIAction::Replace(new UIScene_Game(), new UIFadeOutToNewTransition()));
				}

				return false;
			}
			break;
		}
	default:
		{
			break;
		}
	}

	return true;
}

void UIScene_MapLoading::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Lost connection to server?
	if (Game::Get()->Check_Network_Disconnect())
	{
		return;
	}

	// Leave game?
	if (Input::Get()->Was_Pressed_By_Any(OutputBindings::GUI_Back))
	{
		DBG_LOG("User escaped-quit the game.");

		GameNetManager::Get()->Leave_Network_Game();

		if (manager->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
			manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_GameSetupMenu"));
		else
			manager->Go(UIAction::Pop(new UISlideInTransition(), "UIScene_PlayerLogin"));

		return;
	}

	// Update tip.
	m_tip_timer += time.Get_Frame_Time();

	float delta			= m_tip_timer / float(tip_interval);
	float fade_delta	= float(tip_fade_interval) / float(tip_interval);
	float alpha			= Math::FadeInFadeOut(0.0f, 1.0f, delta, fade_delta);

	// Next tip?
	if (delta >= 1.0f)
	{
		Next_Tip();
	}

	UILabel* label = Find_Element<UILabel*>("loading_tip_text");
	if (label != NULL)
	{
		Color color = label->Get_Color();
		color.A = (int)(alpha * 255.0f);

		label->Set_Value(m_tip);
		label->Set_Color(color);
		label->Set_Use_Markup(true);
	}

	// Update the "..." effect on the progress text.
	UILabel* prog_label = Find_Element<UILabel*>("loading_text");
	if (prog_label != NULL)
	{
		int dot_count = int(time.Get_Frame_Start_Time() / pip_increment_interval) % pip_count;

		std::string txt = m_progress_text + " ";
		for (int i = 0; i <= dot_count; i++)
			txt += ".";

		prog_label->Set_Value(txt.c_str());
	}

	// Update the actual loading!
	if (!Update_Loading(time, manager))
	{
		return;
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_MapLoading::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}



