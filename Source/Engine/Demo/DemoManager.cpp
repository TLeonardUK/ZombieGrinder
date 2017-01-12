// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Demo/DemoManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Actor.h"
#include "Engine/Engine/GameRunner.h"
#include "Engine/Platform/Platform.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/BinaryStream.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetUser.h"

#include "Engine/Particles/ParticleManager.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CSymbol.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/Online/OnlineUser.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include <algorithm>

// Load and play back a previously saved demo file.
//#define OPT_DEMO_DEBUG_PLAYBACK_FILE

// After enough frames are captured a demo will be saved and played back.
//#define OPT_DEMO_DEBUG_PLAYBACK

void Demo::Clear()
{
	for (unsigned int i = 0; i < Frames.size(); i++)
	{
		SAFE_DELETE(Frames[i]);
	}
	Frames.clear();
	Maps.clear();
	Players.clear();
}

Demo::~Demo()
{
	Clear();
}

bool Demo::Serialize(DataBuffer& Buffer, bool bSaving)
{
	BinaryStream stream;
	if (!bSaving)
	{
		stream.Reserve(Buffer.Size());
		stream.WriteBuffer(Buffer.Buffer(), 0, Buffer.Size());
		stream.Seek(0);
	}

	if (!bSaving && stream.Bytes_Remaining() < 4)
	{
		return false;
	}

	int sig = Signature;
	stream.Serialize<int>(bSaving, sig);
	if (!bSaving && sig != Signature)
	{
		return false;
	}

	stream.Serialize<int>(bSaving, reinterpret_cast<int&>(Version));

	unsigned int frameCount = Frames.size();
	unsigned int mapCount = Maps.size();
	unsigned int playerCount = Players.size();

	stream.Serialize<unsigned int>(bSaving, frameCount);
	stream.Serialize<unsigned int>(bSaving, mapCount);
	stream.Serialize<unsigned int>(bSaving, playerCount);

	if (!bSaving)
	{
		Frames.resize(frameCount);
		Maps.resize(mapCount);
		Players.resize(playerCount);
	}

	for (unsigned int i = 0; i < frameCount; i++)
	{
		DemoFrame* frame = Frames[i];
		if (!bSaving)
		{
			Frames[i] = new DemoFrame();
			frame = Frames[i];
		}

		stream.Serialize<double>(bSaving, frame->Time);
		stream.Serialize<int>(bSaving, frame->Map_Index);
		stream.Serialize<bool>(bSaving, frame->Keyframe);
		stream.Serialize<DataBuffer>(bSaving, frame->World_State);		
	}

	for (unsigned int i = 0; i < mapCount; i++)
	{
		DemoMap& map = Maps[i];

		stream.Serialize<std::string>(bSaving, map.GUID);
		stream.Serialize<int>(bSaving, map.Seed);
		stream.Serialize<int>(bSaving, map.Dungeon_Level);
		stream.Serialize<int>(bSaving, map.Load_Index);
		stream.Serialize<u64>(bSaving, map.Workshop_ID);
	}

	for (unsigned int i = 0; i < playerCount; i++)
	{
		DemoPlayer& player = Players[i];

		stream.Serialize<std::string>(bSaving, player.Username);
		stream.Serialize<u64>(bSaving, player.Platform_ID);
	}

	Buffer.Set(stream.Data(), stream.Length());
	return true;
}

void DemoDeferredFrameTask::Run()
{
	DemoManager* manager = DemoManager::Get();

	// compress state.
	double start = Platform::Get()->Get_Ticks();
	manager->m_deferred_frame_data.Compress(manager->m_deferred_frame->World_State, true);
	double elapsed = Platform::Get()->Get_Ticks() - start;
	if (elapsed > 5.0f)
	{
		DBG_LOG("Frame compress took %.2f ms", elapsed);
	}
}

DemoManager::DemoManager()
	: m_demo_frame_timer(0.0f)
	, m_state(DemoManagerState::Recording)
	, m_can_record(false)
	, m_playback_demo(NULL)
	, m_paused(false)
	, m_deferred_frame_exists(false)
	, m_pause_on_next_frame(false)
	, m_scrubbing(false)
	, m_initial_map_load_pending(false)
{
	m_deferred_frame_task = new DemoDeferredFrameTask();
}

DemoManager::~DemoManager()
{
	SAFE_DELETE(m_deferred_frame_task);
}

bool DemoManager::Init()
{
	return true;
}

