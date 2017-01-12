// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Engine/GameEngine.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Audio/AudioRenderer.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Localise/Locale.h"
#include "Engine/Input/Input.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineMatching.h"
#include "Engine/Online/OnlineRankings.h"
#include "Engine/Online/OnlineMods.h"
#include "Engine/Online/OnlineAchievements.h"
#include "Engine/Online/OnlineGlobalChat.h"
#include "Engine/Online/OnlineVoiceChat.h"
#include "Engine/Online/OnlineInventory.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetServer.h"

#include "Generic/Math/Math.h"

#ifdef OPT_STEAM_PLATFORM
#include "Engine/Online/Steamworks/Steamworks_OnlinePlatform.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineMatching.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineClient.h"
#include "Engine/Online/Steamworks/Steamworks_OnlineVoiceChat.h"
#endif

#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Storage/PersistentStorage.h"

#include "Engine/GeoIP/GeoIPManager.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Resources/Reloadable.h"

#include "Engine/UI/Layouts/UILayoutFactory.h"

#include "Engine/Tasks/TaskManager.h"

#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Drawable.h"

#include "Engine/Particles/ParticleFXFactory.h"
#include "Engine/Audio/Sounds/SoundFactory.h"

#include "Engine/Options/OptionRegistry.h"
#include "Engine/Options/OptionCommandLineParser.h"

#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameRunner.h"
#include "Engine/Engine/FrameStats.h"
#include "Engine/Profiling/ProfilingManager.h"
#include "Engine/Network/NetManager.h"

#include "Generic/Stats/Stats.h"
#include "Generic/Types/PooledMemoryHeap.h"

#include "Generic/Types/UTF8String.h"

#include "Generic/Stats/Stats.h"

#include "Engine/Video/VideoRecorder.h"

#include <iostream>
#include <ctime>

DEFINE_RANGE_STATISTIC	("Memory/RAM Usage (MB)",				float,					g_stat_ram_usage,				false);
DEFINE_AVERAGE_STATISTIC("Memory/RAM Alloc Per Frame (KB)",		float,			60,		g_stat_ram_alloc_per_frame,		false);

