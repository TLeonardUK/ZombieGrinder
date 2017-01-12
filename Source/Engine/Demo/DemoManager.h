// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_DEMO_DEMOMANAGER_
#define _ENGINE_DEMO_DEMOMANAGER_

#include "Generic/Patterns/Singleton.h"
#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/DataBuffer.h"

#include "Engine/Tasks/TaskManager.h"

#include <vector>
#include <string>

class Actor;

struct DemoVersion
{
	enum Type
	{
		Base_Version = 1,

		Current,
	};
};

struct DemoEventType
{
	enum Type
	{
	};
};

struct DemoEvent
{

};

struct DemoFrame
{
	double		Time;
	int			Map_Index;
	DataBuffer	World_State;
	DataBuffer	Raw_World_State;
	bool		Keyframe;
};

struct DemoMap
{
	std::string GUID;
	int			Seed;	
	int			Dungeon_Level;
	int			Load_Index;
	u64			Workshop_ID;
};

struct DemoPlayer
{
	std::string	Username;
	u64			Platform_ID;
	Actor*		Active_Actor; // Only valid when calling get_active_players
	int			Frames_Since_Last_Seen;
};

struct Demo
{
public:
	DemoVersion::Type		Version;
	std::vector<DemoFrame*> Frames;
	std::vector<DemoMap>	Maps;
	std::vector<DemoPlayer> Players;

private:
	enum
	{
		Signature = 0x1337C0DE,
	};

public:
	~Demo();
	void Clear();
	bool Serialize(DataBuffer& Buffer, bool bSaving);
	
};

struct DemoManagerState
{
	enum Type
	{
		Recording,
		Playing
	};
};


class DemoDeferredFrameTask : public Task
{
public:
	void Run();
};

class DemoManager : public Singleton<DemoManager>
{
	MEMORY_ALLOCATOR(DemoManager, "Network");

private:

public:
	enum
	{
		Demo_Frame_Rate			= 20,
		Demo_Keyframe_Interval	= Demo_Frame_Rate * 2, // Every 2 second.
		Frame_Reserve_Size		= 256 * 1024,
		Max_Demo_Size			= 30 * 1024 * 1024, // With an average bandwidth of 0.01mb/s, this gives us about a half hour of recording.
		Recent_Players_Frame_Limit	= Demo_Frame_Rate * 60 * 15 // Any players seen in the last 15 minutes are classified as recent.
	};

	DemoManagerState::Type m_state;

	int		m_demo_frame;
	Demo	m_demo;
	float	m_demo_frame_timer;

	Demo*		m_playback_demo;
	DataBuffer	m_playback_frame_data;
	DataBuffer	m_playback_delta_reference_data;

	bool		m_can_record;

	bool		m_paused;

	bool		m_deferred_frame_exists;
	DataBuffer	m_deferred_frame_data;
	DemoFrame*	m_deferred_frame;

	TaskID					m_deferred_frame_task_id;
	DemoDeferredFrameTask*	m_deferred_frame_task;

	bool m_pause_on_next_frame;

	bool m_scrubbing;

	bool m_initial_map_load_pending;

private:
	void Record_Frame();
	void Play_Frame(bool bForceInterp = false);
	void Interpolate_Frame(float frame_delta);

	void Defer_Recorded_Frame(DataBuffer& buffer, DemoFrame* frame);
	void Store_Deferred_Frame();

public:
	DemoManager();
	virtual ~DemoManager();

	// Connects to online platform.
	bool Init();

	// Tick tock tick tock
	void Tick(const FrameTime& time);

	// Current state.
	void Set_Can_Record(bool bCanRecord);
	bool Is_Playing();
	bool Is_Paused();
	void Toggle_Pause();
	void Restart();
	int Get_Current_Time(); 
	int Get_Total_Time();
	void Scrub(int frame_delta);

	// Player management
	std::vector<DemoPlayer> Get_Recent_Players();

	std::vector<DemoPlayer> Get_Active_Players();
	bool Get_Player_By_Player_Index(int id, DemoPlayer& player);
	int Net_ID_To_Player_Index(int id);

	// Plays back a previously recorded demo.
	void Play(Demo* demo);

	// Gets a playable demo from the last amount of time. Recording is constant.
	Demo* Get_Demo(float max_time);

};

#endif