std::vector<DemoPlayer> DemoManager::Get_Recent_Players()
{
	std::vector<DemoPlayer> players;

	for (unsigned int i = 0; i < m_demo.Players.size(); i++)
	{
		DemoPlayer& player = m_demo.Players[i];
		if (player.Frames_Since_Last_Seen < Recent_Players_Frame_Limit)
		{
			players.push_back(player);
		}
	}

	return players;
}

std::vector<DemoPlayer> DemoManager::Get_Active_Players()
{
	std::vector<Actor*> all_actors = GameEngine::Get()->Get_Scene()->Get_Actors();
	std::vector<DemoPlayer> players;
	std::vector<u64> player_ids;

	for (std::vector<Actor*>::iterator iter = all_actors.begin(); iter != all_actors.end(); iter++)
	{
		Actor* actor = *iter;
		if (actor->Get_Demo_Player().Username != "" &&
			std::find(player_ids.begin(), player_ids.end(), actor->Get_Demo_Player().Platform_ID) == player_ids.end())
		{
			DemoPlayer player = actor->Get_Demo_Player();
			player.Active_Actor = actor;
			
			players.push_back(player);
			player_ids.push_back(actor->Get_Demo_Player().Platform_ID);
		}
	}

	return players;
}

bool DemoManager::Get_Player_By_Player_Index(int id, DemoPlayer& player)
{
	std::vector<DemoPlayer> players = Get_Active_Players();
	if (id >= 0 && id < (int)players.size())
	{
		player = players[id];
		return true;
	}

	return false;
}

int DemoManager::Net_ID_To_Player_Index(int id)
{
	NetUser* user = NetManager::Get()->Get_User_By_Net_ID(id);
	if (user)
	{
		std::vector<DemoPlayer> players = Get_Active_Players();
		for (unsigned int i = 0; i < players.size(); i++)
		{
			if (players[i].Platform_ID == user->Get_Online_User()->Get_User_ID())
			{
				return i;
			}
		}
	}
	
	return -1;
}

bool ScriptedActorSort(Actor* A, Actor* B)
{
	return A->Get_Demo_ID() > B->Get_Demo_ID();
}