DEFINE_FRAME_STATISTIC("Engine Tick/Deferred Stall Time", float, g_defered_tasks_stall_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Input Time", float, g_input_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Display Time", float, g_display_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Audio Time", float, g_audio_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/UI Time", float, g_ui_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Matching Time", float, g_matching_time, false);
DEFINE_FRAME_STATISTIC("Engine Tick/Ranking Time", float, g_ranking_time, false);
DEFINE_FRAME_STATISTIC("Engine Tick/Achievement Time", float, g_achievement_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Voice Chat Time", float, g_voice_chat_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Global Chat Time", float, g_global_chat_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Online Platform Time", float, g_online_platform_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Net Manager Time", float, g_net_manager_time, true);
DEFINE_FRAME_STATISTIC("Engine Tick/Resource Factory Time", float, g_resource_factory_time, false);
DEFINE_FRAME_STATISTIC("Engine Tick/Game Runner Time", float, g_game_runner_time, false);

int game_get_frame_index()
{
	GameEngine* engine = GameEngine::Try_Get();
	if (engine != NULL)
	{
		return engine->Get_Time()->Get_Frame();
	}
	else
	{
		return 0;
	}
}

int game_get_gc_size()
{
	GameEngine* engine = GameEngine::Try_Get();
	if (engine != NULL)
	{
		CVirtualMachine* vm = engine->Get_VM();
		if (vm != NULL)
		{
			return vm->Get_GC()->Get_Total_Used();
		}
	}

	return 0;
}

GameEngine::GameEngine(GameRunner* runner, const char* command_line)
	: m_is_running(true)
	, m_runner(runner)
	, m_frame_time(0)
	, m_render_pipeline(NULL)
	, m_map_bgm_pushed(false)
	, m_frame_deferred_taskid(-1)
	, m_group_task("Frame Deferred Task Group")
	, m_display_profiler(false)
	, m_online_platform(NULL)
	, m_online_matching(NULL)
	, m_online_global_chat(NULL)
	, m_net_manager(NULL)
	, m_online_voice_chat(NULL)
	, m_resource_factory(NULL)
	, m_online_achievements(NULL)
	, m_online_rankings(NULL)
	, m_online_mods(NULL)
	, m_script_tick_base(0.0)
	, m_engine_vm(NULL)
	, m_renderer(NULL)
	, m_platform(NULL)
	, m_audio_renderer(NULL)
	, m_display(NULL)
	, m_scene(NULL)
	, m_ui_manager(NULL)
	, m_locale(NULL)
	, m_task_manager(NULL)
	, m_persistent_storage(NULL)
	, m_video_recorder(NULL)
	, m_run_in_fixed_step(false)
	, m_shutdown_when_empty(false)
{
	m_platform = Platform::Get();

	DBG_LOG("Profiling manager.");
	ProfilingManager::Init();

	DBG_LOG("Creating options registry.");
	m_options_registry = OptionRegistry::Create();

	DBG_LOG("Registering options to options registry.");
	EngineOptions::Register_Options();

	DBG_LOG("Performing preload.");
	{
		m_runner->Preload();
		m_frame_time = FrameTime(*EngineOptions::render_frame_rate);
	}
		
	DBG_LOG("Parsing command line options.");
	OptionCommandLineParser parser;
	parser.Parse(command_line);

	// If we are running as a server, force no-gui.
	if (*EngineOptions::server == true)
	{
		DBG_LOG("Running as server, forcing nogui mode.")
		*EngineOptions::nogui = true;
	}
	
	DBG_LOG("Instantiating network manager");
	{
		m_net_manager = m_runner->Get_Net_Manager();
		DBG_ASSERT_STR(m_net_manager != NULL, "Could not retrieve game network manager.");
	}	
	
	if (!(*EngineOptions::server))
	{
		DBG_LOG("Initialising network client");
		DBG_ASSERT_STR(m_net_manager->Init_Client(), "Failed to initialize networks client.");
	}
	
	DBG_LOG("Initializing online platform.");
	{
		m_online_platform = OnlinePlatform::Create();

#ifdef OPT_STEAM_PLATFORM
		static_cast<Steamworks_OnlinePlatform*>(m_online_platform)->Set_AppID(*EngineOptions::steam_appid);
#endif

		if (!(*EngineOptions::server) && m_online_platform->Restart_Required() == true)
		{
			DBG_LOG("Online platform believes a restart is required, fast exiting.");
			exit(0);
		}
		else
		{
#ifdef OPT_STEAM_PLATFORM
			if (*EngineOptions::server)
			{
				DBG_CHECK_STR(m_online_platform->Init_Server(), "Failed to initialize steam as dedicated server, it may not be running.");
			}
			else
			{
				DBG_CHECK_STR(m_online_platform->Init_Client(), "Failed to initialize and connect to steam, are you sure it's running?");
			}
#else
			if (*EngineOptions::server)
			{
				DBG_CHECK_STR(m_online_platform->Init_Server(), "Failed to initialize online platform as dedicated server, it may not be running.");
			}
			else
			{
				DBG_CHECK_STR(m_online_platform->Init_Client(), "Failed to initialize online platform as client, it may not be running.");
			}
#endif
		}
	}

	DBG_LOG("Initializing online matching.");
	{
		m_online_matching = OnlineMatching::Create();
		DBG_ASSERT_STR(m_online_matching->Initialize(), "Failed to initialize online matching.")
	}
	if (!(*EngineOptions::server))
	{
		DBG_LOG("Initializing online achievements.");
		{
			m_online_achievements = OnlineAchievements::Create();
			DBG_ASSERT_STR(m_online_achievements->Initialize(), "Failed to initialize online achievements.")
		}

		DBG_LOG("Initializing online global chat.");
		{
			m_online_global_chat = OnlineGlobalChat::Create();
			DBG_ASSERT_STR(m_online_global_chat->Initialize(), "Failed to initialize online global chat.")
		}

		DBG_LOG("Initializing online rankings.");
		{
			m_online_rankings = OnlineRankings::Create();
			DBG_ASSERT_STR(m_online_rankings->Initialize(), "Failed to initialize online rankings.")
		}

		DBG_LOG("Initializing online mods.");
		{
			m_online_mods = OnlineMods::Create();
			DBG_ASSERT_STR(m_online_mods->Initialize(), "Failed to initialize online mods.")
		}

		DBG_LOG("Initializing online inventory.");
		{
			m_online_inventory = OnlineInventory::Create();
			DBG_ASSERT_STR(m_online_inventory->Initialize(), "Failed to initialize online inventory.")
		}
	}
	DBG_LOG("Initializing online voice chat.");
	{
		m_online_voice_chat = OnlineVoiceChat::Create();
		DBG_ASSERT_STR(m_online_voice_chat->Initialize(), "Failed to initialize online voice chat.")
	}	
	
	DBG_LOG("Initialising persistent storage.");
	{
		m_persistent_storage = PersistentStorage::Create();
		DBG_ASSERT_STR(m_persistent_storage != NULL, "Could not instantiate persistent storage singleton.");
	}
	
	DBG_LOG("Loading user configuration.");
	{
		m_runner->Load_User_Config();
		m_frame_time = FrameTime(*EngineOptions::render_frame_rate);
	}
	
	DBG_LOG("Initialising renderer.");
	{
		m_renderer = Renderer::Create();
		DBG_ASSERT_STR(m_renderer != NULL, "Could not instantiate renderer singleton.");
	}

	DBG_LOG("Initialising video recorder.");
	{
		m_video_recorder = new VideoRecorder();
	}

	DBG_LOG("Initialising display.");
	{
		m_display = GfxDisplay::Create((*EngineOptions::render_display_title).c_str(), *EngineOptions::render_display_width, *EngineOptions::render_display_height, *EngineOptions::render_display_hertz, (GfxDisplayMode::Type)(*EngineOptions::render_display_mode));
		DBG_ASSERT_STR(m_display != NULL, "Could not instantiate display window.");
	}

	DBG_LOG("Binding display to renderer.");
	{
		bool result = m_renderer->Set_Display(m_display);
		DBG_ASSERT_STR(result, "Could not bind renderer to display.");
	}

	DBG_LOG("Creating engine tag replacer.");
	{
		m_tag_replacer = new EngineTagReplacer();
	}

	DBG_LOG("Initialising resource factory.");
	{
		m_resource_factory = new ResourceFactory((*EngineOptions::data_directory).c_str());
		DBG_ASSERT_STR(m_resource_factory != NULL, "Could not instantiate resource factory.");
	}
	
	DBG_LOG("Initialising audio renderer.");
	{
		m_audio_renderer = AudioRenderer::Create();
		DBG_ASSERT_STR(m_audio_renderer != NULL, "Could not instantiate audio renderer.");
	}

	DBG_LOG("Creating locale handler.");
	{
		m_locale = Locale::Create();
		DBG_ASSERT_STR(m_locale != NULL, "Could not instantiate locale singleton.");
	}

	DBG_LOG("Initialising task manager.");
	{
		// Scale task works by core count. We always have cores - 1 workers for best efficiency (so we use all the cores).
		// Minimum of 3 workers.
		*EngineOptions::tasks_max_workers = Max(3, Platform::Get()->Get_Core_Count() - 1);

		m_task_manager = new TaskManager(*EngineOptions::tasks_max_workers, *EngineOptions::tasks_max_tasks);
		DBG_ASSERT_STR(m_task_manager != NULL, "Could not instantiate task manager.");
	}

	DBG_LOG("Parsing resource packages.");
	{
		bool result = m_resource_factory->Parse_Resources();
		DBG_ASSERT_STR(result, "Could not parse resources.");
	}

	DBG_LOG("Loading initial resources.");
	{
		m_resource_factory->Load_Resources("initial", true);
	}

	DBG_LOG("Loading initial graphics resources.");
	{
		m_resource_factory->Load_Resources("gfx_initial", true, true);
	}

	DBG_LOG("Loading joystick mappings.");
	Input::Load_Joystick_Mapping("Data/Config/Controller_Bindings.xml");

	DBG_LOG("Initialising input.");
	{
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			Input* input = Input::Create(i);
			DBG_ASSERT_STR(input != NULL, "Could not instantiate input singleton.");
		}
	}

	DBG_LOG("Load GeoIP database.");
	{		
		DBG_ASSERT_STR(GeoIPManager::Create() != NULL, "Could not instantiate geoip manager.");

		std::string abs_path = Platform::Get()->Get_Absolute_Path(ResourceFactory::Get()->Get_Root_Directory() + "/GeoIP.dat");

		// WinXP vomits if we don't do this :S.
#ifdef PLATFORM_WIN32
		abs_path = StringHelper::Replace(abs_path.c_str(), "/", "\\");
#endif		

		if (!GeoIPManager::Get()->Load_Database(abs_path.c_str()))
		{
			DBG_ASSERT_STR(false, "Failed to load geoip database from %s.", abs_path.c_str());
		}
	}

	DBG_LOG("Selecting default language: %s", (*EngineOptions::language_default).c_str());
	{
		bool result = m_locale->Change_Language((*EngineOptions::language_default).c_str());
		DBG_ASSERT_STR(result, "Failed to set default language to: %s", (*EngineOptions::language_default).c_str());
	}

	DBG_LOG("Initialising rendering pipeline.");
	{
		m_render_pipeline = new RenderPipeline(m_renderer);
		DBG_ASSERT_STR(m_render_pipeline != NULL, "Could not instantiate rendering pipeline.");
	}

	DBG_LOG("Loading rendering pipeline configuration: %s", (*EngineOptions::render_pipeline_file).c_str());
	{
		bool result = m_render_pipeline->Load_Config((*EngineOptions::render_pipeline_file).c_str());
		DBG_ASSERT_STR(result, "Could not load rendering pipeline configuration.");
	}

	DBG_LOG("Setting up scene.");
	{
		m_scene = m_runner->Get_Scene();
		DBG_ASSERT_STR(m_scene != NULL, "Could not instantiate scene.");
	}

	DBG_LOG("Setting up engine VM.");
	{
		m_engine_vm = m_runner->Create_VM();
		DBG_ASSERT_STR(m_engine_vm != NULL, "Could not instantiate engine vm.");
	}
	
	DBG_LOG("Setting up UI manager.");
	{
		m_ui_manager = new UIManager();
		DBG_ASSERT_STR(m_ui_manager != NULL, "Could not instantiate UI manager.");
	}
}

