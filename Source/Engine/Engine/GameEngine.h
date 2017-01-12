// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMEENGINE_
#define _ENGINE_GAMEENGINE_

#include "Engine/Engine/FrameTime.h"
#include "Engine/UI/UIManager.h"
#include "Generic/Patterns/Singleton.h"

#include "Engine/Engine/EngineTagReplacer.h"
#include "Engine/Tasks/TaskManager.h"
#include "Engine/Tasks/GroupTask.h"

class Renderer;
class AudioRenderer;
class Platform;
class GfxDisplay;
class Scene;
class Map;
class GameRunner;
class RenderPipeline;
class Locale;
class TaskManager;
class Input;
class ResourceFactory;
class OnlinePlatform;
class OnlineMatching;
class OnlineAchievements;
class OnlineGlobalChat;
class OnlineRankings;
class OnlineInventory;
class OnlineMods;
class PersistentStorage;
class OptionRegistry;
class NetManager;
class OnlineMatching_LobbySettings;
class OnlineVoiceChat;
class EngineVirtualMachine;
class VideoRecorder;
struct Demo;

class GameEngine : public Singleton<GameEngine>
{
	MEMORY_ALLOCATOR(GameEngine, "Engine");

private:
	bool							m_is_running;
	FrameTime						m_frame_time;

	GameRunner*						m_runner;

	Platform*						m_platform;
	Renderer*						m_renderer;
	AudioRenderer*					m_audio_renderer;
	GfxDisplay*						m_display;
	Scene*							m_scene;
	UIManager*						m_ui_manager;
	Locale*							m_locale;
	TaskManager*					m_task_manager;
	PersistentStorage*				m_persistent_storage;
	OnlinePlatform*					m_online_platform;
	OnlineMatching*					m_online_matching;
	OnlineRankings*					m_online_rankings;
	OnlineMods*						m_online_mods;
	OnlineAchievements*				m_online_achievements;
	OnlineGlobalChat*				m_online_global_chat;
	NetManager*						m_net_manager;
	OnlineVoiceChat*				m_online_voice_chat;
	OnlineInventory*				m_online_inventory;

	ResourceFactory*				m_resource_factory;
	RenderPipeline*					m_render_pipeline;

	EngineVirtualMachine*			m_engine_vm;

	OptionRegistry*					m_options_registry;

	EngineTagReplacer*				m_tag_replacer;

	bool							m_map_bgm_pushed;

	TaskID							m_frame_deferred_taskid;
	GroupTask						m_group_task;

	bool							m_display_profiler;

	double							m_script_tick_base;

	VideoRecorder*					m_video_recorder;

	bool							m_run_in_fixed_step;

	bool							m_shutdown_when_empty;

protected:
	void Do_Debug_Controls();
	void Main_Loop();
	void Update_Statistics();

public:
	~GameEngine();
	GameEngine(GameRunner* runner, const char* command_line);

	Scene*							Get_Scene();
	RenderPipeline*					Get_RenderPipeline();
	UIManager*						Get_UIManager();
	FrameTime*						Get_Time();
	Map*							Get_Map();
	GameRunner*						Get_Runner();

	double Get_Script_Tick_Base();

	TaskID Get_Frame_Deferred_TaskID();

	void Apply_Config();

	void Set_Fixed_Step(bool bFixedStep);

	bool Start_Local_Server(OnlineMatching_LobbySettings* lobby_settings);
	bool Stop_Local_Server();

	void Push_Map_BGM(SoundHandle* m_handle);
	void Pop_Map_BGM();

	void Restart_Map();

	bool Is_Running();
	void Stop();
	void Run();

	bool Is_Stop_When_Empty_Pending();
	void Stop_When_Empty();

	EngineVirtualMachine* Get_VM();

	void Tick(const FrameTime& time);

};

#endif