void DemoManager::Record_Frame()
{
	PROFILE_SCOPE("Record Frame");

	double start = Platform::Get()->Get_Ticks();

	m_demo.Version = (DemoVersion::Type)((int)DemoVersion::Current - 1);

	// Have we changed map since last time?
	if (m_demo.Maps.size() <= 0 ||
		NetManager::Get()->Get_Current_Map_Load_Index()		!= m_demo.Maps[m_demo.Maps.size() - 1].Load_Index ||
		NetManager::Get()->Get_Current_Map_GUID()			!= m_demo.Maps[m_demo.Maps.size() - 1].GUID ||
		NetManager::Get()->Get_Current_Map_Dungeon_Level()	!= m_demo.Maps[m_demo.Maps.size() - 1].Dungeon_Level ||
		NetManager::Get()->Get_Current_Map_Seed()			!= m_demo.Maps[m_demo.Maps.size() - 1].Seed ||
		NetManager::Get()->Get_Current_Map_Workshop_ID()	!= m_demo.Maps[m_demo.Maps.size() - 1].Workshop_ID)
	{
		DemoMap map;
		map.GUID			= NetManager::Get()->Get_Current_Map_GUID();
		map.Load_Index		= NetManager::Get()->Get_Current_Map_Load_Index();
		map.Dungeon_Level	= NetManager::Get()->Get_Current_Map_Dungeon_Level();
		map.Seed			= NetManager::Get()->Get_Current_Map_Seed();
		map.Workshop_ID		= NetManager::Get()->Get_Current_Map_Workshop_ID();
		m_demo.Maps.push_back(map);
	}

	DemoFrame* frame = new DemoFrame();
	frame->Time = Platform::Get()->Get_Ticks();
	frame->Map_Index = m_demo.Maps.size() - 1;

	// order actors by id.
	std::vector<Actor*> all_actors = GameEngine::Get()->Get_Scene()->Get_Actors();
	std::vector<Actor*> actors;
	actors.reserve(all_actors.size());

	for (std::vector<Actor*>::iterator iter = all_actors.begin(); iter != all_actors.end(); iter++)
	{
		Actor* actor = *iter;
		if (actor->Is_Relevant_To_Demo())
		{
			actors.push_back(actor);
		}
	}

	// nvm, best to keep ordered by lifetime for temporal locality.
	//std::sort(actors.begin(), actors.end(), &ScriptedActorSort);

	// write each actor.
	BinaryStream data_stream;
	data_stream.Reserve(Frame_Reserve_Size);

	double serialize_start = Platform::Get()->Get_Ticks();

	// Write actor demo information.
	data_stream.Write<unsigned int>(actors.size());

	for (unsigned int i = 0; i < m_demo.Players.size(); i++)
	{
		DemoPlayer& player = m_demo.Players[i];
		player.Frames_Since_Last_Seen++;
	}

	{
		PROFILE_SCOPE("Record Actors");

		for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
		{
			Actor* actor = *iter;

			int player_index = -1;
			OnlineUser* user = actor->Get_Demo_Owner();
			if (user)
			{
				bool bFound = false;
				for (unsigned int i = 0; i < m_demo.Players.size(); i++)
				{
					DemoPlayer& player = m_demo.Players[i];
					if (player.Platform_ID == user->Get_User_ID())
					{
						bFound = true;
						player_index = i;
						player.Frames_Since_Last_Seen = 0;
						break;
					}
				}

				if (!bFound)
				{
					DemoPlayer player;
					player.Username = user->Get_Username();
					player.Platform_ID = user->Get_User_ID();
					player.Frames_Since_Last_Seen = 0;
					m_demo.Players.push_back(player);

					player_index = m_demo.Players.size() - 1;
				}
			}

			DBG_ASSERT(actor->Get_Demo_ID() != 0);
			data_stream.Write<int>(actor->Get_Demo_ID());
			data_stream.Write<int>(player_index);
			actor->Serialize_Demo(&data_stream, m_demo.Version, true, 1.0f);
		}
	}

	{
		PROFILE_SCOPE("Record Managers");

		// Write VFX demo information.
		ParticleManager::Get()->Serialize_Demo(&data_stream, m_demo.Version, true, 1.0f, false);

		// Read audio information.
		AudioRenderer::Get()->Serialize_Demo(&data_stream, m_demo.Version, true, 1.0f, false);

		// Read chat information.
		GameEngine::Get()->Get_Runner()->Serialize_Demo(&data_stream, m_demo.Version, true, 1.0f);
	}

	double serialized_elapsed = Platform::Get()->Get_Ticks() - serialize_start;

	frame->Raw_World_State.Set(data_stream.Data(), data_stream.Length());

	DataBuffer result(data_stream.Data(), data_stream.Length());

	// Generate a keyframe every so often so we can trim memory usage arbitrarily.
	frame->Keyframe = (m_demo.Frames.size() <= 0);
	if ((m_demo.Frames.size() % Demo_Keyframe_Interval) == 0)
	{
		frame->Keyframe = true;
	}

	// delta encode with previous frame unless we are a keyframe.
	if (!frame->Keyframe)
	{
		PROFILE_SCOPE("Delta Encode");

		DemoFrame* prev_frame = m_demo.Frames[m_demo.Frames.size() - 1];
		result.Delta_Encode_In_Place(prev_frame->Raw_World_State);
		prev_frame->Raw_World_State.Reserve(0, false);
	}

	Defer_Recorded_Frame(result, frame);

	double elapsed = Platform::Get()->Get_Ticks() - start;

	DBG_ONSCREEN_LOG(StringHelper::Hash("DemoManagerBandwidth2"), Color::Green, 1.0f, "Record Time: %.2f", elapsed);
}

void DemoManager::Defer_Recorded_Frame(DataBuffer& buffer, DemoFrame* frame)
{
	m_deferred_frame_data = buffer;
	m_deferred_frame = frame;
	m_deferred_frame_exists = true;

	// Add task for the next frame.
	m_deferred_frame_task_id = TaskManager::Get()->Add_Task(m_deferred_frame_task, GameEngine::Get()->Get_Frame_Deferred_TaskID());
	TaskManager::Get()->Queue_Task(m_deferred_frame_task_id);
}

