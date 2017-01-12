// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MAINGAMERUNNER_
#define _GAME_MAINGAMERUNNER_

#include "Generic/Patterns/Singleton.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Renderer/Canvas.h"

#include "Game/Scene/Map/Blocks/Factories.h"

#include "Engine/Online/OnlineInventory.h"

#include "Engine/Tasks/GroupTask.h"
#include "Engine/Tasks/TaskManager.h"

#include "Generic/Stats/Stats.h"

#include "Generic/Types/CircleBuffer.h"

class Camera;
class Map;
class CompositeActor;
class GameTagReplacer;
class GameNetManager;
class GameMode;
class ChatManager;
class EditorManager;
class VoteManager;
class ActorReplicator;
class ProfileManager;
class AchievementManager;
class ChallengeManager;
class TutorialManager;
class DLCManager;
struct RankingsManager;
struct StatisticsManager;
class DemoManager;
class ItemManager;
class SkillManager;
class GameScene;
class CollisionManager;
class PathManager;
class ParticleManager;
class BoidManager;
class GamePostProcessManager;
class ScriptEventListener;
class AdminServer;
class MonitorServer;
struct CVMLinkedSymbol;
class PackageFile;

struct CameraID
{
	enum Type
	{
		Game1 = 0,		// Game1 is default camera, 2,3&4 are used for split-screen support.
		Game2,			
		Game3,
		Game4,
		UI,
		Editor_Main,
		Editor_Tileset,
		Editor_Sub,
		COUNT
	};
};

// ------------------------------------------------------------------------------

// Required function to integrate with editor.
void		Print_Game_Version();
//GameRunner* New_Game();
void		Game_Entry_Point(const char* command_line);

class ScriptTickTask : public Task
{
public:
	int Task_Index;

	void Run();
};

struct GameStatValue
{
	enum
	{
		max_history = 200,
	};

	std::string name;
	CircleBuffer<float, max_history> previous_values;
	float target;
	Color color;
	bool lower_is_better;
	bool show_on_graph;
};

struct GameVibration
{
	Vector3 center;
	float	intensity;
	float	duration;
	float	elapsed;
};

class Game : public GameRunner, public Singleton<Game>
{
	MEMORY_ALLOCATOR(Game, "Game");

private:
	enum 
	{
		max_script_tasks = 8
	};

	std::vector<GameVibration>		m_vibrations;

	const char*						m_config_location;
	const char*						m_user_config_location;	

	Camera*							m_cameras[CameraID::COUNT];
	Map*							m_map;

	GameTagReplacer*				m_tag_replacer;

	GameNetManager*					m_net_manager;
	ChatManager*					m_chat_manager;
	VoteManager*					m_vote_manager;
	ActorReplicator*				m_actor_replicator;
	EditorManager*					m_editor_manager;
	ProfileManager*					m_profile_manager;
	AchievementManager*				m_achievement_manager;
	ChallengeManager*				m_challenge_manager;
	TutorialManager*				m_tutorial_manager;
	DLCManager*						m_dlc_manager;
	RankingsManager*				m_rankings_manager;
	StatisticsManager*				m_statistics_manager;
	ItemManager*					m_item_manager;
	SkillManager*					m_skill_manager;
	BoidManager*					m_boid_manager;
	CollisionManager*				m_collision_manager;
	PathManager*					m_path_manager;
	ParticleManager*				m_particle_manager;
	GamePostProcessManager*			m_post_process_manager;

	AdminServer*					m_admin_server;
	MonitorServer*					m_monitor_server;

	GameMode*						m_game_mode;
	
	MapBlockFactories				m_map_block_factories;

	GameScene*						m_scene;
	
	Canvas							m_canvas;
	CVMGCRoot						m_canvas_instance;

	bool							m_has_uploaded_to_workshop;

	bool							m_game_update_enabled;
	bool							m_update_actors;
	bool							m_sub_menu_open;
	bool							m_loading;

	int								m_last_accepted_invite_index;
	bool							m_invite_login_shown;

#ifdef VM_ALLOW_MULTITHREADING
	GroupTask						m_script_group_task;
	int								m_script_task_count;
	ScriptTickTask					m_script_tasks[max_script_tasks];
	std::vector<ScriptEventListener*> m_script_task_listeners;
#endif

	bool							m_save_queued;

	bool							m_crashed;

	CVMLinkedSymbol*				m_event_particle_class;

	bool							m_game_input_enabled;

	HashTable<GameStatValue, int>	m_game_stats;

	double							m_next_server_stats_print_time;

	std::vector<OnlineInventoryItem>	m_inventory_drops;

