// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Runner/Game.h"

#include "Game/Admin/AdminServer.h"
#include "Game/Monitor/MonitorServer.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Display/GfxDisplay.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Light.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Engine/GameEngine.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Engine/Storage/PersistentStorage.h"

#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Textures/PNG/PNGPixelmapFactory.h"

#include "Engine/Renderer/Atlases/AtlasFactory.h"

#include "Engine/Input/Input.h"
#include "Engine/Input/Steam/Steam_JoystickState.h"

#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundChannel.h"
#include "Engine/Audio/Sounds/SoundFactory.h"

#include "Game/UI/Scenes/UIScene_ConnectToServer.h"
#include "Game/UI/Scenes/UIScene_StartingLocalServer.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"
#include "Engine/UI/Transitions/UIFadeTopLevelTransition.h"

#include "Engine/Scene/Map/Map.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Engine/Demo/DemoManager.h"

#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Game/Scene/Cameras/EditorCamera.h"
#include "Game/Scene/Cameras/GameCamera.h"
#include "Game/Scene/Cameras/OrthoCamera.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"
#include "Game/Profile/SkillManager.h"
#include "Game/Online/DLCManager.h"

#include "Game/UI/Scenes/UIScene_FindGame.h"
#include "Game/UI/Scenes/UIScene_PlayerLogin.h"
#include "Engine/UI/Transitions/UIFadeInTransition.h"

#include "Engine/Online/OnlineMatching.h"
#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineClient.h"
#include "Engine/Online/OnlineInventory.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Scene/Map/Map.h"
#include "Game/Scene/Map/Legacy/LegacyMapConverter.h"

#include "Game/UI/Scenes/UIScene_Game.h"
#include "Game/UI/Scenes/UIScene_Base.h"
#include "Game/UI/Scenes/UIScene_Dialog.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"

#include "Game/Scene/Actors/CompositeActor.h"
#include "Game/Scene/Actors/Components/Drawable/SpriteComponent.h"
#include "Game/Scene/Actors/Components/Collision/CollisionComponent.h"
#include "Game/Scene/Actors/Components/Tickable/BoidComponent.h"
#include "Game/Scene/Actors/Components/Tickable/EffectComponent.h"
#include "Game/Scene/Actors/Components/Tickable/ParticleCollectorComponent.h"
#include "Game/Scene/Actors/Components/Tickable/PathPlannerComponent.h"

#include "Engine/Scene/Collision/CollisionManager.h"
#include "Engine/Scene/Pathing/PathManager.h"

#include "Game/Scene/Boids/BoidManager.h"

#include "Engine/Particles/ParticleManager.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Game/Scene/GameScene.h"

#include "Game/Runner/SaveData/SaveData.h"
#include "Game/Runner/GameTagReplacer.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/ActorReplicator.h"
#include "Game/Runner/GameOptions.h"
#include "Game/Version.h"

#include "Generic/Helper/CommandLineParser.h"
#include "Generic/Math/Math.h"

#include "Engine/Options/OptionRegistry.h"
#include "Engine/Engine/EngineOptions.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/GameMode.h"
#include "Game/Runner/GameModeTeam.h"

#include "Game/Scripts/GameVirtualMachine.h"
#include "Game/Scripts/ScriptEventListener.h"

#include "Game/Game/ChatManager.h"
#include "Game/Game/VoteManager.h"
#include "Game/Game/EditorManager.h"
#include "Game/Game/TutorialManager.h"
#include "Game/Online/AchievementManager.h"
#include "Game/Online/ChallengeManager.h"
#include "Game/Online/RankingsManager.h"
#include "Game/Online/StatisticsManager.h"

#include "Game/Moderation/ReportManager.h"
#include "Engine/Demo/DemoManager.h"

#include "Game/Scene/PostProcess/GamePostProcessManager.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Engine/IO/StreamFactory.h"

#include "XScript/VirtualMachine/CVMBinary.h"

DEFINE_FRAME_STATISTIC("Game Tick/Statistics Time", float, g_game_statistics_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Rankings Time", float, g_game_rankings_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Achievemnts Time", float, g_game_achievements_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/DLC Time", float, g_game_dlc_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Profile Time", float, g_game_profile_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Global Script Time", float, g_game_global_script_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Scene Despawn Time", float, g_game_scene_despawn_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Collect Deferred Time", float, g_game_collect_deferred_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Non-Scene Script Time", float, g_game_non_scene_script_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Scene Script Time", float, g_game_scene_script_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Latent Script Time", float, g_game_script_latent_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Map Time", float, g_game_map_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Particle Time", float, g_game_particle_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Create Deferred Time", float, g_game_create_deferred_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Collision Time", float, g_game_collision_time, false);
DEFINE_FRAME_STATISTIC("Game Tick/Path Update Time", float, g_game_path_update_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Game Managers Time", float, g_game_managers_time , true);
DEFINE_FRAME_STATISTIC("Game Tick/Game Mode Time", float, g_game_mode_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Boid Manager Time", float, g_boid_manager_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Chat Manager Time", float, g_chat_manager_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Vote Manager Time", float, g_vote_manager_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Editor Manager Time", float, g_editor_manager_time, true);
DEFINE_FRAME_STATISTIC("Game Tick/Post Process Manager Time", float, g_post_process_manager_time, true);

DEFINE_FRAME_STATISTIC("Replication/Actor Replicator Time", float, g_actor_manager_time, true);

// Maximum CPU memory usage for the game.
int max_cpu_memory = 300 * 1024 * 1024;

void Print_Game_Version()
{
#ifdef DEBUG_BUILD
	DBG_LOG(" ZombieGrinder Debug    Version %s (Last Change @ %s/%s/%s %s:%s)", VersionInfo::FULLVERSION_STRING, VersionInfo::DAY, VersionInfo::MONTH, VersionInfo::YEAR, VersionInfo::HOUR, VersionInfo::MINUTE);
#else
	DBG_LOG(" ZombieGrinder Release  Version %s (Last Change @ %s/%s/%s %s:%s)", VersionInfo::FULLVERSION_STRING, VersionInfo::DAY, VersionInfo::MONTH, VersionInfo::YEAR, VersionInfo::HOUR, VersionInfo::MINUTE);
#endif
}

void Game_Entry_Point(const char* command_line)
{
	Game* runner = new Game();

	DBG_LOG("Creating new engine instance.");
	GameEngine* engine = new GameEngine(runner, command_line);

	DBG_LOG("Running game engine.");
	engine->Run();

	// Not the correct way to deal with this at all! But at this point
	// any persistent information we need to save should have been, so
	// this should be reasonably safe.
	//_exit(0);

	DBG_LOG("Destroying game engine.");
	SAFE_DELETE(engine);
	SAFE_DELETE(runner);
}

Game::Game()
	: m_config_location("Engine.xml")
	, m_user_config_location("USER.XML")
	, m_net_manager(NULL)
	, m_game_mode(NULL)
	, m_chat_manager(NULL)
	, m_achievement_manager(NULL)
	, m_tutorial_manager(NULL)
	, m_rankings_manager(NULL)
	, m_statistics_manager(NULL)
	, m_scene(NULL)
	, m_game_update_enabled(false)
	, m_sub_menu_open(false)
	, m_update_actors(true)
	, m_last_accepted_invite_index(0)
	, m_crashed(false)
	, m_loading(false)
	, m_has_uploaded_to_workshop(false)
	, m_admin_server(NULL)
	,m_monitor_server (NULL)
#ifdef VM_ALLOW_MULTITHREADING
	, m_script_group_task("Script Tick")
#endif
	, m_save_queued(false)
	, m_game_input_enabled(true)
	, m_invite_login_shown(false)
	, m_save_icon_timer(0.0f)
	, m_save_icon_alpha(0.0f)
	, m_save_icon_sin_bob_timer(0.0f)
	, m_map_restricted_mode(false)
	, m_players_idle_timer(0.0f)
	, m_active_demo(NULL)
	, m_starting_demo(false)
{
	memset(m_cameras, 0, sizeof(Camera*) * CameraID::COUNT);

	m_next_server_stats_print_time = Platform::Get()->Get_Ticks();
}

void Game::Set_Game_Input_Enabled(bool enabled)
{
	m_game_input_enabled = enabled;
}

bool Game::Get_Game_Input_Enabled()
{
	return m_game_input_enabled;
}

void Game::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	ChatManager::Get()->Serialize_Demo(stream, version, bSaving, frameDelta);
}