void DemoManager::Store_Deferred_Frame()
{
	// Ensure we have finished simulation before drawing debug info!
	TaskManager::Get()->Wait_For(m_deferred_frame_task_id);

	// Trim off data while above max size.
	int total_size = 0;
	while (true)
	{
		total_size = 0;
		for (unsigned int i = 0; i < m_demo.Frames.size(); i++)
		{
			total_size += m_demo.Frames[i]->World_State.Size();
		}

		if (total_size < Max_Demo_Size)
		{
			break;
		}

		// Find first keyframe that we can cull at.
		int keyframe_index = -1;
		for (unsigned int i = 0; i < m_demo.Frames.size(); i++)
		{
			if (m_demo.Frames[i]->Keyframe && i > 0)
			{
				keyframe_index = i;
				break;
			}
		}

		// No good keyframes to cull to, just have to deal with the overflow for the time being.
		if (keyframe_index == -1)
		{
			break;
		}

		// Cull everything up to this keyframe, leave the keyframe as the first frame.
		while (keyframe_index > 0)
		{
			SAFE_DELETE(m_demo.Frames[0]);
			m_demo.Frames.erase(m_demo.Frames.begin());
			keyframe_index--;
		}
	}

	m_demo.Frames.push_back(m_deferred_frame);

	static double s_start = Platform::Get()->Get_Ticks();
	double total_elapsed = (Platform::Get()->Get_Ticks() - s_start) / 1000.0f;
	double bandwidth = total_size / total_elapsed;

	DBG_ONSCREEN_LOG(StringHelper::Hash("DemoManagerBandwidth"), Color::Orange, 1.0f, "[Demo] Frame=%i Size=%i / %i Total=%.2f mb (%.2f mb/s)",
		m_demo.Frames.size(),
		m_deferred_frame->World_State.Size(),
		m_deferred_frame->Raw_World_State.Size(),
		total_size / 1024.0f / 1024.0f,
		bandwidth / 1024.0f / 1024.0f);

	m_deferred_frame_exists = false;
}

void DemoManager::Play_Frame(bool bForceInterp)
{
	DemoFrame* frame = m_playback_demo->Frames[m_demo_frame];
	DemoMap& map = m_playback_demo->Maps[frame->Map_Index];

	DBG_ASSERT_STR(m_demo_frame != 0 || frame->Keyframe, "Initial frame in demo is not a keyframe. Wut.");

	// Don't do anything while loading.
	if (NetManager::Get()->Is_Map_Load_Pending() ||
		!m_can_record)
	{
		return;
	}

	// Do we need to load a map?
	if (m_initial_map_load_pending || 
		NetManager::Get()->Get_Current_Map_Load_Index()		!= map.Load_Index ||
		NetManager::Get()->Get_Current_Map_GUID()			!= map.GUID ||
		NetManager::Get()->Get_Current_Map_Dungeon_Level()	!= map.Dungeon_Level ||
		NetManager::Get()->Get_Current_Map_Seed()			!= map.Seed ||
		NetManager::Get()->Get_Current_Map_Workshop_ID()	!= map.Workshop_ID)
	{
		m_initial_map_load_pending = false;
		DBG_LOG("[Demo] Encountered new map, changing to '%s'.", map.GUID.c_str());
		NetManager::Get()->Change_Map_By_GUID(map.GUID.c_str(), map.Workshop_ID, map.Seed, map.Load_Index, map.Dungeon_Level);
		return;
	}

	// If demo frame > 0 then interpolate with a value of 1 so actors etc can store their last positions for interpolation.
	if (m_demo_frame > 0 && (!m_scrubbing || bForceInterp))
	{
		Interpolate_Frame(1.0f);
	}

	// Create any objects that didn't exist on the last frame.
	m_playback_frame_data.Reserve(0, false);
	frame->World_State.Decompress(m_playback_frame_data);
	if (!frame->Keyframe)
	{		
		m_playback_frame_data.Delta_Decode_In_Place(m_playback_delta_reference_data);
	}

	m_playback_delta_reference_data = m_playback_frame_data;

	if (!m_scrubbing || bForceInterp)
	{
		BinaryStream frame_stream(m_playback_frame_data.Buffer(), m_playback_frame_data.Size());
		std::vector<int> frame_actor_ids;

		//DBG_LOG("[Frame %i] Keyframe=%i", m_demo_frame, frame->Keyframe);

		GameEngine::Get()->Get_Scene()->Build_Demo_ID_Lookup();

		// Read actor information.
		unsigned int actor_count = frame_stream.Read<unsigned int>();
		for (unsigned int i = 0; i < actor_count; i++)
		{
			int demo_id = frame_stream.Read<int>();
			int player_index = frame_stream.Read<int>();

			Actor* actor = GameEngine::Get()->Get_Scene()->Get_Actor_By_Demo_ID(demo_id);
			if (actor == NULL)
			{
			//	DBG_LOG("[Demo] Encountered new actor %i, creating proxy actor.", demo_id);
				actor = GameEngine::Get()->Get_Scene()->Create_Demo_Proxy_Actor(demo_id);
				actor->Set_Created_On_Demo_Frame(m_demo_frame);

				if (player_index >= 0)
				{
					actor->Set_Demo_Player(m_playback_demo->Players[player_index]);
				}

				GameEngine::Get()->Get_Scene()->Build_Demo_ID_Lookup();
			}

			actor->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, 0.0f);

			frame_actor_ids.push_back(demo_id);
		}

		// Read VFX information.
		ParticleManager::Get()->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, 0.0f, m_scrubbing);

		// Read audio information.
		AudioRenderer::Get()->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, 0.0f, m_scrubbing);

		// Read chat information.
		GameEngine::Get()->Get_Runner()->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, 0.0f);

		// Delete any object that no longer exist.
		std::vector<Actor*> actors = GameEngine::Get()->Get_Scene()->Get_Actors();
		for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
		{
			Actor* actor = *iter;
			if (actor->Is_Relevant_To_Demo() && actor->Is_Demo_Proxy())
			{
				if (std::find(frame_actor_ids.begin(), frame_actor_ids.end(), actor->Get_Demo_ID()) == frame_actor_ids.end())
				{
				//	DBG_LOG("[Demo] Actor %i no longer exists, removing proxy actor.", actor->Get_Demo_ID());
					SAFE_DELETE(actor);
				}
			}
		}
	}

	// Advance the frame!
	m_demo_frame++;
}