void GameEngine::Set_Fixed_Step(bool bFixedStep)
{
	m_run_in_fixed_step = bFixedStep;
}

// Apply configurable settings.
void GameEngine::Apply_Config()
{
	// Audio settings.
	m_audio_renderer->Set_BGM_Volume(*EngineOptions::audio_bgm_volume);
	m_audio_renderer->Set_SFX_Volume(*EngineOptions::audio_sfx_volume);

	// Steamworks.
#ifdef OPT_STEAM_PLATFORM
	if (m_online_platform->Client() != NULL)
		static_cast<Steamworks_OnlineClient*>(m_online_platform->Client())->Set_Notification_Corner(*EngineOptions::steam_notification_corner);
#endif

	// Update display.
	bool displayChanged = m_display->Resize(*EngineOptions::render_display_width, *EngineOptions::render_display_height, *EngineOptions::render_display_hertz, (GfxDisplayMode::Type)*EngineOptions::render_display_mode);
	if (displayChanged)
	{
		m_render_pipeline->Display_Mode_Changed();
	}

	// Apply language.	
	Locale::Get()->Change_Language((*EngineOptions::language_default).c_str());

	// Resize based on new display.
	m_ui_manager->Refresh();
}

GameEngine::~GameEngine()
{
	// Bllllllllllllah dependency issues :(

	// Lazy fix for crashes on exit.
	// TODO: Fix properly.
	NetManager::Get()->DeInit();
	NetManager::Destroy();
	OnlinePlatform::Destroy(); // Ensure steam closes down correctly.
	Platform_Exit();

/*

	// Destroy resources.
	UILayoutFactory::Dispose();
	TextureFactory::Dispose();
	SoundFactory::Dispose();
	ParticleFXFactory::Dispose();

	// Destroy singletons.
	NetManager::Destroy();
	GeoIPManager::Destroy();
	TaskManager::Destroy();
	Locale::Destroy();
	ResourceFactory::Destroy();
	RenderPipeline::Destroy();
	AudioRenderer::Destroy();
	Input::Destroy();
	GfxDisplay::Destroy();
	Renderer::Destroy();
	PersistentStorage::Destroy();
	OnlineGlobalChat::Destroy();
	OnlineVoiceChat::Destroy();
	OnlineMods::Destroy();
	OnlineRankings::Destroy();
	OnlineAchievements::Destroy();
	OnlineMatching::Destroy();
	OnlinePlatform::Destroy();
	Platform::Destroy();
	OptionRegistry::Destroy();

*/
}