void Game::Load_User_Config()
{
	// Don't load user config if dedicated server.
	if (*EngineOptions::server)
	{
		DBG_LOG("Ignoring request to load user config, running as dedicated.");
		return;
	}

	// Don't load persistent storage if we have asked to clean it.
	if (*EngineOptions::reset_save_data)
	{
		DBG_LOG("Ignoring request to load user config, engine option asked to clear it.");
		return;
	}

	DBG_LOG("Loading user configuration from persistent storage: %s", m_user_config_location);

	if (OnlinePlatform::Get()->Is_Online())
	{
		m_user_config_location = "USER.XML";
	}
	else
	{
		m_user_config_location = "OFFLINE_USER.XML";
	}

	PersistentStorage* storage = PersistentStorage::Get();
	storage->Wait_For_Pending_Writes();

	if ((*EngineOptions::local_save_data_file) != "")
	{
		Stream* stream = StreamFactory::Open((*EngineOptions::local_save_data_file).c_str(), StreamMode::Read);
		if (stream)
		{

			DataBuffer buffer;
			buffer.Reserve(stream->Length());
			stream->ReadBuffer(buffer.Buffer(), 0, buffer.Size());

			SaveData::Deserialize(buffer);

			SAFE_DELETE(stream);
		}
	}
	else if (storage->File_Exists(m_user_config_location))
	{
		int size = storage->File_Size(m_user_config_location);		
		if (size >= 0)
		{
			char* b = new char[size];

			while (true)
			{
				if (storage->Read_File(m_user_config_location, b, size) == size)
				{
					DataBuffer buffer(b, size);
					bool result = SaveData::Deserialize(buffer);
					if (!result)
					{
						DBG_LOG("Failed to load user configuration.");
					}
					else
					{
						DBG_LOG("Loaded user configuration from persistent storage.");

						m_crashed = (*GameOptions::graceful_shutdown == false);
						*GameOptions::graceful_shutdown = false;

						if (m_crashed == true)
						{
							DBG_LOG("=========================================================");
							DBG_LOG("   Game appears to have been ungracefully shutdown. :(   ");
							DBG_LOG("=========================================================");
						}
					}
					break;
				}
				else
				{
					DBG_LOG("Failed to read file, probably syncing, waiting ...");
					Platform::Get()->Sleep(100.0f);
				}
			}
		}
	}
	else
	{
		DBG_LOG("User configuration does not exist in persistent storage.");
	}
}

void Game::Queue_Save()
{
	m_save_queued = true;
}

bool Game::Has_Crashed()
{
	return m_crashed;
}

bool Game::Has_Uploaded_To_Workshop()
{
	return m_has_uploaded_to_workshop;
}

void Game::Flag_Uploaded_To_Workshop()
{
	m_has_uploaded_to_workshop = true;
}

void Game::Save_User_Config()
{
	// Don't load user config if dedicated server.
	if (*EngineOptions::server)
	{
		DBG_LOG("Ignoring request to save user config, running as dedicated.");
		return;
	}

	// If we are using local save data we don't want to persist it.
	/*if ((*EngineOptions::local_save_data_file) != "")
	{
		DBG_LOG("Ignoring request to save user config, using local save file.");
		return;
	}*/

	// Restricted mode prevents saves.
	if (In_Restricted_Mode())
	{
		DBG_LOG("Ignoring request to save user config, in restricted mode.");
		return;
	}

	DBG_LOG("Saving user configuration from persistent storage: %s", m_user_config_location);

	PersistentStorage* storage = PersistentStorage::Get();

	DataBuffer buffer;

	double time = Platform::Get()->Get_Ticks();
	bool result = SaveData::Serialize(buffer);
	DBG_LOG("Save serialize took %.2f ms", Platform::Get()->Get_Ticks() - time);
	time = Platform::Get()->Get_Ticks();

	if (result == true)
	{
		if ((*EngineOptions::local_save_data_file) != "")
		{
			Stream* stream = StreamFactory::Open((*EngineOptions::local_save_data_file).c_str(), StreamMode::Write);
			if (stream)
			{
				stream->WriteBuffer(buffer.Buffer(), 0, buffer.Size());
				SAFE_DELETE(stream);
			}
		}
		else
		{
			if (storage->Write_File(m_user_config_location, (void*)buffer.Buffer(), buffer.Size()))
			{		
				DBG_LOG("Saved user configuration.");
			}
			else
			{
				DBG_LOG("Failed to write user configuration file from persistent storage.");
			}
		}
	}
	else
	{
		DBG_LOG("Failed to serialize user configuration file.");
	}

	DBG_LOG("Save write took %.2f ms", Platform::Get()->Get_Ticks() - time);

	// Hack: Ignore initial save, no reason to display this, happens right on intro as well making it pretty dam ugly.
	static bool bInitialSave = true;
	if (!bInitialSave)
	{
		m_save_icon_timer = SAVE_ICON_MIN_DISPLAY_TIME;
	}
	bInitialSave = false;
}

NetManager*	Game::Get_Net_Manager()
{
	if (m_net_manager == NULL)
	{
		DBG_LOG("Creating game network manager ...");
		m_net_manager = new GameNetManager();
		DBG_ASSERT_STR(m_net_manager->Init(), "Failed to setup game network manager.");
	}
	return m_net_manager;
}

Scene* Game::Get_Scene()
{
	if (m_scene == NULL)
	{
		DBG_LOG("Creating game scene ...");
		m_scene = new GameScene();
	}
	return m_scene;
}

GameScene* Game::Get_Game_Scene()
{
	return m_scene;
}

Map* Game::Get_Map()
{
	return m_map;
}

ChatManager* Game::Get_Chat_Manager()
{
	return m_chat_manager;
}

EditorManager* Game::Get_Editor_Manager()
{
	return m_editor_manager;
}

CollisionManager* Game::Get_Collision_Manager()
{
	return m_collision_manager;
}

PathManager* Game::Get_Path_Manager()
{
	return m_path_manager;
}

ParticleManager* Game::Get_Particle_Manager()
{
	return m_particle_manager;
}

EngineVirtualMachine* Game::Create_VM()
{
	return new GameVirtualMachine();
}

void Game::Particle_Script_Event(ParticleInstance* instance, std::string event_name)
{
	if (instance != NULL && instance->instigator != NULL && instance->instigator_incarnation == instance->instigator->Get_Incarnation())
	{
		ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(instance->instigator);
		if (actor != NULL)
		{
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

			// Order assumed, make sure it matches class in events.xs
			CVMObjectHandle param = vm->New_Object(m_event_particle_class);
			param.Get()->Resize(1);
			param.Get()->Get_Slot(0).object_value = vm->Create_Vec3(instance->spawn_position + instance->position - Vector3(0.0f, -instance->height, 0.0f));

			actor->Get_Event_Listener()->Fire_Custom(StringHelper::Hash(event_name.c_str()), param); // TODO: Hash too.
		}
	}
}

void Game::Particle_Global_Script_Event(ParticleInstance* instance, std::string event_name)
{
	if (instance != NULL)
	{
		CVirtualMachine* vm = GameEngine::Get()->Get_VM();
		CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

		// Order assumed, make sure it matches class in events.xs
		CVMObjectHandle param = vm->New_Object(m_event_particle_class);
		param.Get()->Resize(1);
		param.Get()->Get_Slot(0).object_value = vm->Create_Vec3(instance->spawn_position + instance->position - Vector3(0.0f, -instance->height, 0.0f));

		ScriptEventListener::Fire_Global_Custom(StringHelper::Hash(event_name.c_str()), param); // TODO: Hash too.
	}
}

void Game::Preload()
{
	// Register game options.
	GameOptions::Register_Options();

	// Create managers.
	ReportManager::Create();
	DemoManager::Create();

	m_tag_replacer = new GameTagReplacer();
	m_chat_manager = new ChatManager();
	m_vote_manager = new VoteManager();
	m_actor_replicator = new ActorReplicator();
	m_editor_manager = new EditorManager();
	m_profile_manager = new ProfileManager();
	m_achievement_manager = new AchievementManager();
	m_challenge_manager = new ChallengeManager();
	m_tutorial_manager = new TutorialManager();
	m_rankings_manager = new RankingsManager();
	m_statistics_manager = new StatisticsManager();
	m_item_manager = new ItemManager();
	m_skill_manager = new SkillManager();
	m_dlc_manager = new DLCManager();
	m_collision_manager = new CollisionManager();
	m_path_manager = new PathManager();
	m_boid_manager = new BoidManager();
	m_post_process_manager = new GamePostProcessManager();

	// Register all components, kinda gross reflection code.
	Component::Register_Component_Type<CollisionComponent>();
	Component::Register_Component_Type<SpriteComponent>();
	Component::Register_Component_Type<BoidComponent>();
	Component::Register_Component_Type<EffectComponent>();
	Component::Register_Component_Type<ParticleCollectorComponent>();
	Component::Register_Component_Type<PathPlannerComponent>();
}

void Game::Setup_Cameras()
{
	GfxDisplay* display = GfxDisplay::Get();
	GameEngine* engine = GameEngine::Get();

	// Setup cameras.
	m_cameras[CameraID::Game1]			= new GameCamera  (70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Game2]			= new GameCamera  (70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Game3]			= new GameCamera  (70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Game4]			= new GameCamera  (70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::UI]				= new OrthoCamera (70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Editor_Main]	= new EditorCamera(70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Editor_Tileset]	= new EditorCamera(70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Editor_Sub]		= new EditorCamera(70, Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));

	// Add cameras to scene.
	for (int i = 0; i < CameraID::COUNT; i++)
	{
		GameEngine::Get()->Get_Scene()->Add_Camera(m_cameras[i]);
		GameEngine::Get()->Get_Scene()->Add_Tickable(m_cameras[i]);
	}

	// Set HUD camera.
	engine->Get_UIManager()->Set_Camera(m_cameras[CameraID::UI]);

	// Create the map.
	m_map = new Map();
	GameEngine::Get()->Get_Scene()->Add_Tickable(m_map);	

	// Create the game mode.
	m_game_mode = new GameMode();

	// Setup particles.	
	m_particle_manager = new ParticleManager();

	// Script tasks.
