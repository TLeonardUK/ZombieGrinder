// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Runner/GameMode.h"
#include "Game/Runner/Game.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetServer.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Transitions/UIFadeOutToNewTransition.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Game/UI/Scenes/UIScene_Video.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Engine/Resources/ResourceFactory.h"


GameMode::GameMode()
	: m_active_net_id(-1)
	, m_game_mode_object(NULL)
	, m_hud_event_listener(NULL)
	, m_camera_count(0)
	, m_persistent_state_stored(false)
	, m_game_over_score(0)
	, m_old_camera_symbol(NULL)
	, m_old_camera_is_pvp(false)
	, m_old_camera_map_load_index(-1)
{
	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		m_camera_event_listeners[i] = NULL;
	}
}

GameMode::~GameMode()
{
	SAFE_DELETE(m_hud_event_listener);

	for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
	{
		SAFE_DELETE(m_camera_event_listeners[i]);
	}
}

ScriptedActor* GameMode::Get_Game_Mode()
{
	return m_game_mode_object;
}

std::vector<std::string>& GameMode::Get_Scoreboard_Columns()
{
	return m_scoreboard_columns;
}

std::vector<GameModeTeam>& GameMode::Get_Teams()
{
	return m_teams;
}

std::string	GameMode::Get_Stage_Text()
{
	return m_stage_text;
}

bool GameMode::Is_PVP()
{
	return m_is_pvp;
}

CVMObjectHandle GameMode::Get_Camera(int index)
{
	DBG_ASSERT(index >= 0 && index < m_camera_count);
	return m_camera_instances[index];
}

int	GameMode::Get_Camera_Count()
{
	return m_camera_count;
}

CVMObjectHandle GameMode::Get_HUD()
{
	return m_hud_instance;
}

GameModeTeam* GameMode::Get_Team_By_Index(int index)
{
	if (m_teams.size() == 1)
	{
		return &m_teams.at(0);
	}

	if (index < 0 || index >= (int)m_teams.size())
	{
		return NULL;
	}

	return &m_teams.at(index);
}

void GameMode::Tick(const FrameTime& time)
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	float display_width = (float)GfxDisplay::Get()->Get_Width();
	float display_height = (float)GfxDisplay::Get()->Get_Height();

	if (client != NULL)
	{
		// Has game mode updated?
		GameNetServerState state = client->Get_Server_State();
		if (state.Game_Mode_Net_ID != m_active_net_id && server == NULL)
		{
			Set_Active_Net_ID(state.Game_Mode_Net_ID);
		}

		if (!NetManager::Get()->Is_Dedicated_Server())
		{

			// Setup splitscreen viewports.
			for (int i = 0; i < m_camera_count; i++)
			{
				Rect2D viewport;

				float padding = 4.0f;

#ifdef OPT_FORCE_CAMERA_COUNT
				switch (OPT_FORCE_CAMERA_COUNT)
#else
				switch (m_camera_count)
#endif
				{
					// 1 camera - full screen.
				case 1:
					{
						viewport = Rect2D(0.0f, 0.0f, display_width, display_height);
						break;
					}
					// 2 cameras - horizontal split.
				case 2:
					{
						if (i == 0)
						{
							viewport = Rect2D(0.0f, 0.0f, display_width, (display_height * 0.5f) - padding);
						}
						else
						{
							viewport = Rect2D(0.0f, (display_height * 0.5f) + padding, display_width, (display_height * 0.5f) - padding);
						}
						break;
					}
					// 3 cameras - horizontal split and vertical split of top segment.
				case 3:
					{
						if (i == 0)
						{
							viewport = Rect2D(0.0f, 0.0f, (display_width * 0.5f) - padding, (display_height * 0.5f) - padding);
						}
						else if (i == 1)
						{
							viewport = Rect2D((display_width * 0.5f) + padding, 0.0f, (display_width * 0.5f) - (padding*2), (display_height * 0.5f) - padding);
						}
						else if (i == 2)
						{
							viewport = Rect2D(0.0f, (display_height * 0.5f) + padding, display_width, (display_height * 0.5f) - padding);
						}
						break;
					}
					// 4 cameras - horizontal split and vertical split.
				case 4:
					{
						if (i == 0)
						{
							viewport = Rect2D(0.0f, 0.0f, (display_width * 0.5f) - padding, (display_height * 0.5f) - padding);
						}
						else if (i == 1)
						{
							viewport = Rect2D((display_width * 0.5f) + padding, 0.0f, (display_width * 0.5f) - (padding*2), (display_height * 0.5f) - padding);
						}
						else if (i == 2)
						{
							viewport = Rect2D(0.0f, (display_height * 0.5f) + padding, (display_width * 0.5f) - padding, (display_height * 0.5f) - padding );
						}
						else if (i == 3)
						{
							viewport = Rect2D((display_width * 0.5f) + padding, (display_height * 0.5f) + padding, (display_width * 0.5f) - (padding*2), (display_height * 0.5f) - padding);
						}
						break;
					}
				default:
					{
						DBG_ASSERT_STR(false, "To many split screen cameras! DAFUQ!");
					}
				}

				Camera* camera = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
				camera->Set_Screen_Viewport(viewport);
			}

			for (int i = 0; i < m_camera_count; i++)
			{
				// Get camera viewport.
				if (m_camera_instances[i].Get().Get() != NULL &&
					m_symbol_camera_get_viewport != NULL)
				{
					CVirtualMachine* vm = GameEngine::Get()->Get_VM();
					CVMContextLock lock = vm->Set_Context(m_game_mode_object->Get_Script_Context());
					CVMValue instance_value = m_camera_instances[i];

					// Tick camera first.
					m_camera_event_listeners[i]->On_Tick();

					// Then get viewport.
					vm->Invoke(m_symbol_camera_get_viewport, instance_value);

					CVMValue retval;
					vm->Get_Return_Value(retval);

					Rect2D viewport = Rect2D
					(
						retval.object_value.Get()->Get_Slot(0).float_value,
						retval.object_value.Get()->Get_Slot(1).float_value,
						retval.object_value.Get()->Get_Slot(2).float_value,
						retval.object_value.Get()->Get_Slot(3).float_value
					);

					Camera* camera = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
					camera->Set_Position(Vector3(viewport.X, viewport.Y, 0.0f));
					camera->Set_Viewport(Rect2D(0.0f, 0.0f, viewport.Width, viewport.Height));
					camera->Set_Enabled(true);
				}
			}

			// Disable other game cameras.
			for (int i = m_camera_count; i < MAX_LOCAL_PLAYERS; i++)
			{
				Camera* camera = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
				camera->Set_Enabled(false);
			}
		}
	}
	
	if (server != NULL)
	{
		Sync(false);
	}
}