void DemoManager::Interpolate_Frame(float frame_delta)
{
	// Don't do anything while loading.
	if (NetManager::Get()->Is_Map_Load_Pending() ||
		!m_can_record)
	{
		return;
	}

	BinaryStream frame_stream(m_playback_frame_data.Buffer(), m_playback_frame_data.Size());

	GameEngine::Get()->Get_Scene()->Build_Demo_ID_Lookup();

	// Interpolate actors.
	unsigned int actor_count = frame_stream.Read<unsigned int>();
	for (unsigned int i = 0; i < actor_count; i++)
	{
		int demo_id = frame_stream.Read<int>();
		int player_index = frame_stream.Read<int>();

		Actor* actor = GameEngine::Get()->Get_Scene()->Get_Actor_By_Demo_ID(demo_id);
		if (actor == NULL)
		{
			//DBG_LOG("[Demo] Found demo actor that no proxy actor exists for!!? Unable to interpolate frame.");
			return;
		}

		int diff = abs(m_demo_frame - actor->Get_Created_On_Demo_Frame());
		actor->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, diff <= 1 ? 1.0f : frame_delta);
	}

	// Interpolate VFX.
	ParticleManager::Get()->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, frame_delta, m_scrubbing);

	// Read audio information.
	AudioRenderer::Get()->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, frame_delta, m_scrubbing);

	// Read chat information.
	GameEngine::Get()->Get_Runner()->Serialize_Demo(&frame_stream, m_playback_demo->Version, false, frame_delta);
}

void DemoManager::Tick(const FrameTime& time)
{
	const float frame_interval = 1000.0f / Demo_Frame_Rate;

	switch (m_state)
	{
	case DemoManagerState::Recording:
	{
		if (m_deferred_frame_exists)
		{
			Store_Deferred_Frame();
		}

		m_demo_frame_timer += time.Get_Delta_Seconds() * 1000.0f;
		if (m_demo_frame_timer >= frame_interval)
		{
			if (m_can_record)
			{
				Record_Frame();
			}

#ifdef OPT_DEMO_DEBUG_PLAYBACK_FILE
			Demo* demo = new Demo();

			DataBuffer buffer;

			Stream* s = StreamFactory::Open("debug.zgd", StreamMode::Read);
			buffer.Reserve(s->Length());
			s->ReadBuffer(buffer.Buffer(), 0, buffer.Size());
			SAFE_DELETE(s);

			demo->Serialize(buffer, false);

			Play(demo);
			return;
#endif

#ifdef OPT_DEMO_DEBUG_PLAYBACK
			if (m_demo.Frames.size() > 6000)
			{
				Demo* play_demo = Get_Demo(6000.0f);
				Play(play_demo);

				DataBuffer buffer;
				play_demo->Serialize(buffer, true);

				Stream* s = StreamFactory::Open("debug.zgd", StreamMode::Write);
				s->WriteBuffer(buffer.Buffer(), 0, buffer.Size());
				SAFE_DELETE(s);

				break;
			}
#endif

			m_demo_frame_timer -= frame_interval;
		}
		break;
	}
	case DemoManagerState::Playing:
	{
		if (!m_paused)
		{
			m_demo_frame_timer += time.Get_Delta_Seconds() * 1000.0f;
		}

		if (m_demo_frame < (int)m_playback_demo->Frames.size() - 1)
		{
			if (m_demo_frame_timer >= frame_interval || m_demo_frame == 0)
			{
				Play_Frame();

				if (m_demo_frame != 0)
				{
					m_demo_frame_timer -= frame_interval;
				}
			}
		}
		else
		{
			m_paused = true;
		}

		float frame_delta = m_demo_frame_timer / frame_interval;
		Interpolate_Frame(frame_delta);

		if (m_pause_on_next_frame)
		{
			m_paused = true;
			m_pause_on_next_frame = false;
		}

		break;
	}
	}
}