#ifdef VM_ALLOW_MULTITHREADING
	m_script_task_count = Min(max_script_tasks, TaskManager::Get()->Get_Worker_Count());
	for (int i = 0; i < m_script_task_count; i++)
	{
		m_script_tasks[i].Task_Index = i;
	}
#endif
}

void Game::Setup_Editor()
{

}

void Game::Apply_Config()
{
	DBG_LOG("Binding input controls ...");
	Input::Get()->Unbind_All();

	const int CURRENT_BINDING_UPGRADE_VERSION = 4;

	if ((*GameOptions::binding_upgrade_version) < CURRENT_BINDING_UPGRADE_VERSION)
	{
		// Remove all joystick bindings.

		for (std::vector<InputBindingConfig>::iterator iter = (*GameOptions::input_bindings).begin(); iter != (*GameOptions::input_bindings).end(); )
		{
			InputBindingConfig& config = *iter;
			if (config.input >= InputBindings::JOYSTICK_START || config.output == OutputBindings::DropCoins || config.output == OutputBindings::DropWeapon)
			{
				iter = (*GameOptions::input_bindings).erase(iter);
			}
			else
			{
				iter++;
			}
		}

		// Add all joystick bindings from defaults.
		for (std::vector<InputBindingConfig>::iterator iter = GameOptions::g_default_input_bindings.begin(); iter != GameOptions::g_default_input_bindings.end(); iter++)
		{
			InputBindingConfig& config = *iter;
			if (config.input >= InputBindings::JOYSTICK_START)
			{
				(*GameOptions::input_bindings).push_back(config);
			}
			else if (config.output == OutputBindings::DropCoins || config.output == OutputBindings::DropWeapon)
			{
				(*GameOptions::input_bindings).push_back(config);
			}
		}

		*GameOptions::binding_upgrade_version = CURRENT_BINDING_UPGRADE_VERSION;
	}

	for (int i = 0; i < Input::Get()->Get_Source_Count(); i++)
	{
		Input* input = Input::Get(i);

		for (std::vector<InputBindingConfig>::iterator iter = (*GameOptions::input_bindings).begin(); iter != (*GameOptions::input_bindings).end(); iter++)
		{
			InputBindingConfig& config = *iter;

			const char* input_cstr = NULL;
			const char* output_cstr = NULL;

			if (!InputBindings::To_String(config.input, input_cstr))
			{		
				if (i == 0)
				{
					DBG_LOG("\tInvalid input binding."); //fuuuuu, need a way of converting to string :S
				}
			}
			else if (!OutputBindings::ToString(config.output, output_cstr))
			{		
				if (i == 0)
				{
					DBG_LOG("\tInvalid output binding for '%s'.", input_cstr);
				}
			}
			else
			{			
				if (i == 0)
				{
					DBG_LOG("\tBinding %s to %s.", input_cstr, output_cstr);
				}
				input->Bind(config.input, config.output);
			}
		}
	}

	// Base application of config.
	GameEngine::Get()->Apply_Config();

	// Resize camera viewports based on new display.
	GfxDisplay* display = GfxDisplay::Get();
	if (m_cameras[CameraID::Game1] != NULL)
	{
		for (int i = (int)CameraID::Game1; i < (int)CameraID::Game4; i++)
		{
			if (*EngineOptions::render_game_aspect_shrink_height == true)
			{
				float aspect = (float)display->Get_Height() / (float)display->Get_Width();
				m_cameras[CameraID::Game1 + i]->Set_Viewport(Rect2D(0.0f, 0.0f, (float)*EngineOptions::render_game_width, (float)*EngineOptions::render_game_width * aspect));
			}
			else
			{
				float aspect = (float)display->Get_Width() / (float)display->Get_Height();
				m_cameras[CameraID::Game1 + i]->Set_Viewport(Rect2D(0.0f, 0.0f, (float)*EngineOptions::render_game_height * aspect, (float)*EngineOptions::render_game_height));	
			}
		}

 		m_cameras[CameraID::UI]->Set_Viewport				(Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
		m_cameras[CameraID::Editor_Main]->Set_Viewport		(Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
		m_cameras[CameraID::Editor_Tileset]->Set_Viewport	(Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
		m_cameras[CameraID::Editor_Sub]->Set_Viewport		(Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	}

	for (int i = 0; i < CameraID::COUNT; i++)
	{
		Rect2D viewport = m_cameras[i]->Get_Viewport();
		DBG_LOG("Camera[%i] Viewport=%.0f,%.0f,%.0f,%.0f", i, viewport.X, viewport.Y, viewport.Width, viewport.Height);
	}

	// Enable/Disable gfx effects.
	Renderer::Get()->Set_VSync(*GameOptions::vsync_enabled);

	// Turn distortion off.
	RenderPipeline::Get()->Get_Pass_From_Name("distortion")->Enabled = *GameOptions::distortion_enabled;

	// Reduce graphical effects.
	if (*EngineOptions::nogui)
	{
		// If running without gui (dedicated server etc), skip all the particles we can.
		ParticleManager::Get()->Set_Skip_Modulus(1);
	}
	else
	{
		int skip_particle_modulus = Max(1, (int)(10 - (*GameOptions::graphics_quality * 10)));
		ParticleManager::Get()->Set_Skip_Modulus(skip_particle_modulus);
	}

	// Save configuration.
	Save_User_Config();
}

void Game::Start()
{
	GfxDisplay* display = GfxDisplay::Get();
	
	// Setup cameras.
	Setup_Cameras();

	// Setup editor related stuff.
	Setup_Editor();

	// Apply input bindings.
	Apply_Config();

	// Push the game state.
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	manager->Go(UIAction::Push(new UIScene_Base()));
}

Camera* Game::Get_Camera(CameraID::Type camera)
{
	return m_cameras[(int)camera];
}

GameMode* Game::Get_Game_Mode()
{
	return m_game_mode;
}

CVMObjectHandle Game::Get_Script_Canvas()
{
	return m_canvas_instance;
}

void Game::End()
{
	// Final save before unloading eveyrthing.
	*GameOptions::graceful_shutdown = true;
	Save_User_Config();

	PersistentStorage::Get()->Wait_For_Pending_Writes();

	// Bllllllllllllah dependency issues :(
	/*
	/// Annnnd unload.
	for (int i = 0; i < CameraID::COUNT; i++)
	{
		SAFE_DELETE(m_cameras[i]);
	}

	DemoManager::Destroy();
	ReportManager::Destroy();
	SAFE_DELETE(m_post_process_manager);
	SAFE_DELETE(m_boid_manager);
	SAFE_DELETE(m_particle_manager);
	SAFE_DELETE(m_collision_manager);
	SAFE_DELETE(m_path_manager);
	SAFE_DELETE(m_item_manager);
	SAFE_DELETE(m_skill_manager);
	SAFE_DELETE(m_dlc_manager);
	SAFE_DELETE(m_statistics_manager);
	SAFE_DELETE(m_rankings_manager);
	SAFE_DELETE(m_achievement_manager);
	SAFE_DELETE(m_game_mode);
	SAFE_DELETE(m_map);
	SAFE_DELETE(m_tag_replacer);
	SAFE_DELETE(m_chat_manager);
	SAFE_DELETE(m_vote_manager);
	SAFE_DELETE(m_actor_replicator); 
	SAFE_DELETE(m_editor_manager);
	SAFE_DELETE(m_profile_manager);
	SAFE_DELETE(m_scene);
	*/
}

void Game::Tick(const FrameTime& time)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	// Switch steam state.
	UIScene* focusScene = manager->Get_Top_Focusable_Screen("UIScene_Game");
	if (focusScene != NULL && focusScene->Get_Name() == "UIScene_Game")
	{
		Steam_JoystickState::Set_Global_Mode("InGame");
	}
	else
	{
		Steam_JoystickState::Set_Global_Mode("InMenu");
	}

	// Update demo state.
	Update_Demo();

	// Monitor server go!
	if (m_monitor_server != NULL)
	{
		m_monitor_server->Tick(time);
	}

	// Update scene viewport.
	m_scene->Calculate_Full_Camera_Bounding_Box();
	if (m_game_mode != NULL)
	{
		AudioRenderer::Get()->Set_Listener_Count(m_game_mode->Get_Camera_Count());
	}

	// Can we accept invite?
	if (OnlineMatching::Get()->Invite_Pending() == true)
	{
		int invite_index = OnlineMatching::Get()->Invite_Index();

		if (manager->Can_Accept_Invite() && invite_index != m_last_accepted_invite_index)
		{
			if (!m_invite_login_shown)
			{
				DBG_LOG("Starting invite flow for invite: %i...", invite_index);

				if (manager->Get_Scene_By_Type<UIScene_PlayerLogin*>() == NULL)
				{
					manager->Go(UIAction::Pop(NULL, "UIScene_MainMenu"));
					manager->Go(UIAction::Push(new UIScene_PlayerLogin(true), NULL));
				}

				m_invite_login_shown = true;
			}
			else
			{
				bool any_logins = false;

				std::vector<OnlineUser*> local_users = OnlinePlatform::Get()->Get_Local_Users();
				for (std::vector<OnlineUser*>::iterator iter = local_users.begin(); iter != local_users.end(); iter++)
				{
					OnlineUser* user = *iter;
					if (user->Get_Input_Source().Device != InputSourceDevice::NONE &&
						user->Get_Profile_Index() >= 0)
					{
						any_logins = true;
						break;
					}
				}

				if (manager->Get_Scene_By_Type<UIScene_PlayerLogin*>() == NULL)
				{
					DBG_LOG("Ignoring invite %i, player did not login ...", invite_index);

					m_last_accepted_invite_index = invite_index;
					m_invite_login_shown = false;
				}
				else if (any_logins)
				{
					DBG_LOG("Accepting invite %i ...", invite_index);

					// If we have no lobby screen, there is nothing to clean up the network state, so lets do it here.
					if (GameEngine::Get()->Get_UIManager()->Get_Scene_By_Name("UIScene_Lobby") == NULL)
					{
						GameNetManager::Get()->Leave_Network_Game();
					}

					manager->Go(UIAction::Pop(NULL, "UIScene_PlayerLogin"));
					manager->Go(UIAction::Push(new UIScene_FindGame(true), new UIFadeInTransition()));	

					m_last_accepted_invite_index = invite_index;
					m_invite_login_shown = false;
				}
			}
		}
	}

	// Do global tick.
	if (!m_loading)
	{
		// Tick managers.
		{
			PROFILE_SCOPE("Statistics Manager");
			STATISTICS_TIMED_SCOPE(g_game_statistics_time);

			m_statistics_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("Rankings Manager");
			STATISTICS_TIMED_SCOPE(g_game_rankings_time);

			m_rankings_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("Achievement Manager");
			STATISTICS_TIMED_SCOPE(g_game_achievements_time);

			m_achievement_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("Challenge Manager");

			m_challenge_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("DLC Manager");
			STATISTICS_TIMED_SCOPE(g_game_dlc_time);

			m_dlc_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("Profile Manager");
			STATISTICS_TIMED_SCOPE(g_game_profile_time);

			m_profile_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("Report Manager");

			ReportManager::Get()->Tick(time);
		}
		{
			PROFILE_SCOPE("Demo Manager");

			DemoManager::Get()->Set_Can_Record(m_game_update_enabled && !GameNetManager::Get()->Is_Dedicated_Server());
			DemoManager::Get()->Tick(time);
		}


		STATISTICS_TIMED_SCOPE(g_game_global_script_time);
		ScriptEventListener::Fire_On_Global_Tick();
	}

	if (!GameNetManager::Get()->Is_Map_Load_Pending())
	{
		// Tick if we are in-game.
		if (m_game_update_enabled == true)
		{
			Tick_In_Game(time);

			if (m_admin_server != NULL)
			{
				m_admin_server->Tick(time);
			}
		}

		// If we can't tick the game, at least tick the cameras
		// so we can move in editor etc.
		if (m_game_update_enabled == false || m_update_actors == false)
		{		
			std::vector<Camera*> cameras = GameEngine::Get()->Get_Scene()->Get_Cameras();
			for (std::vector<Camera*>::iterator iter = cameras.begin(); iter != cameras.end(); iter++)
			{
				(*iter)->Tick(time);
			}
		}
	}

	Tick_Vibrations(time);

	// Despawn any actors that have requested it.,
	{
		STATISTICS_TIMED_SCOPE(g_game_scene_despawn_time);
		m_scene->Run_Despawns();
	}

	// Run pending saves.
	if (m_save_queued == true)
	{
		// TODO: To another thread please!
		Save_User_Config();
		m_save_queued = false;
	}

	// If we are a dedi server, periodically dump out stats to tty.
	if (NetManager::Get()->Is_Dedicated_Server())
	{
		double ticks = Platform::Get()->Get_Ticks();
		if (ticks > m_next_server_stats_print_time)
		{
			DBG_LOG("Server Stats [ avg_tick_rate=%.1f, frame_time=%.1f, fps=%.1f reserved_id=%llu reserved_time=%.2f ]",
				ActorReplicator::Get()->Get_Average_Server_Tick_Rate(),
				(float)time.Get_Frame_Time_No_Sync(),
				(float)time.Get_FPS(),
				NetManager::Get()->Server()->Get_Reserved_Lobby_ID(),
				NetManager::Get()->Server()->Get_Reserved_Lobby_Time()
			);
			m_next_server_stats_print_time = ticks + 5000.0f;
		}
	}
}

#ifdef VM_ALLOW_MULTITHREADING
void ScriptTickTask::Run()
{
	Game::Get()->Run_Script_Ticks(Task_Index);
}

void Game::Run_Script_Ticks(int task_index)
{
	PROFILE_SCOPE("Script Tick Task");

	// Simulate our batch of emitters.
	int listeners_per_worker = m_script_task_listeners.size() / m_script_task_count;
	int start_index			 = listeners_per_worker * task_index;
	int end_index			 = start_index + listeners_per_worker;

	if (task_index == m_script_task_count - 1)
	{
		end_index += m_script_task_listeners.size() % m_script_task_count;
	}

	for (int i = start_index; i < end_index; i++)
	{
		ScriptEventListener* listener = m_script_task_listeners[i];
		listener->On_Tick();
	}
}
#endif

void Game::Tick_In_Game(const FrameTime& time)
{	
	GameNetManager* manager = GameNetManager::Get();

	if (!manager->Is_Game_Active())	
	{
		return;
	}

	Update_Local_Idle_Timer(time);
	
	{
		PROFILE_SCOPE("Collect Deferred");			
		STATISTICS_TIMED_SCOPE(g_game_collect_deferred_time);

		GameEngine::Get()->Get_Scene()->Collect_Deferred(time);
	}

	if (!DemoManager::Get()->Is_Playing())
	{
		if (m_update_actors)
		{
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();

			{
				PROFILE_SCOPE("Non-Scene Tick");
				STATISTICS_TIMED_SCOPE(g_game_non_scene_script_time);

				// Tick non-scene based objects (achievements etc).
				std::vector<ScriptEventListener*> listeners = ScriptEventListener::g_script_event_listeners; 
				for (std::vector<ScriptEventListener*>::iterator iter = listeners.begin(); iter != listeners.end(); iter++)
				{
					ScriptEventListener* listener = *iter;
					ScriptedActor* actor = listener->Get_Actor();

					if (actor == NULL && !listener->Get_Manual_Tick())
					{
						listener->On_Tick();
					}
				}
				/*
				static bool bDumped = false;
				if (!bDumped)
				{
					bDumped = true;
					DBG_LOG("====================== On_Tick Subscribers ======================");
					for (std::vector<ScriptEventListener*>::iterator iter = listeners.begin(); iter != listeners.end(); iter++)
					{
						ScriptEventListener* listener = *iter;
						if (!listener->Get_Manual_Tick())
						{
							if (listener->m_event_On_Tick)
							{
								DBG_LOG("%s", listener->Get_Object()->Get_Symbol()->symbol->name);
							}
						}
					}
				}*/
			}

			{
				PROFILE_SCOPE("Scene Tick");	
				STATISTICS_TIMED_SCOPE(g_game_scene_script_time);

				// Calculate screen viewports which is used to work out what actors should tick.
				std::vector<Rect2D> screen_views;
				std::vector<GameNetUser*> users = manager->Get_Game_Net_Users();
				for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
				{
					GameNetUser* user = *iter;
					Rect2D view = user->Get_Estimated_Viewport();
					screen_views.push_back(view);
				}

				GameEngine::Get()->Get_Scene()->Tick(time, screen_views);
			}

			// Run latent execution of contexts.
			{
				PROFILE_SCOPE("Latent Execution");
				STATISTICS_TIMED_SCOPE(g_game_script_latent_time);

				vm->Execute_All_Contexts(time.Get_Frame_Time());
			}
		}
		// If we don't want to tick entire scene, at least tick map for anims etc.
		else
		{
			{
				PROFILE_SCOPE("Map Tick");
				STATISTICS_TIMED_SCOPE(g_game_map_time);

				m_map->Tick(time);
			}
		}
	}

	if (!DemoManager::Get()->Is_Playing())
	{
		PROFILE_SCOPE("Game Managers");
		STATISTICS_TIMED_SCOPE(g_game_managers_time);

		if (m_update_actors)
		{
			{
				STATISTICS_TIMED_SCOPE(g_game_mode_time);
				m_game_mode->Tick(time);
			}
			{
				STATISTICS_TIMED_SCOPE(g_boid_manager_time);
				m_boid_manager->Tick(time);
			}
		}

		{
			STATISTICS_TIMED_SCOPE(g_chat_manager_time);
			m_chat_manager->Tick(time);
		}
		{
			STATISTICS_TIMED_SCOPE(g_vote_manager_time);
			m_vote_manager->Tick(time);
		}
		{
			STATISTICS_TIMED_SCOPE(g_actor_manager_time);
			m_actor_replicator->Tick(time);
		}
		{
			STATISTICS_TIMED_SCOPE(g_editor_manager_time);
			m_editor_manager->Tick(time);
		}
		{
			STATISTICS_TIMED_SCOPE(g_post_process_manager_time);
			m_post_process_manager->Tick(time);
		}
		{
			PROFILE_SCOPE("Tutorial Manager");
			m_tutorial_manager->Tick(time);
		}
	}

	// Make sure this goes after game update, as it relies on damage-acceptance
	// events to know when to despawn projectiles.
	{
		PROFILE_SCOPE("Particle Update");	
		STATISTICS_TIMED_SCOPE(g_game_particle_time);

		if (!DemoManager::Get()->Is_Playing() || !DemoManager::Get()->Is_Paused())
		{
			m_particle_manager->Tick(time);
		}
	}

	{
		PROFILE_SCOPE("Create Deferred");
		STATISTICS_TIMED_SCOPE(g_game_create_deferred_time);

		GameEngine::Get()->Get_Scene()->Create_Deferred(time);
	}

	{
		PROFILE_SCOPE("Collision Update");	
		STATISTICS_TIMED_SCOPE(g_game_collision_time);
		
		m_collision_manager->Tick(time);
	}

	{
		PROFILE_SCOPE("Path Update");	
		STATISTICS_TIMED_SCOPE(g_game_path_update_time);

		m_path_manager->Tick(time);
	}

	// Generate inventory.
	if (!DemoManager::Get()->Is_Playing())
	{
		if (NetManager::Get()->Client() != NULL)
		{
			if (OnlineInventory::Try_Get() != NULL)
			{
				if (m_players_idle_timer > 0.0f)
				{
					OnlineInventory::Get()->Generate();
				}
			}
		}
	}
}

void Game::Reset_Local_Idle_Timer()
{
	m_players_idle_timer = IDLE_TIMER_MAX;
}

void Game::Update_Local_Idle_Timer(const FrameTime& time)
{
	m_players_idle_timer -= time.Get_Delta_Seconds();
	DBG_ONSCREEN_LOG(StringHelper::Hash("IDLE"), Color::Blue, 1.0f, "Local players are: %s", m_players_idle_timer > 0.0f ? "ACTIVE" : "IDLE");
}

void Game::Check_Inventory_Drops()
{
	// Check inventory of logged in users has space for drops.
	for (int i = 0; i < OnlinePlatform::Get()->Get_Local_User_Count(); i++)
	{
		OnlineUser* user = OnlinePlatform::Get()->Get_Local_User_By_Index(i);		
		Profile* profile = m_profile_manager->Get_Profile(user->Get_Profile_Index());
		if (profile->Get_Free_Slots() <= 0)
		{
			return;
		}
	}

	// Generate inventory.
	if (NetManager::Get()->Client() != NULL)
	{
		if (OnlineInventory::Try_Get() != NULL)
		{
			OnlineInventory::Get()->CheckForDrops(INVENTORY_ITEM_PLAYTIME_DROP_ID);
		}
	}
}

bool Game::Get_Next_Inventory_Drop(OnlineInventoryItem& item)
{
	if (OnlineInventory::Try_Get() != NULL)
	{
		if (m_inventory_drops.size() <= 0)
		{
			// Generate :inventory.
			if (NetManager::Get()->Client() != NULL)
			{
				m_inventory_drops = OnlineInventory::Get()->GetDroppedItems();
			}
		}
		else
		{
			item = m_inventory_drops[0];
			m_inventory_drops.erase(m_inventory_drops.begin());
			return true;
		}
	}
	return false;
}

void Game::Draw_Stat_Group(float& offset, float& x_offset, Stat** stats, int stat_count, std::string path, int depth)
{
	if (offset > GfxDisplay::Get()->Get_Height() * 0.75f)
	{
		x_offset += 270.0f;
		offset = 20.0f;
	}

	// Draw title.
	Draw_Stat(path, 0.0f, 0.0f, Rect2D(x_offset, offset, 250.0f, 12.0f), Color::Green, false, false, true);
	offset += 16.0f;

	// Draw all sub-stats.
	for (int i = 0; i < stat_count; i++)
	{
		Stat* stat = stats[i];
		if (!stat->Display)
		{
			continue;
		}

		if (stat->Path_Count == depth + 1)
		{
			std::string base = "";
			for (int j = 0; j < depth; j++)
			{
				if (base != "")
				{
					base += "/";
				}
				base += stat->Path[j];
			}

			if (base == path)
			{
				Draw_Stat(stat->Path[depth], stat->To_Float(), 0.0f, Rect2D(x_offset, offset, 250.0f, 12.0f), Color::White, true, false, false);
				offset += 16.0f;
			}
		}
	}

	// Draw all sub-groups.
	std::vector<std::string> drawn_groups;
	for (int i = 0; i < stat_count; i++)
	{
		Stat* stat = stats[i];
		if (!stat->Display)
		{
			continue;
		}

		if (stat->Path_Count > depth + 1)
		{
			std::string base = "";
			for (int j = 0; j < depth; j++)
			{
				if (base != "")
				{
					base += "/";
				}
				base += stat->Path[j];
			}

			if (base == path)
			{
				if (std::find(drawn_groups.begin(), drawn_groups.end(), base) == drawn_groups.end())
				{
					Draw_Stat_Group(offset, x_offset, stats, stat_count, base, depth + 1);
					drawn_groups.push_back(base);
				}
			}
		}
	}

	// Spacing!
	if (depth == 1)
	{
		offset += 5.0f;
	}
}

void Game::Draw_Onscreen_Logs(const FrameTime& time)
{
#ifdef MASTER_BUILD
	return;
#endif

	UIManager* ui = GameEngine::Get()->Get_UIManager();
	MarkupFontRenderer font_renderer(ui->Get_Font(), false, true);

	std::vector<PersistentLog> logs = PersistentLogHelper::GetActive();
	float y_offset = 0.0f;

	for (std::vector<PersistentLog>::iterator iter = logs.begin(); iter != logs.end(); iter++)
	{
		PersistentLog& log = *iter;
		font_renderer.Draw_String(log.message.c_str(), Vector2(10.0f, 10.0f + y_offset), 12.0f, log.color);
		y_offset += 14.0f;
	}

	PersistentLogHelper::Tick(time.Get_Delta_Seconds());
}

void Game::Draw_Watermark(const FrameTime& time)
{
	return;

	GfxDisplay* display = GfxDisplay::Get();

	float height = display->Get_Height() * 0.075f;
	float padding = display->Get_Height() * 0.02f;

	UIManager* ui = GameEngine::Get()->Get_UIManager();
	MarkupFontRenderer font_renderer(ui->Get_Font(), false, true);
	font_renderer.Draw_String(
		StringHelper::Format("EARLY ACCESS - %s", VersionInfo::FULLVERSION_STRING).c_str(), 
		Rect2D(0.0f, display->Get_Height() - height - padding, (float)display->Get_Width() - padding, height), 
		16.0f, 
		Color(255.0f, 255.0f, 255.0f, 64.0f), 
		TextAlignment::Right, 
		TextAlignment::ScaleToFit);
}

void Game::Draw_Autosave(const FrameTime& time)
{
	m_save_icon_timer -= time.Get_Delta_Seconds();
	if (m_save_icon_timer > 0.0f)
	{
		m_save_icon_alpha = Math::Lerp(m_save_icon_alpha, 1.0f, 2.0f * time.Get_Delta_Seconds());
	}
	else
	{
		m_save_icon_alpha = Math::Lerp(m_save_icon_alpha, 0.0f, 5.0f * time.Get_Delta_Seconds());
	}

	float alpha = m_save_icon_alpha;

	// Add a fade-in-out sine bob.
	m_save_icon_sin_bob_timer += time.Get_Delta_Seconds();
	alpha *= (sin(m_save_icon_sin_bob_timer * 7.0f) + 1.0f) / 2.0f;  

	AtlasRenderer renderer;
	renderer.Draw_Frame("autosave_icon", Vector2(10.0f, 10.0f), 0.0f, Color(255.0f, 255.0f, 255.0f, 255.0f * alpha), false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, GameEngine::Get()->Get_UIManager()->Get_UI_Scale() * Vector2(0.5f, 0.5f));
}

void Game::Draw_Uploading_Icon(const FrameTime& time)
{
	m_uploading_icon_alpha = Math::Lerp(m_uploading_icon_alpha, ReportManager::Get()->Is_Uploading() ? 1.0f : 0.0f, 2.0f * time.Get_Delta_Seconds());
	m_uploading_icon_rotation += 180.0f * time.Get_Delta_Seconds();

	float display_w = GfxDisplay::Get()->Get_Width();
	float display_h = GfxDisplay::Get()->Get_Height();

	AtlasRenderer renderer;
	renderer.Draw_Frame(
		"uploading_icon", 
		Vector2(display_w - (16.0f + 16.0f), (16.0f + 16.0f)), 
		0.0f, 
		Color(255.0f, 194.0f, 14.0f, 255.0f * m_uploading_icon_alpha),
		false, 
		false, 
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha,
		GameEngine::Get()->Get_UIManager()->Get_UI_Scale() * Vector2(0.30f, 0.30f),
		m_uploading_icon_rotation
	);
}

void Game::Draw_Overlays(const FrameTime& time)
{
	Draw_Autosave(time);
	Draw_Uploading_Icon(time);

	if (!(*GameOptions::show_perf_overlay) && !(*GameOptions::show_extended_perf_overlay))
	{
		return;
	}

	float gfx_width = (float)GfxDisplay::Get()->Get_Width();
	float gfx_height = (float)GfxDisplay::Get()->Get_Height();

	Stat** stats;
	int stats_count = StatManager::Get_Statistics(stats);

	// Draw stat list in top-left.
	float offset = 20.0f;
	float x_offset = 20.0f;

	if (*GameOptions::show_extended_perf_overlay)
	{
		// Draw top-level stat tree.
		std::vector<std::string> drawn_groups;
		for (int i = 0; i < stats_count; i++)
		{
			Stat* stat = stats[i];
			if (!stat->Display)
			{
				continue;
			}
			if (stat->Path_Count == 2)
			{
				if (std::find(drawn_groups.begin(), drawn_groups.end(), stat->Path[0]) == drawn_groups.end())
				{
					Draw_Stat_Group(offset, x_offset, stats, stats_count, stat->Path[0], 1);
					drawn_groups.push_back(stat->Path[0]);
				}
			}
		}
	}

/*	Draw_Stat("Engine Timings", 0.0f, 0.0f, Rect2D(20.0f, offset, 250.0f, 12.0f), Color::Green, false, false, true);
	offset += 16.0f;
	Draw_Stat("Particle Count", 1000.0f, 30000.0f, Rect2D(20.0f, offset, 250.0f, 12.0f), Color::White, false, false, false);
	*/

	Rect2D graph_bounds = Rect2D(gfx_width - (float)GameStatValue::max_history - 20.0f, gfx_height - 300.0f - 20.0f - (16 * 9), (float)GameStatValue::max_history, 300.0f);

	// Draw timing in bottom-left.
	offset = graph_bounds.Y + graph_bounds.Height + 16.0f;
	Draw_Stat("FPS", (1000.0f / (float)time.Get_Frame_Time()), 60.0f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::White, false, false);
	offset += 16.0f;
	Draw_Stat("Frame", time.Get_Frame_Time(), 16.6f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Green, true, true);
	offset += 16.0f;
	Draw_Stat("CPU", time.Get_Update_Time(), 16.6f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Red, true);
	offset += 16.0f;
	Draw_Stat("Draw", time.Get_Render_Time(), 16.6f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Blue, true);
	offset += 16.0f;
	Draw_Stat("GPU", time.Get_GPU_Time(), 16.6f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Yellow, true);
	offset += 16.0f;
	Draw_Stat("VSync", time.Get_VSync_Time(), 16.6f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Magenta, true);
	offset += 16.0f;
	Draw_Stat("Net In", NetConnection::GetInRate() / 1024.0f, (NetManager::Get()->Is_Server() ? *EngineOptions::net_server_target_rate : *EngineOptions::net_client_target_rate) / 1024.0f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Orange, true);
	offset += 16.0f;
	Draw_Stat("Net Out", NetConnection::GetOutRate() / 1024.0f, (!NetManager::Get()->Is_Server() ? *EngineOptions::net_server_target_rate : *EngineOptions::net_client_target_rate) / 1024.0f, Rect2D(graph_bounds.X, offset, graph_bounds.Width, 12.0f), Color::Gray, true);
	offset += 16.0f;

	// Draw graph in bottom-right.
	PrimitiveRenderer primitive_renderer;
	primitive_renderer.Draw_Wireframe_Quad(graph_bounds, 0.0f, Color::Gray, 1.0f);
	primitive_renderer.Draw_Line(Vector3(graph_bounds.X - 4.0f, graph_bounds.Y + (graph_bounds.Height * 0.5f), 0.0f), Vector3(graph_bounds.X + graph_bounds.Width + 4.0f, graph_bounds.Y + (graph_bounds.Height * 0.5f), 0.0f), 1.0f, Color::Orange);

	// Draw states.
	for (HashTable<GameStatValue, int>::Iterator iter = m_game_stats.Begin(); iter != m_game_stats.End(); iter++)
	{
		GameStatValue& value = *iter;
		if (!value.show_on_graph)
		{
			continue;
		}

		x_offset = graph_bounds.X + Max(0, graph_bounds.Width - value.previous_values.Size());
		for (int i = 0; i < value.previous_values.Size() - 1; i++)
		{
			float next = value.previous_values.Peek(i + 1);
			float prev = value.previous_values.Peek(i);

			float next_fraction = (next / value.target) * 0.5f;
			float prev_fraction = (prev / value.target) * 0.5f;

			float next_y_offset = graph_bounds.Y + (graph_bounds.Height * (1.0f - next_fraction));
			float prev_y_offset = graph_bounds.Y + (graph_bounds.Height * (1.0f - prev_fraction));

			primitive_renderer.Draw_Line(
				Vector3(x_offset - 1, prev_y_offset, 0.0f),
				Vector3(x_offset, next_y_offset, 0.0f),
				1.0f, 
				value.color
			);

			x_offset += 1.0f;
		}
	}
}

void Game::Draw_Stat(std::string name, float value, float target, Rect2D bounds, Color base_color, bool bLowerIsBetter, bool bShowOnGraph, bool bAsBar)
{
	UIManager* ui = GameEngine::Get()->Get_UIManager();
	MarkupFontRenderer font_renderer(ui->Get_Font(), false, true);
	PrimitiveRenderer primitive_renderer;

	// Add stat.
	GameStatValue* stat;
	int hash = StringHelper::Hash(name.c_str());
	if (m_game_stats.Get_Ptr(hash, stat))
	{
		stat->previous_values.Push(value);
	}
	else
	{
		GameStatValue new_stat;
		new_stat.name = name;
		new_stat.color = base_color;
		new_stat.previous_values.Push(value);
		new_stat.target = target;
		new_stat.lower_is_better = bLowerIsBetter;
		new_stat.show_on_graph = bShowOnGraph;

		m_game_stats.Set(hash, new_stat);

		m_game_stats.Get_Ptr(hash, stat);
	}

	// Work out min/max/average.
	float min = 99999999.0f;
	float max = -99999999.0f;
	float avg = 0.0f;

	for (int i = 0; i < stat->previous_values.Size(); i++)
	{
		float val = stat->previous_values.Get(i);
		if (val < min)
		{
			min = val;
		}
		if (val > max)
		{
			max = val;
		}
		avg += val;
	}

	avg /= (float)stat->previous_values.Size();

	// Work out colors.
	Color color = Color::Green;
	if (target > 0.01f)
	{
		if (!bLowerIsBetter)
		{
			if (value < target * 0.33f)
			{
				color = Color::Red;
			}
			else if (value < target * 0.66f)
			{
				color = Color::Yellow;
			}
		}
		else
		{
			if (value > target * 1.3f)
			{
				color = Color::Red;
			}
			else if (value > target * 1.1f)
			{
				color = Color::Yellow;
			}
		}
	}

	primitive_renderer.Draw_Solid_Quad(Rect2D(bounds.X - 2, bounds.Y - 2, bounds.Width + 4, bounds.Height + 4), Color(0, 0, 0, 175));
	font_renderer.Draw_String(name.c_str(), bounds, 12.0f, base_color, TextAlignment::Left, TextAlignment::Center);

	if (bAsBar)
	{
		return;
	}

	// Draw value.
	font_renderer.Draw_String(StringHelper::Format("%.1f", value).c_str(), Rect2D(bounds.X, bounds.Y, bounds.Width * 0.55f, bounds.Height), 12.0f, color, TextAlignment::Right, TextAlignment::Center);

	// Draw min/max/average
	font_renderer.Draw_String(StringHelper::Format("%.1f", min).c_str(), Rect2D(bounds.X, bounds.Y, bounds.Width * 0.7f, bounds.Height), 12.0f, color, TextAlignment::Right, TextAlignment::Center);
	font_renderer.Draw_String(StringHelper::Format("%.1f", max).c_str(), Rect2D(bounds.X, bounds.Y, bounds.Width * 0.85f, bounds.Height), 12.0f, color, TextAlignment::Right, TextAlignment::Center);
	font_renderer.Draw_String(StringHelper::Format("%.1f", avg).c_str(), Rect2D(bounds.X, bounds.Y, bounds.Width * 1.0f, bounds.Height), 12.0f, color, TextAlignment::Right, TextAlignment::Center);
}

void Game::Set_Update_Game(bool enabled)
{
	m_game_update_enabled = enabled;
}

void Game::Set_Loading(bool enabled)
{
	m_loading = enabled;
}

void Game::Set_Sub_Menu_Open(bool enabled)
{
	m_sub_menu_open = enabled;
}

void Game::Set_Update_Actors(bool enabled)
{
	m_update_actors = enabled;
}

bool Game::Is_Sub_Menu_Open()
{
	return m_sub_menu_open;
}

void Game::Convert_Legacy_Maps()
{
	std::vector<std::string> map_files;
	map_files.push_back("arena_circle");
	map_files.push_back("arena_snowbound");
	map_files.push_back("bc_forest");
	map_files.push_back("c1_s1");
	map_files.push_back("ctb_heist");
	map_files.push_back("dc_random");
	map_files.push_back("dm_street");
	map_files.push_back("gg_town");
	map_files.push_back("ru_lab");
	map_files.push_back("tdm_district");
	map_files.push_back("tutorial");
	map_files.push_back("wave_cliff");
	map_files.push_back("wave_crowd");
	map_files.push_back("wave_egypt");
	map_files.push_back("wave_forest");
	map_files.push_back("wave_tiny");
	map_files.push_back("wave_urban");

	for (std::vector<std::string>::iterator iter = map_files.begin(); iter != map_files.end(); iter++)
	{
		LegacyMapConverter converter;
		std::string name = *iter;
		
		std::string from = StringHelper::Format("../../BlitzMax/Bin/Maps/%s.map", name.c_str());
		std::string to   = StringHelper::Format("../Data/Base/Maps/%s/%s.zgmap", name.c_str(), name.c_str());

		converter.Convert(from.c_str(), to.c_str());
	}
}

bool Game::Check_Network_Disconnect()
{
	NetClient* net_client = NetManager::Get()->Client();
	OnlineMatching* matching = OnlineMatching::Get();
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	std::string error = "";

	// Only do network disconnects if we are a client.
	if (net_client != NULL)
	{
		// Check network connection.
		switch (net_client->Get_State())
		{
		// All good!
		case NetClient_State::Connected:
			{
				break;
			}

		// Error :(
		default:
			{
				error = net_client->Get_Error_Message(net_client->Get_Error());
				break;
			}
		}

		// Check lobby connection.
		switch (matching->Get_State())
		{
		// All good!
		case OnlineMatching_State::Idle:
		case OnlineMatching_State::In_Lobby:
			{
				break;
			}
			
		// Error :(
		default:
			{
				error = matching->Get_Error_Message(matching->Get_Error());				
				break;
			}
		}
	}

	if (error != "")
	{
		DBG_LOG("Encountered disconnect, showing error '%s'.", error.c_str());

		NetManager::Get()->Leave_Network_Game();

		if (manager->Get_Scene_By_Name("UIScene_GameSetupMenu") != NULL)
			manager->Go(UIAction::Pop(NULL, "UIScene_GameSetupMenu"));
		else
		{
			manager->Go(UIAction::Pop(NULL, "UIScene_PlayerLogin"));
		}

		manager->Go(UIAction::Push(new UIScene_Dialog(error)));

		return true;
	}
	else
	{
		return false;
	}
}

void Game::Finalize_Loading()
{
	GameEngine::Get()->Get_VM()->Register_Scripts();
	
	if (!*EngineOptions::server)
	{
		if (!AchievementManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize achievement manager.");
		}
		if (!ChallengeManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize challenge manager.");
		}
		if (!StatisticsManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize statistics manager.");
		}
		if (!RankingsManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize rankings manager.");
		}
		if (!DLCManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize dlc manager.");
		}
		if (!TutorialManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize tutorial manager.");
		}
		if (!ReportManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize report manager.");
		}
		if (!DemoManager::Get()->Init())
		{
			DBG_ASSERT_STR(false, "Failed to initialize demo manager.");
		}
	}
	if (!ProfileManager::Get()->Init())
	{
		DBG_ASSERT_STR(false, "Failed to initialize profile manager.");
	}
	if (!ItemManager::Get()->Init())
	{
		DBG_ASSERT_STR(false, "Failed to initialize item manager.");
	}
	if (!SkillManager::Get()->Init())
	{
		DBG_ASSERT_STR(false, "Failed to initialize skill manager.");
	}	
	if (!GamePostProcessManager::Get()->Init())
	{
		DBG_ASSERT_STR(false, "Failed to initialize post process manager.");
	}

	if (*EngineOptions::server && *EngineOptions::nogui)
	{
		std::string username = *GameOptions::admin_username;
		std::string password = *GameOptions::admin_password;
		if (username != "" && password != "")
		{
			m_admin_server = new AdminServer();
			m_admin_server->Init();
		}
	}

#ifndef MASTER_BUILD
	if (*GameOptions::monitor_enabled)
	{
		m_monitor_server = new MonitorServer();
		m_monitor_server->Init();
	}
#endif

	// Setup hud canvas.
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();	
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* canvas_symbol = vm->Find_Class("Canvas");
	DBG_ASSERT(canvas_symbol != NULL);

	m_canvas_instance = vm->New_Object(canvas_symbol, false, &m_canvas);
	m_canvas.Set_Font(ResourceFactory::Get()->Get_Font("UI"));

	// Grab symbols we care about.
	m_event_particle_class = vm->Find_Class("Event_Particle");		
	DBG_ASSERT(m_event_particle_class != NULL);

	// Enables saving/loading of save data. Which we could not
	// do until the scripts are registered. Applys previously
	// loaded data.
	SaveData::Enable_Save_Data();

	/*
	CVMLinkedSymbol* testsym = vm->Find_Symbol(0xb8d8fdd3);
	CVMLinkedSymbol* classsym = vm->Get_Symbol_Table_Entry(testsym->symbol->class_scope_index);
	DBG_LOG("SYM=%s::%s", testsym->symbol->name, classsym->symbol->name);
	*/
}

void Game::Finalize_Loading_Main_Thread()
{
	if (!ParticleManager::Get()->Init())
	{
		DBG_ASSERT_STR(false, "Failed to initialize particle manager manager.");
	}
	
	// Load whitelist.
	Load_Package_Whitelist();

	// Go through each resource package and work out if they need to be server-enforced
	// as well as if they will cause us to run in restricted mode.
	m_restricted_mode = false;

	std::vector<PackageFile*> packages = ResourceFactory::Get()->Get_Packages();
	for (std::vector<PackageFile*>::iterator iter = packages.begin(); iter != packages.end(); iter++)
	{
		PackageFile* file = *iter;

		// If file is signed, we can ignore all of this, we treat it 
		// as an official file if this is the case.
		if (!file->Is_Signed() && !file->Is_Whitelisted())
		{
			if (Does_Package_Touch_Blacklisted_Files(file))
			{
				DBG_ASSERT_STR(false, "Package '%s' touches blacklisted files. Unable to start.", file->Get_Name().c_str());
			}
			if (Does_Package_Touch_Greylisted_Files(file))
			{
				m_restricted_mode = true;
				m_restricted_mode_packages.push_back(file);
				DBG_LOG("Package '%s' caused us to enter restricted mode.", file->Get_Name().c_str());
			}
		}

		if (Does_Package_Touch_Enforced_Files(file))
		{
			m_server_enforced_packages.push_back(file);
			DBG_LOG("Package '%s' will be server-enforced.", file->Get_Name().c_str());
		}
	}
}

void Game::Load_Package_Whitelist()
{
	std::string path = "Data/Config/Mod_Whitelist.xml";

	// Always read from base, this file should never be overriden.
	Stream* stream = ResourceFactory::Get()->Open_Base(path.c_str(), StreamMode::Read);
	if (stream == NULL)
	{
		DBG_ASSERT_STR(false, "Unable to load mod whitelisting file. Cannot start without it.");
		return;
	}

	// Load source in a single string.
	int source_len = stream->Length();

	char* buffer = new char[source_len + 1];
	buffer[source_len] = '\0';
	DBG_ASSERT(buffer != NULL);

	stream->ReadBuffer(buffer, 0, source_len);

	// Try and parse XML.
	rapidxml::xml_document<>* document = new rapidxml::xml_document<>();
	try
	{
		document->parse<0>(buffer);
	}
	catch (rapidxml::parse_error error)
	{
		int offset = error.where<char>() - buffer;
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(buffer, offset, line, column);

		DBG_LOG("Failed to parse mod whitelist XML with error @ %i:%i: %s", line, column, error.what());
		delete stream;
		delete[] buffer;
		return;
	}

	rapidxml::xml_node<>* root = document->first_node("xml", 0, false);
	DBG_ASSERT(root != NULL);
	
	// Load all greylist patterns.
	rapidxml::xml_node<>* node = root->first_node("grey", 0, false);
	while (node != NULL)
	{
		std::string pattern = "";

		rapidxml::xml_attribute<>* sub = node->first_attribute("pattern", 0, false);
		if (sub != NULL)
		{
			pattern = sub->value();
		}

		if (pattern != "")
		{
			pattern = StringHelper::Lowercase(pattern.c_str());
			DBG_LOG("Loaded Mod Greylist Pattern: %s", pattern.c_str());
			m_mod_greylist_patterns.push_back(pattern);
		}
		node = node->next_sibling("grey", 0, false);
	}

	// Load all blacklist patterns.
	node = root->first_node("black", 0, false);
	while (node != NULL)
	{
		std::string pattern = "";

		rapidxml::xml_attribute<>* sub = node->first_attribute("pattern", 0, false);
		if (sub != NULL)
		{
			pattern = sub->value();
		}

		if (pattern != "")
		{
			pattern = StringHelper::Lowercase(pattern.c_str());
			DBG_LOG("Loaded Mod Blacklist Pattern: %s", pattern.c_str());
			m_mod_blacklist_patterns.push_back(pattern);
		}
		node = node->next_sibling("black", 0, false);
	}

	// Load all server-enforced patterns.
	node = root->first_node("server_enforced", 0, false);
	while (node != NULL)
	{
		std::string pattern = "";

		rapidxml::xml_attribute<>* sub = node->first_attribute("pattern", 0, false);
		if (sub != NULL)
		{
			pattern = sub->value();
		}

		if (pattern != "")
		{
			pattern = StringHelper::Lowercase(pattern.c_str());
			DBG_LOG("Loaded Mod Enforced Pattern: %s", pattern.c_str());
			m_mod_enforced_patterns.push_back(pattern);
		}
		node = node->next_sibling("server_enforced", 0, false);
	}

	// Clean up and return.
	delete stream;
	delete[] buffer;
	return;
}

bool Game::Does_Package_Touch_Blacklisted_Files(PackageFile* file)
{
	for (int i = 0; i < file->Get_Chunk_Count(); i++)
	{
	 	PackageFileChunk* chunk = file->Get_Chunk(i);
		const char* name = file->Get_String(chunk->name_offset);

		std::string normalised = StringHelper::Lowercase(name);

		for (unsigned int j = 0; j < m_mod_blacklist_patterns.size(); j++)
		{
			std::string& pattern = m_mod_blacklist_patterns[j];
			if (StringHelper::Match(normalised, pattern))
			{
				return true;
			}
		}
	}

	return false;
}

bool Game::Does_Package_Touch_Greylisted_Files(PackageFile* file)
{
	for (int i = 0; i < file->Get_Chunk_Count(); i++)
	{
		PackageFileChunk* chunk = file->Get_Chunk(i);
		const char* name = file->Get_String(chunk->name_offset);

		std::string normalised = StringHelper::Lowercase(name);

		for (unsigned int j = 0; j < m_mod_greylist_patterns.size(); j++)
		{
			std::string& pattern = m_mod_greylist_patterns[j];
			if (StringHelper::Match(normalised, pattern))
			{
				return true;
			}
		}
	}

	return false;
}

bool Game::Does_Package_Touch_Enforced_Files(PackageFile* file)
{
	for (int i = 0; i < file->Get_Chunk_Count(); i++)
	{
		PackageFileChunk* chunk = file->Get_Chunk(i);
		const char* name = file->Get_String(chunk->name_offset);

		// Scripts are always enforced.
		if (chunk->type == PackageFileChunkType::Script)
		{
			return true;
		}

		std::string normalised = StringHelper::Lowercase(name);

		for (unsigned int j = 0; j < m_mod_enforced_patterns.size(); j++)
		{
			std::string& pattern = m_mod_enforced_patterns[j];
			if (StringHelper::Match(normalised, pattern))
			{
				return true;
			}
		}
	}

	return false;
}

std::vector<PackageFile*> Game::Get_Restricted_Packages()
{
	return m_restricted_mode_packages;
}

std::vector<PackageFile*> Game::Get_Server_Enforced_Packages()
{
	return m_server_enforced_packages;
}

bool Game::In_Restricted_Mode()
{
	return m_restricted_mode || m_map_restricted_mode;
}

bool Game::In_Map_Restricted_Mode()
{
	return m_map_restricted_mode;
}

void Game::Set_Map_Restricted_Mode(bool bRestricted)
{
	m_map_restricted_mode = bRestricted;
}

// Called on client after first sync, and on server after first map load. Used
// to finalize the load (eg. grab game mode object etc).
void Game::Finalize_Map_Load()
{
	RenderPipeline::Get()->Set_Ambient_Lighting(Vector4(0.5f, 0.5f, 0.5f, 1.0f));

	m_game_mode->Start_Map();
}

void Game::Vibrate(Vector3 position, float duration, float intensity)
{
	GameVibration vib;
	vib.center = position;
	vib.duration = duration;
	vib.intensity = intensity;
	vib.elapsed = 0.0f;
	m_vibrations.push_back(vib);
}

void Game::Tick_Vibrations(const FrameTime& time)
{
	for (std::vector<GameVibration>::iterator iter = m_vibrations.begin(); iter != m_vibrations.end(); )
	{
		GameVibration& vib = *iter;
		vib.elapsed += time.Get_Frame_Time();
		if (vib.elapsed > vib.duration)
		{
			iter = m_vibrations.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	GameNetManager* manager = GameNetManager::Get();
	std::vector<NetUser*> users = manager->Get_Local_Net_Users();

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		Input* input = Input::Get(i);
		float intensity = 0.0f;

		ScriptedActor* player_actor = NULL;

		bool bIsJoystick = false;

		for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			GameNetUser* user = static_cast<GameNetUser*>(*iter);
			ScriptedActor* actor = user->Get_Controller();
			if (user->Get_Online_User()->Is_Local() && actor != NULL)
			{
				if (user->Get_Online_User()->Get_Input_Source().Device_Index == i)
				{
					if (user->Get_Online_User()->Get_Input_Source().Device == InputSourceDevice::Joystick)
					{
						bIsJoystick = true;
					}
					player_actor = actor;
					break;
				}
			}
		}

		if (bIsJoystick)
		{
			if (player_actor != NULL)
			{
				const float inner_radius = 50.0f;
				const float outer_radius = 300.0f;

				Vector3 center = player_actor->Get_Position();

				for (std::vector<GameVibration>::iterator iter = m_vibrations.begin(); iter != m_vibrations.end(); iter++)
				{
					GameVibration& vib = *iter;
					float distance = (vib.center - center).Length();
					float time_delta = vib.elapsed / vib.duration;

					float radius_delta = 1.0f - (Max(0.0f, distance - inner_radius) / (outer_radius - inner_radius));

					intensity += (radius_delta) * (vib.intensity * time_delta);
				}
			}

			intensity *= *GameOptions::joystick_vibration;

			input->Get_Joystick_State()->Set_Vibration(Clamp(intensity, 0.0f, 1.0f), Clamp(intensity, 0.0f, 1.0f));
		}
		else
		{
			input->Get_Joystick_State()->Set_Vibration(0.0f, 0.0f);
		}
	}
}

void Game::Activate_Demo(Demo* demo)
{
	DemoMap& map = demo->Maps[0];

	MapFileHandle* handle = ResourceFactory::Get()->Get_Map_File_By_GUID(map.GUID.c_str());
	DBG_ASSERT(handle != NULL);

	// Create offline lobby.
	OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
	settings.MapGUID = handle->Get()->Get_Header()->GUID;
	settings.MapWorkshopID = map.Workshop_ID;
	settings.MaxPlayers = 1;
	settings.Private = true;
	settings.Secure = false;
	settings.IsOffline = true;
	OnlineMatching::Get()->Set_Lobby_Settings(settings);
	OnlineMatching::Get()->Create_Lobby(settings);

	DBG_ASSERT(OnlineMatching::Get()->Get_State() == OnlineMatching_State::In_Lobby);

	// Start local server.
	GameEngine::Get()->Get_UIManager()->Go(UIAction::Push(new UIScene_StartingLocalServer(false), new UIFadeInTransition()));

	DemoManager::Get()->Play(demo);
	m_active_demo = demo;
	m_starting_demo = true;
}

void Game::Update_Demo()
{
	// Starting custom map?
	UIManager* ui_manager = GameEngine::Get()->Get_UIManager();
	if (m_starting_demo)
	{
		if (ui_manager->Get_Scene_By_Type<UIScene_StartingLocalServer*>() == NULL && !ui_manager->Transition_In_Progress() && !ui_manager->Pending_Actions())
		{
			m_starting_demo = false;

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
				SAFE_DELETE(m_active_demo);
			}
		}
	}
	// In a custom map?
	else if (m_active_demo != NULL)
	{
		// Server changed? No longer in challenge map.
		if (ui_manager->Get_Scene_By_Type<UIScene_StartingLocalServer*>() == NULL &&
			ui_manager->Get_Scene_By_Type<UIScene_ConnectToServer*>() == NULL &&
			ui_manager->Get_Scene_By_Type<UIScene_Game*>() == NULL &&
			ui_manager->Get_Scene_By_Type<UIScene_MapLoading*>() == NULL &&
			!ui_manager->Transition_In_Progress() && !ui_manager->Pending_Actions())
		{
			DBG_LOG("User is not in game, assuming they have left demo map.");
			SAFE_DELETE(m_active_demo);
		}
	}
}

Demo* Game::Get_Active_Demo()
{
	return m_active_demo;
}