Scene* GameEngine::Get_Scene()
{
	return m_scene;
}

FrameTime* GameEngine::Get_Time()
{
	return &m_frame_time;
}

UIManager* GameEngine::Get_UIManager()
{
	return m_ui_manager;
}

RenderPipeline* GameEngine::Get_RenderPipeline()
{
	return m_render_pipeline;
}

bool GameEngine::Is_Running()
{
	return m_is_running;
}

EngineVirtualMachine* GameEngine::Get_VM()
{
	return m_engine_vm;
}

Map* GameEngine::Get_Map()
{
	return m_runner->Get_Map();
}

GameRunner* GameEngine::Get_Runner()
{
	return m_runner;
}

TaskID GameEngine::Get_Frame_Deferred_TaskID()
{
	return m_frame_deferred_taskid;
}

bool GameEngine::Is_Stop_When_Empty_Pending()
{
	return m_shutdown_when_empty;
}

void GameEngine::Stop_When_Empty()
{
	m_shutdown_when_empty = true;
}

bool GameEngine::Start_Local_Server(OnlineMatching_LobbySettings* lobby_settings)
{
	m_net_manager->Pause_Connection_Polling();

	DBG_LOG("Initialising local server ...");
	
	if (!m_online_platform->Init_Server())
	{
		DBG_LOG("Failed to intialise online platform server.");
		return false;
	}

	if (!m_net_manager->Init_Server())
	{
		DBG_LOG("Failed to intialise net manager server.");
		return false;
	}
	 
	OnlineMatching_LobbySettings settings = OnlineMatching::Get()->Get_Lobby_Settings();
	m_net_manager->Server()->Reserve(OnlineMatching::Get()->Get_Lobby_ID(), &settings);

	m_net_manager->Resume_Connection_Polling();

	DBG_LOG("Initialised local server.");
	return true;
}