int GameMode::Get_Active_Net_ID()
{
	return m_active_net_id;		
}

void GameMode::Set_Active_Net_ID(int id)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	GameScene* scene = Game::Get()->Get_Game_Scene();

	DBG_LOG("Active game mode changed to object '%i'.", id);

	// Grab the game object.
	m_game_mode_object = scene->Find_Actor_By_Unique_ID(id);
	DBG_ASSERT(m_game_mode_object != NULL);
	m_active_net_id = id;
	
	CVMLinkedSymbol* class_sym = m_game_mode_object->Get_Script_Object().Get()->Get_Symbol();

	m_team_class_symbol				= vm->Find_Class("Team");
	m_team_name_symbol				= vm->Find_Variable(m_team_class_symbol, "Name");
	m_team_primary_color_symbol		= vm->Find_Variable(m_team_class_symbol, "Primary_Color");

	// Grab symbols.
	m_symbol_is_pvp					= vm->Find_Event(NULL, class_sym, "Is_PVP", 0);
	m_symbol_get_teams				= vm->Find_Event(NULL, class_sym, "Get_Teams", 0);
	m_symbol_get_scoreboard_columns = vm->Find_Event(NULL, class_sym, "Get_Scoreboard_Columns", 0);
	m_symbol_get_scoreboard_title	= vm->Find_Event(NULL, class_sym, "Get_Scoreboard_Title", 0);
	m_symbol_create_camera			= vm->Find_Event(NULL, class_sym, "Create_Camera", 1, "int");
	m_symbol_create_hud				= vm->Find_Event(NULL, class_sym, "Create_HUD", 0);
	m_symbol_persist_state			= vm->Find_Event(NULL, class_sym, "Persist_State", 0);
	m_symbol_restore_state			= vm->Find_Event(NULL, class_sym, "Restore_State", 1, "object");

	Sync(true);
}