int DemoManager::Get_Current_Time()
{
	return (int)((float)m_demo_frame / (float)Demo_Frame_Rate);
}

int DemoManager::Get_Total_Time()
{
	if (m_playback_demo == NULL)
	{
		return 0;
	}
	return (int)((float)m_playback_demo->Frames.size() / (float)Demo_Frame_Rate);
}

void DemoManager::Set_Can_Record(bool bCanRecord)
{
	m_can_record = bCanRecord;
}

bool DemoManager::Is_Playing()
{
	return (m_state == DemoManagerState::Playing);
}

bool DemoManager::Is_Paused()
{
	return m_paused;
}

void DemoManager::Toggle_Pause()
{
	m_paused = !m_paused;
}

void DemoManager::Play(Demo* demo)
{
	m_demo_frame_timer = 0.0f;
	m_demo_frame = 0;
	m_state = DemoManagerState::Playing;
	m_playback_demo = demo;
	m_paused = false;
	m_initial_map_load_pending = true;

	//ParticleManager::Get()->Reset_For_Demo();
	ParticleManager::Get()->Reset();
}

void DemoManager::Restart()
{
	Play(m_playback_demo);
}

void DemoManager::Scrub(int frame_delta)
{
	if (NetManager::Get()->Is_Map_Load_Pending() || !m_can_record)
	{
		return;
	}

	const float frame_interval = 1000.0f / Demo_Frame_Rate;

	int target_frame = Clamp((m_demo_frame - 1) + frame_delta, 0, (int)m_playback_demo->Frames.size() - 1);

	m_scrubbing = true;

	// Go back to last keyframe if interping.
	if (frame_delta < 0)
	{
		while (m_demo_frame >= 0)
		{
			if (m_demo_frame < (int)m_playback_demo->Frames.size() && 
				m_playback_demo->Frames[m_demo_frame]->Keyframe == true && 
				m_demo_frame < target_frame)
			{
				break;
			}
			m_demo_frame--;
		}
	}

	m_demo_frame = Clamp(m_demo_frame, 0, (int)m_playback_demo->Frames.size() - 1);

	// Go forwards to target frame.
	while (m_demo_frame < target_frame)
	{
		Play_Frame();
	}

	Play_Frame(true);

	m_scrubbing = false;

	m_demo_frame_timer = 0.0f;
	ParticleManager::Get()->Reset();
	m_paused = true;
}

Demo* DemoManager::Get_Demo(float max_time)
{
	const float idle_frame_count = max_time * Demo_Frame_Rate;

	// Work out what keyframe to start from.
	int keyframe_index = 0;
	for (int i = (int)m_demo.Frames.size() - 1; i >= 0; i--)
	{
		DemoFrame* frame = m_demo.Frames[i];
		int total_frames = m_demo.Frames.size() - i;
		if (total_frames >= idle_frame_count)
		{
			if (frame->Keyframe)
			{
				keyframe_index = i;
				break;
			}
		}
	}

	// Copy appropriate keyframes.
	Demo* demo = new Demo();
	demo->Maps = m_demo.Maps;
	demo->Version = m_demo.Version;
	demo->Players = m_demo.Players;
	
	for (unsigned int i = keyframe_index; i < m_demo.Frames.size(); i++)
	{
		DemoFrame* frame = m_demo.Frames[i];
		
		DemoFrame* new_frame = new DemoFrame();
		*new_frame = *frame;

		demo->Frames.push_back(new_frame);
	}

	DBG_LOG("Retrieved demo containing %i frames (%.1f seconds).", demo->Frames.size(), (float)(demo->Frames.size() / (float)Demo_Frame_Rate));

	return demo;
}