bool GameEngine::Stop_Local_Server()
{
	m_net_manager->Pause_Connection_Polling();
	
	DBG_LOG("Destroying local server ...");

	if (!m_net_manager->Destroy_Server())
	{
		DBG_LOG("Failed to destroy net manager server.");
		return false;
	}
	else
	{
		DBG_LOG("Destroyed network server.");
	}

	if (!m_online_platform->Destroy_Server())
	{
		DBG_LOG("Failed to destroy online platform server.");
		return false;
	}
	else
	{
		DBG_LOG("Destroyed online server.");
	}
	
	m_net_manager->Resume_Connection_Polling();

	return true;
}

void GameEngine::Stop()
{
	m_is_running = false;

	DBG_LOG("Waiting for tasks to complete...");
	m_task_manager->Wait_For_All();
}

void GameEngine::Run()
{
	m_runner->Start();

	while (Is_Running())
	{
#ifndef MASTER_BUILD
		Do_Debug_Controls();
#endif

		ProfilingManager::New_Frame();

		{
			PROFILE_SCOPE_BUDGETED("Main Loop", 16.0f);
			Main_Loop();
		}		
	}	

	m_runner->End();
}	

double GameEngine::Get_Script_Tick_Base()
{
	return m_script_tick_base;
}

void GameEngine::Do_Debug_Controls()
{
	//F1=In-Game Profiler

	//F5=Pause
	//F6=Step Frame

	//F9=Dump Memory
	//F10=Toggle Profile

	// Dump memory usage?
	if (Input::Get(0)->Get_Keyboard_State()->Was_Key_Pressed(InputBindings::Keyboard_F9))
	{
		MemoryManager::Dump_Stats();
	}

	// Toggle profiling?
	if (Input::Get(0)->Get_Keyboard_State()->Was_Key_Pressed(InputBindings::Keyboard_F10))
	{
		if (ProfilingManager::Is_Running())
		{
			ProfilingManager::Stop();
			ProfilingManager::Dump_Output();
		}
		else
		{
			ProfilingManager::Start();
		}
	}

	// Step through frames.
	if (Input::Get(0)->Get_Keyboard_State()->Was_Key_Pressed(InputBindings::Keyboard_F5))
	{
		if (m_frame_time.Get_Paused())
		{
			DBG_LOG("Resumed delta time.");
		}
		else
		{
			DBG_LOG("Paused delta time.");
		}
		
		m_frame_time.Set_Paused(!m_frame_time.Get_Paused());		
	}
	if (Input::Get(0)->Get_Keyboard_State()->Was_Key_Pressed(InputBindings::Keyboard_F6))
	{
		m_frame_time.Single_Step();
	}

	/*
	// Turn video on/off.
	if (Input::Get(0)->Get_Keyboard_State()->Was_Key_Pressed(InputBindings::Keyboard_Insert))
	{
		if (m_video_recorder->IsActive())
		{
			m_video_recorder->End();
		}
		else
		{  
			time_t rawtime;
			struct tm * timeinfo;
			char date_time[80];

			time(&rawtime);
			timeinfo = localtime(&rawtime);

			strftime(date_time, 80, "%d-%m-%Y %I-%M-%S", timeinfo);

			std::string directory = Platform::Get()->Get_Absolute_Path(StringHelper::Format("E:\\RECORDED_VIDEOS\\%s\\", date_time).c_str()).c_str();
			m_video_recorder->Start(directory.c_str(), StringHelper::Format("%s\\video.avi", directory.c_str()).c_str());
		}
	}


	*/

	/*
	// Capture screenshots
	static bool bCapturingScreenshots = false;
	static int ScreenshotIndex = 0;
	if (Input::Get(0)->Get_Keyboard_State()->Was_Key_Pressed(InputBindings::Keyboard_Delete))
	{
		bCapturingScreenshots = !bCapturingScreenshots;
		Platform::Get()->Create_Directory("Screenshots", false);
	}

	if ((m_frame_time.Get_Frame() % 20) == 0 && bCapturingScreenshots)
	{
		Pixelmap* pix = Renderer::Get()->Capture_Backbuffer();
		std::string filename = StringHelper::Format("Screenshots\\%i.png", ScreenshotIndex++);
		PixelmapFactory::Save(filename.c_str(), pix);
		SAFE_DELETE(pix);
	}*/
}