void GameMode::Sync(bool full)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(m_game_mode_object->Get_Script_Context());

	CVMValue retval;
	CVMValue instance_value = m_game_mode_object->Get_Script_Object();;

	if (full)
	{
		m_scoreboard_columns.clear();
		m_teams.clear();
		m_stage_text = "";

		// Grab properties.
		vm->Invoke(m_symbol_is_pvp, instance_value);
		vm->Get_Return_Value(retval);
		m_is_pvp = (retval.int_value != 0);

		// Grab teams.
		vm->Invoke(m_symbol_get_teams, instance_value);
		vm->Get_Return_Value(retval);
		CVMObject* value = retval.object_value.Get();
		for (int i = 0; i < value->Slot_Count(); i++)
		{
			GameModeTeam team;
			team.Script_Handle = value->Get_Slot(i).object_value;
			
			vm->Get_Field(m_team_primary_color_symbol, team.Script_Handle, retval);
			CVMObject* color_value = retval.object_value.Get();
			
			team.Primary_Color	= Color(color_value->Get_Slot(0).float_value * 255.0f,
								color_value->Get_Slot(1).float_value * 255.0f,
								color_value->Get_Slot(2).float_value * 255.0f,
								color_value->Get_Slot(3).float_value * 255.0f);

			vm->Get_Field(m_team_name_symbol, team.Script_Handle, retval);
			team.Name	= retval.string_value.C_Str();

			m_teams.push_back(team);
		}
	
		// Grab scoreboard columns.
		vm->Invoke(m_symbol_get_scoreboard_columns, instance_value);
		vm->Get_Return_Value(retval);
		value = retval.object_value.Get();
		for (int i = 0; i < value->Slot_Count(); i++)
		{
			m_scoreboard_columns.push_back(value->Get_Slot(i).string_value.C_Str());			
		}

		// Create new camera instance.
		if (!NetManager::Get()->Is_Dedicated_Server())
		{
			// If PVP, we are going to split screen this shit, so create a camera for each
			// individual local player.
			int local_user_count = NetManager::Get()->Get_Local_Net_Users().size();
			int old_camera_count = m_camera_count;

			if (m_is_pvp)
			{
				m_camera_count = local_user_count;
			}
			else
			{
				m_camera_count = 1;
			}

			// Shitty hack to prevent recreation of cameras when changing game mode.
			bool bDisableCameraCreation = false;
			if (m_camera_count == old_camera_count && m_is_pvp == m_old_camera_is_pvp && NetManager::Get()->Get_Current_Map_Load_Index() == m_old_camera_map_load_index)// && m_symbol_create_camera == m_old_camera_symbol)
			{
				bDisableCameraCreation = true;
			}
			m_old_camera_symbol = m_symbol_create_camera;
			m_old_camera_is_pvp = m_is_pvp;
			m_old_camera_map_load_index = NetManager::Get()->Get_Current_Map_Load_Index();

			if (!bDisableCameraCreation)
			{
				DBG_LOG("Creating %i cameras for %i local players", m_camera_count, local_user_count);

				for (int i = 0; i < MAX_LOCAL_PLAYERS; i++)
				{
					m_camera_instances[i] = NULL;
					SAFE_DELETE(m_camera_event_listeners[i]);
				}

				for (int i = 0; i < m_camera_count; i++)
				{
					vm->Push_Parameter(i);
					vm->Invoke(m_symbol_create_camera, instance_value);
					vm->Get_Return_Value(retval);
					m_camera_instances[i] = retval.object_value;
					vm->Set_Default_State(m_camera_instances[i]);
				}

				CVMLinkedSymbol* camera_symbol = m_camera_instances[0].Get().Get()->Get_Symbol();	
				m_symbol_camera_get_viewport = vm->Find_Function(camera_symbol, "Get_Viewport", 0);

				for (int i = 0; i < m_camera_count; i++)
				{
					m_camera_event_listeners[i] = new ScriptEventListener(vm->Get_Static_Context(), m_camera_instances[i]);
					m_camera_event_listeners[i]->Set_Manual_Tick(true);
				}
			}
			else
			{
				DBG_LOG("Recycling old cameras - game mode seems to be the same.");
			}
		}

		// Create new hud instance.
		//if (!NetManager::Get()->Is_Dedicated_Server())
		{
			vm->Invoke(m_symbol_create_hud, instance_value);
			vm->Get_Return_Value(retval);
			m_hud_instance = retval.object_value;
			vm->Set_Default_State(m_hud_instance);

			CVMLinkedSymbol* hud_symbol = m_hud_instance.Get().Get()->Get_Symbol();	

			SAFE_DELETE(m_hud_event_listener);
			m_hud_event_listener = new ScriptEventListener(vm->Get_Static_Context(), m_hud_instance);
		}
	}

	// Grab scoreboard title.
	vm->Invoke(m_symbol_get_scoreboard_title, instance_value);
	vm->Get_Return_Value(retval);
	m_stage_text = retval.string_value.C_Str();
}