	std::vector<PackageFile*>			m_restricted_mode_packages;
	std::vector<PackageFile*>			m_server_enforced_packages;
	std::vector<std::string>			m_mod_greylist_patterns;
	std::vector<std::string>			m_mod_blacklist_patterns;
	std::vector<std::string>			m_mod_enforced_patterns;
	bool								m_restricted_mode;
	bool								m_map_restricted_mode;

	float								m_save_icon_timer;
	float								m_save_icon_alpha;
	float								m_save_icon_sin_bob_timer;

	float								m_players_idle_timer;

	float								m_uploading_icon_alpha;
	float								m_uploading_icon_rotation;

	Demo*								m_active_demo;
	bool								m_starting_demo;

	enum
	{
		SAVE_ICON_MIN_DISPLAY_TIME = 3,
		IDLE_TIMER_MAX = 2 * 60,
	};

protected:
	friend class ScriptTickTask;

	// Base functions.
	void Preload();
	void Start();
	void End();
	void Tick(const FrameTime& time);
	void Run_Script_Ticks(int task_index);
	void Draw_Overlays(const FrameTime& time);
	void Draw_Autosave(const FrameTime& time);
	void Draw_Uploading_Icon(const FrameTime& time);
	void Draw_Onscreen_Logs(const FrameTime& time);
	void Draw_Watermark(const FrameTime& time);
	void Draw_Stat_Group(float& offset, float& x_offset,  Stat** stats, int stat_count, std::string path, int depth);
	void Draw_Stat(std::string name, float value, float target, Rect2D bounds, Color base_color, bool bLowerIsBetter, bool bShowOnGraph = true, bool bAsBar = false);

	// Creation functions.
	virtual EngineVirtualMachine* Create_VM();

	// Initialisation
	void Setup_Editor();
	void Setup_Cameras();

	// Save/Load
	void Load_User_Config();
	void Save_User_Config();

public:
	
	void Reset_Local_Idle_Timer();
	void Update_Local_Idle_Timer(const FrameTime& time);

	// Modding restriction.
	void Load_Package_Whitelist();
	bool Does_Package_Touch_Blacklisted_Files(PackageFile* file);
	bool Does_Package_Touch_Greylisted_Files(PackageFile* file);
	bool Does_Package_Touch_Enforced_Files(PackageFile* file);

	std::vector<PackageFile*> Get_Restricted_Packages();
	std::vector<PackageFile*> Get_Server_Enforced_Packages();
	bool In_Restricted_Mode();
	bool In_Map_Restricted_Mode();
	void Set_Map_Restricted_Mode(bool bRestricted);

	void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	// Nasty, this should go elsewhere.
	virtual void Particle_Script_Event(ParticleInstance* instance, std::string event_name);
	virtual void Particle_Global_Script_Event(ParticleInstance* instance, std::string event_name);

	// Checks if we crashed last tiem or not.
	bool Has_Crashed();
	bool Has_Uploaded_To_Workshop();
	void Flag_Uploaded_To_Workshop();

	// Save/Load Stuff.	
	void Queue_Save();
	void Finalize_Loading();
	void Finalize_Loading_Main_Thread();
	void Finalize_Map_Load();

	// Ticks all in-game managers.	
	void Tick_In_Game(const FrameTime& time);

	// Config settings.
	NetManager*						Get_Net_Manager();
	ChatManager*					Get_Chat_Manager();
	EditorManager*					Get_Editor_Manager();
	CollisionManager*				Get_Collision_Manager();
	PathManager*					Get_Path_Manager();
	ParticleManager*				Get_Particle_Manager();

	// Apply config changes.
	void							Apply_Config();

	// Cameras.
	Scene*							Get_Scene();
	GameScene*						Get_Game_Scene();
	Camera*							Get_Camera(CameraID::Type camera);
	Map*							Get_Map();
	GameMode*						Get_Game_Mode();
	CVMObjectHandle					Get_Script_Canvas();

	void							Set_Loading(bool enabled);
	void							Set_Update_Game(bool enabled);
	void							Set_Update_Actors(bool enabled);
	void							Set_Sub_Menu_Open(bool enabled);
	bool							Is_Sub_Menu_Open();

	// Game input state.
	void							Set_Game_Input_Enabled(bool enabled);
	bool							Get_Game_Input_Enabled();

	void							Vibrate(Vector3 position, float duration, float intensity);
	void							Tick_Vibrations(const FrameTime& time);

	// Network managment.
	bool							Check_Network_Disconnect();

	// General.
	void							Convert_Legacy_Maps();

	// Items!
	void							Check_Inventory_Drops();
	bool							Get_Next_Inventory_Drop(OnlineInventoryItem& item);

	// Demos
	void							Activate_Demo(Demo* demo);
	void							Update_Demo();
	Demo*							Get_Active_Demo();

	// Constructors.
	Game();

};

#endif