void GameEngine::Main_Loop()
{
	/*
	static int last_mem = 0;
	int used = platform_get_used_memory() - last_mem;
	last_mem = platform_get_used_memory();

	DBG_LOG("Memory Delta: %i", used);
	*/

#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Print_Output();
	FrameStats::New_Tick();
#endif

	// If we are running as a dedicated server and nobody
	// is connected to us, might as well sleep for a bit, no point eating the CPU.
	if ((*EngineOptions::server) && NetManager::Get()->Get_Connection_Count() <= 1)
	{
		Platform::Get()->Sleep(100);

		// Shutdown.
		if (m_shutdown_when_empty)
		{
			Stop();
			m_shutdown_when_empty = false;
		}
	}

	m_frame_time.New_Tick(NetManager::Get()->Is_Dedicated_Server(), m_run_in_fixed_step);
	m_frame_time.Begin_Update();			
			
	{
		PROFILE_SCOPE_BUDGETED("Tick", 8.0f);
		Tick(m_frame_time);
	}

	m_frame_time.Finish_Update();

	// Don't bother trying to draw as a dedicated server either.		
	if (*EngineOptions::nogui == false)
	{
		{
			PROFILE_SCOPE_BUDGETED("Render", 8.0f);
			m_render_pipeline->Draw(m_frame_time);
		}

#if !defined(MASTER_BUILD) && defined(PLATFORM_WIN32)
		static float title_update_timer = 0;
		title_update_timer += m_frame_time.Get_Delta_Seconds();

		if (title_update_timer >= 0.3f)
		{
			// Update the display title.
			char buffer[256];
			sprintf_s(buffer, "%s [fps:%i, ft:%0.2f, ut:%0.2f, dt:%0.2f, mem:%.2f MB]", (*EngineOptions::render_display_title).c_str(), m_frame_time.Get_FPS(), m_frame_time.Get_Frame_Time(), m_frame_time.Get_Update_Time(), m_frame_time.Get_Render_Time(), (platform_get_used_memory() / 1024.0f) / 1024.0f);
			GfxDisplay::Get()->Set_Title(buffer);

			title_update_timer = 0.0f;
		}
#endif
	}
	else
	{
		// As we are a dedicated server we will never get draw events, but we need to ensure all pre-draw
		// functions are run as they are responsible for updating animations etc.

		// Predraw step for all drawables.
		{
			std::vector<IDrawable*>& drawable = GameEngine::Get()->Get_Scene()->Get_Drawables();
			for (std::vector<IDrawable*>::iterator iter = drawable.begin(); iter != drawable.end(); iter++)
			{
				(*iter)->Pre_Draw();
			}
		}
	}

	// Update some stats.
	{
		PROFILE_SCOPE_BUDGETED("Update Statistics", 8.0f);
		Update_Statistics();
	}

	// Fullscreen/Windowed toggle.
	{
		PROFILE_SCOPE_BUDGETED("Update Hotkeys", 8.0f);

		GfxDisplay* display = GfxDisplay::Get();
		Input* input = Input::Get();
		if (input->Is_Down(InputBindings::Keyboard_LeftAlt) || input->Is_Down(InputBindings::Keyboard_RightAlt))
		{
			if (input->Was_Pressed(InputBindings::Keyboard_Enter))
			{
				if (display->Get_Mode() == GfxDisplayMode::Fullscreen)
				{
					DBG_LOG("Fullscreen toggle shortcut recognised. Going windowed!");
					display->Resize(display->Get_Width(), display->Get_Height(), display->Get_Hertz(), GfxDisplayMode::Windowed);
				}
				else
				{
					DBG_LOG("Fullscreen toggle shortcut recognised. Going fullscreen!");
					display->Resize(display->Get_Width(), display->Get_Height(), display->Get_Hertz(), GfxDisplayMode::Fullscreen);
				}
			}
		}
	}
}