void GameMode::Start_Map()
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	// If server, grab first enabled game mode actor.
	if (server != NULL)
	{
		CVMLinkedSymbol* sym = GameEngine::Get()->Get_VM()->Find_Class("Game_Mode");
		DBG_ASSERT_STR(sym != NULL, "Unable to find 'Game_Mode' symbol!");

		std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(sym);
		if (actors.size() == 0)
		{
			DBG_LOG("No game mode objects found in level. Using Wave_Game_Mode as default.");

			CVMLinkedSymbol* wave_sym = GameEngine::Get()->Get_VM()->Find_Class("Wave_Game_Mode");
			DBG_ASSERT_STR(wave_sym != NULL, "Failed to spawn default game mode.");
			Game::Get()->Get_Game_Scene()->Spawn(wave_sym, NULL);

			actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(sym);
			DBG_ASSERT_STR(actors.size() > 0, "Failed to spawn default game mode.");
		}
		
		ScriptedActor* active_actor = NULL;
		for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
		{
			ScriptedActor* actor = *iter;
			if (actor->Get_Enabled())
			{
				if (active_actor == NULL)
				{
					active_actor = actor;
				}
				else if (active_actor != NULL)
				{
					DBG_LOG("[WARNING] Multiple enabled game mode objects. Unable first.");
					actor->Set_Enabled(false);
				}
			}
		}

		if (active_actor == NULL)
		{
			DBG_LOG("No enabled game mode object. Enabling first in list.");

			active_actor = actors.at(0);
			active_actor->Set_Enabled(true);
		}

		Set_Active_Net_ID(active_actor->Get_Replication_Info().unique_id);
	}

	// If client get game mode actor from state.
	else
	{
		GameNetServerState state = client->Get_Server_State();
		Set_Active_Net_ID(state.Game_Mode_Net_ID);
	}

	// Reload persisted state if required.
	Game::Get()->Get_Game_Mode()->Restore_State();
}

void GameMode::Persist_State()
{
	if (m_game_mode_object == NULL)
	{
		m_persistent_state_stored = false;
		return;
	}

	if (!NetManager::Get()->Is_Server())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(m_game_mode_object->Get_Script_Context());

	CVMValue instance_value = m_game_mode_object->Get_Script_Object();
	CVMValue retval;

	// Grab scoreboard title.
	vm->Invoke(m_symbol_persist_state, instance_value);
	vm->Get_Return_Value(retval);

	m_persistent_state_object = retval.object_value;
	m_persistent_state_stored = (m_persistent_state_object.Get().Get() != NULL);

	if (m_persistent_state_stored)
	{
		DBG_LOG("Stored game mode persistent state.");
	}
}

void GameMode::Restore_State()
{
	if (m_game_mode_object == NULL || m_persistent_state_stored == false)
	{
		return;
	}

	if (!NetManager::Get()->Is_Server())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(m_game_mode_object->Get_Script_Context());

	CVMValue instance_value = m_game_mode_object->Get_Script_Object();
	CVMValue param_1 = m_persistent_state_object;

	// Grab scoreboard title.
	vm->Push_Parameter(param_1);
	vm->Invoke(m_symbol_restore_state, instance_value);

	DBG_LOG("Reloaded game mode persistent state.");
	m_persistent_state_stored = false;
}

void GameMode::Reset_Game_Over()
{
	m_game_over_bonuses.clear();
}

void GameMode::Add_Game_Over_Bonus(std::string name, int value)
{
	GameOverBonus bonus;
	bonus.name = name;
	bonus.value = value;
	m_game_over_bonuses.push_back(bonus);
}

void GameMode::Show_Game_Over(std::string title, std::string destination)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	if (NetManager::Get()->Is_Dedicated_Server())
	{
		manager->Go(UIAction::Pop(NULL, "UIScene_Server"));
	}
	else
	{
		if (manager->Get_Scene_By_Name("UIScene_Game"))
		{
			manager->Go(UIAction::Pop(NULL, "UIScene_Game"));
		}
		else
		{
			manager->Go(UIAction::Push(new UIScene_Game()));
		}
	}

#ifndef OPT_DISABLE_CUTSCENES				
	
	if (destination != "" && GameEngine::Get()->Get_Map()->Get_Map_Header().Outro_Cutscene != "" && !NetManager::Get()->Is_Dedicated_Server())
	{
		VideoHandle* handle = ResourceFactory::Get()->Get_Video(GameEngine::Get()->Get_Map()->Get_Map_Header().Outro_Cutscene.c_str());
		if (handle)
		{
			UIAction next_action = UIAction::Replace(new UIScene_GameOver(m_game_over_bonuses, title, destination, true), new UIFadeOutToNewTransition());
			manager->Go(UIAction::Push(new UIScene_Video(handle, next_action), new UIFadeInTransition()));
		}
		else
		{
			DBG_LOG("Failed to find outro video: %s", GameEngine::Get()->Get_Map()->Get_Map_Header().Outro_Cutscene.c_str());
			manager->Go(UIAction::Push(new UIScene_GameOver(m_game_over_bonuses, title, destination), new UIFadeInTransition()));
		}
	}
	else
#endif
	{
		manager->Go(UIAction::Push(new UIScene_GameOver(m_game_over_bonuses, title, destination), new UIFadeInTransition()));
	}
}

int GameMode::Get_Game_Over_Score()
{
	return m_game_over_score;
}

void GameMode::Set_Game_Over_Score(int val)
{
	m_game_over_score = val;
}