void GameEngine::Update_Statistics()
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	// Memory allocation.
	static int m_last_global_allocation = 0;
	int global_allocs = platform_get_malloc_heap()->Get_Lifetime_Bytes_Allocated();

	g_stat_ram_usage.Set((float)platform_get_used_memory() / 1024.0f / 1024.0f, platform_get_malloc_heap()->Get_Total() / 1024.0f / 1024.0f);
	g_stat_ram_alloc_per_frame.Set((float)(global_allocs - m_last_global_allocation) / 1024.0f);

	m_last_global_allocation = global_allocs;
#endif

	StatManager::New_Frame();
}

void GameEngine::Tick(const FrameTime& time)
{
	//DBG_LOG("Waiting for task");
	// Finish defered tasks from last frame.
	if (m_frame_deferred_taskid >= 0)
	{
		STATISTICS_TIMED_SCOPE(g_defered_tasks_stall_time);
		m_task_manager->Wait_For(m_frame_deferred_taskid);
	}

	//DBG_LOG("New task");
	// New group for deferred tasks.
	m_frame_deferred_taskid = m_task_manager->Add_Task(&m_group_task);

	// Check for resource reloads.
#ifndef MASTER_BUILD
	{
		PROFILE_SCOPE_BUDGETED("Reloadables", 1.0f);
		Reloadable::Check_For_Reloads();
	}
#endif

	// Tick all the main elements.
	{
		PROFILE_SCOPE_BUDGETED("Input", 1.0f);
		STATISTICS_TIMED_SCOPE(g_input_time);

		Input::Global_Tick(m_frame_time);

		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			Input::Get(i)->Tick(m_frame_time);
		}
	}
	{
		PROFILE_SCOPE_BUDGETED("Display", 1.0f);
		STATISTICS_TIMED_SCOPE(g_display_time);

		m_display->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Audio", 1.0f);
		STATISTICS_TIMED_SCOPE(g_audio_time);

		m_audio_renderer->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("UI", 1.0f);
		STATISTICS_TIMED_SCOPE(g_ui_time);

		m_ui_manager->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Online Matching", 1.0f);
		STATISTICS_TIMED_SCOPE(g_matching_time);

		m_online_matching->Tick(time);
	}
	if (!(*EngineOptions::server))
	{
		{
			PROFILE_SCOPE_BUDGETED("Online Rankings", 1.0f);
			STATISTICS_TIMED_SCOPE(g_ranking_time);

			m_online_rankings->Tick(time);
		}
		{
			PROFILE_SCOPE_BUDGETED("Online Achievements", 1.0f);
			STATISTICS_TIMED_SCOPE(g_achievement_time);

			m_online_achievements->Tick(time);
		}
		{
			PROFILE_SCOPE_BUDGETED("Global Chat", 1.0f);
			STATISTICS_TIMED_SCOPE(g_global_chat_time);

			m_online_global_chat->Tick(time);
		}
		{
			PROFILE_SCOPE_BUDGETED("Online Inventory", 1.0f);
			STATISTICS_TIMED_SCOPE(g_global_chat_time);

			m_online_inventory->Tick(time);
		}
	}
	{
		PROFILE_SCOPE_BUDGETED("Voice Chat", 1.0f);
		STATISTICS_TIMED_SCOPE(g_voice_chat_time);

		m_online_voice_chat->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Online Platform", 1.0f);
		STATISTICS_TIMED_SCOPE(g_online_platform_time);

		m_online_platform->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Net Manager", 1.0f);
		STATISTICS_TIMED_SCOPE(g_net_manager_time);

		m_net_manager->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Video Recorder", 1.0f);
		m_video_recorder->Tick(time);
	}
	{
		PROFILE_SCOPE_BUDGETED("Resource Factory", 1.0f);
		STATISTICS_TIMED_SCOPE(g_resource_factory_time);
		
		m_resource_factory->Pump();
	}

	// Tick game runner.
	{
		PROFILE_SCOPE_BUDGETED("Game Runner", 8.0f);
		STATISTICS_TIMED_SCOPE(g_game_runner_time);

		m_runner->Tick(time);
	}
	
	//DBG_LOG("Queueing deferred task");
	// Get running deferred tasks.
	m_task_manager->Queue_Task(m_frame_deferred_taskid);
}

void GameEngine::Restart_Map()
{
	m_script_tick_base = Platform::Get()->Get_Ticks();
}

void GameEngine::Push_Map_BGM(SoundHandle* m_handle)
{
	if (m_map_bgm_pushed)
	{
		Pop_Map_BGM();
	}

	m_map_bgm_pushed = true;
	AudioRenderer::Get()->Push_BGM(m_handle);
}

void GameEngine::Pop_Map_BGM()
{
	if (m_map_bgm_pushed == true)
	{
		m_map_bgm_pushed = false;
		AudioRenderer::Get()->Pop_BGM();
	}
}
