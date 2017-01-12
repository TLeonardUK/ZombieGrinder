// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/ActorReplicator.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetUser.h"

#include "Game/Runner/Game.h"
#include "Game/Scene/GameScene.h"

#include "Engine/Scene/Scene.h"

#include "Engine/Platform/Platform.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Network/Packets/GamePackets.h"

#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/UI/UIManager.h"
#include "Game/UI/Scenes/UIScene_MapLoading.h"

#include "Generic/Math/Math.h"

#include "Generic/Stats/Stats.h"
#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Helper/PersistentLogHelper.h"


#include <queue>
#include <functional>
#include <float.h>

DEFINE_FRAME_STATISTIC("Replication/Server Tick Time", float, g_server_tick_time, true);
DEFINE_FRAME_STATISTIC("Replication/Client Tick Time", float, g_client_tick_time, true);
DEFINE_FRAME_STATISTIC("Replication/Event Processing Time", float, g_event_processing_time, true);
DEFINE_FRAME_STATISTIC("Replication/RPC Processing Time", float, g_rpc_processing_time, true);
DEFINE_FRAME_STATISTIC("Replication/RPC Decode Time", float, g_rpc_decode_time, true);
DEFINE_FRAME_STATISTIC("Replication/RPC Invoke Time", float, g_rpc_invoke_time, true);
DEFINE_FRAME_STATISTIC("Replication/RPC Execution Time", float, g_rpc_execution_time, true);
DEFINE_FRAME_STATISTIC("Replication/State Interpolation Time", float, g_state_interpolation_time, true);
DEFINE_FRAME_STATISTIC("Replication/Event Count", float, g_event_count, true);
DEFINE_FRAME_STATISTIC("Replication/RPC Count", float, g_rpc_count, true);
DEFINE_FRAME_STATISTIC("Replication/Average Tick Rate", float, g_avg_tick_rate, true);

struct PriorityQueueActor
{
	ScriptedActor* actor;
	float score;

	struct Compare : std::binary_function<PriorityQueueActor,PriorityQueueActor,bool> 
	{
		bool operator() (const PriorityQueueActor& a, const PriorityQueueActor& b)
		{
			return a.score < b.score;
		}
	};
};

struct PriorityQueueActorState
{
	PriorityQueueActor	actor;
	DataBuffer			state;
	bool				full;
};

void CompressedStateHeader::Read(Stream* stream)
{
	u32 bit = stream->Read<u32>();

	unique_id = (bit >> 13);// & 0x1FFFFF;
	state_size = (bit >> 1) & 0xFFF;
	full = (bit & 0x1) != 0;
}

void CompressedStateHeader::Write(Stream* stream)
{

	//0x10e81fff = 00010000 11101000 00011111 11111111
	//123421     = 00000000 00000001 11100010 00011101
	//4095       = 00000000 00000000 00001111 11111111

	u32 bit = 0;
	bit |= (unique_id << 13);
	bit |= (state_size << 1);
	bit |= (full ? 1 : 0);
	stream->Write<u32>(bit);
}

ActorReplicator::ActorReplicator()
	: m_server_tick_timer(0.0f)
	, m_client_tick_timer(0.0f)
	, m_has_recieved_initial_update(false)
	, m_has_recieved_initial_update_pending_spawn(false)
	, m_unique_id(0)
	, m_server_tick(0)
	, m_client_tick(0)
	, m_snapshot_count(0)
	, m_callback_events_recieved(this, &ActorReplicator::Callback_EventsRecieved, false, NULL)
	, m_callback_states_recieved(this, &ActorReplicator::Callback_StatesRecieved, false, NULL)
	, m_simulation_tick(0.0f)
	, m_predicted_server_tick(0.0f)
	, m_simulation_tick_base(0.0f)
	, m_simulation_tick_timer(0.0f)
	, m_simulation_drifting_frames(0)
	, m_simulation_tick_drift_adjustment_init(false)
{
	BinaryStream state_stream;

	CompressedStateHeader header;
	header.unique_id	= 123421;
	header.state_size	= 4095;
	header.full			= true;
	header.Write(&state_stream);

	CompressedStateHeader decoded_header;
	state_stream.Seek(state_stream.Position() - 4);
	decoded_header.Read(&state_stream);

	DBG_ASSERT(header.unique_id == decoded_header.unique_id);
	DBG_ASSERT(header.state_size == decoded_header.state_size);
	DBG_ASSERT(header.full == decoded_header.full);
}

ActorReplicator::~ActorReplicator()
{
}

u32 ActorReplicator::Generate_Unique_ID()
{
	// TODO: Should validate that ID is actually unique?
	int id = m_unique_id;
	m_unique_id = (++m_unique_id) % MAX_UNIQUE_ID;
	return id;
}

void ActorReplicator::Spawn_Actor(ScriptedActor* actor)
{
	if (GameNetManager::Get()->Game_Server() != NULL)
	{
		ActorReplicationInfo& info = actor->Get_Replication_Info();
		info.unique_id = Generate_Unique_ID();
		//DBG_LOG("[Replication] [Server] Added actor %i to spawn list.", info.unique_id);
		m_spawned_actors.push_back(actor);
		m_actors.push_back(actor);
	}
	else
	{
		m_actors.push_back(actor);
	}
}

void ActorReplicator::Despawn_Actor(ScriptedActor* actor)
{
	PRINTF_PROFILE_SCOPE("Despawn_Actor");

	if (GameNetManager::Get()->Game_Server() != NULL)
	{
		ActorReplicationInfo& info = actor->Get_Replication_Info(); 		
		//DBG_LOG("[Replication] [Server] Added actor %i ('%s') to remove list.", info.unique_id, actor->Get_Script_Symbol()->symbol->name);
		
		info.spawned = false;
		info.despawn_snapshot_count = m_snapshot_count;

		// Add ID to despawn list.
		m_despawned_actor_ids.push_back(info.unique_id);

		// Destroy the scripted actor.
		Purge_Actor(actor, true);
	}
	else
	{
		// Destroy the scripted actor.
		Purge_Actor(actor, true);
	}
}

ScriptedActor* ActorReplicator::Get_Actor_By_ID(u32 id)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		ActorReplicationInfo& info = actor->Get_Replication_Info(); 		
		if (info.unique_id == id)
		{
			return actor;
		}
	}

	return NULL;
}

/*
void ActorReplicator::Remove_Actor(ScriptedActor* actor)
{
	if (GameNetManager::Get()->Game_Server() != NULL)
	{
		ActorReplicationInfo& info = actor->Get_Replication_Info(); 		
		DBG_LOG("[Replication] [Server] Forcibly removed actor from game.", info.unique_id);

		Purge_Actor(actor, false);
	}
	else
	{
		std::vector<ScriptedActor*>::iterator iter = std::find(m_actors.begin(), m_actors.end(), actor);
		if (iter != m_actors.end())
		{
			m_actors.erase(iter);
		}
	}
}
*/

void ActorReplicator::Despawn_Users_Actors(GameNetUser* user)
{
	DBG_LOG("Despawning all of user '%s' actors ...", user->Get_Username().c_str());

	user->Set_Controller_ID(-1);

	std::vector<ScriptedActor*> actors = m_actors;
	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		if (actor->Get_Owner() == user)
		{
			Despawn_Actor(actor);
			Game::Get()->Get_Game_Scene()->Despawn_Children(actor);
		}
	}
}

void ActorReplicator::Purge_Actor(ScriptedActor* actor, bool delete_at_end)
{
	if (actor->Get_Replication_Info().bPurged)
	{
		return;
	}

	actor->Get_Replication_Info().bPurged = true;

	//DBG_LOG("[Replication] [Server] Purging actor %i.", actor->Get_Replication_Info().unique_id);

	{
		PRINTF_PROFILE_SCOPE("Erase From Lists");
		{
			std::vector<ScriptedActor*>::iterator iter = std::find(m_actors.begin(), m_actors.end(), actor);
			if (iter != m_actors.end())
			{
				m_actors.erase(iter);
			}
		}
	
		{
			std::vector<ScriptedActor*>::iterator iter = std::find(m_spawned_actors.begin(), m_spawned_actors.end(), actor);
			if (iter != m_spawned_actors.end())
			{
				m_spawned_actors.erase(iter);
			}
		}
	}

	{
		PRINTF_PROFILE_SCOPE("Purge Snapshots");
		m_snapshot_manager.Purge_Actor(actor);
	}

	if (delete_at_end == true)
	{
		PRINTF_PROFILE_SCOPE("Register Despawn");
		Game::Get()->Get_Game_Scene()->Register_Despawn(actor);
		//SAFE_DELETE(actor);
	}
}

bool ActorReplicator::Should_Generate_Shapshot(bool server, ScriptedActor* actor)
{
	ActorReplicationInfo& info = actor->Get_Replication_Info();
	CVMObjectHandle script_handle = actor->Get_Script_Object();	
	CVMLinkedSymbol* symbol = script_handle.Get()->Get_Symbol();

	if (!symbol->symbol->class_data->is_replicated)
	{
		return false;
	}

	GameNetUser* user = actor->Get_Owner();
	bool is_owner = (user != NULL && user->Get_Online_User()->Is_Local());

	//server:
	//  send any replicated objects
	//client:
	//  send only client controlled vars we own

	if (!server)
	{
		if (symbol->symbol->class_data->replication_info->has_client_controlled_vars == false ||
			is_owner == false)
		{
			return false;
		}
	}

	return true;
}

void ActorReplicator::Generate_Snapshots(bool server, double tick)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		ActorReplicationInfo& info = actor->Get_Replication_Info();
		if (server == true || info.spawned == true)
		{
			if (Should_Generate_Shapshot(server, actor))
			{
				m_snapshot_manager.Generate_Snapshot(actor, tick);
			}
		}
	}

	m_snapshot_manager.Increment_Snapshot_Counter();
	m_snapshot_count++;

#ifdef DEBUG_BUILD
	//Dump_Snapshot_States();
#endif
}

struct SnapshotDumpState 
{
	CVMLinkedSymbol* symbol;
	int size;
	int count;

	static bool SortPredicate(const SnapshotDumpState& a, const SnapshotDumpState& b)
	{
		return a.size > b.size;
	}
};

void ActorReplicator::Dump_Snapshot_States()
{
	std::vector<SnapshotDumpState> states;

	for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		ActorReplicationInfo& info = actor->Get_Replication_Info();
		ActorSnapshot snapshot = info.snapshots.Peek();
		bool found = false;

		// We ignore full snapshots, these will always be pretty large. We care about the average incremental
		if (snapshot.is_full)
		{
			continue;
		}

		for (std::vector<SnapshotDumpState>::iterator iter2 = states.begin(); iter2 != states.end(); iter2++)
		{
			SnapshotDumpState& dump = *iter2;
			if (dump.symbol == actor->Get_Script_Symbol())
			{
				found = true;
				dump.count++;
				dump.size += snapshot.data.Size();
				break;
			}
		}

		if (!found)
		{
			SnapshotDumpState dump;
			dump.symbol = actor->Get_Script_Symbol();
			dump.count = 1;
			dump.size = snapshot.data.Size();
			states.push_back(dump);
		}
	}

	std::sort(states.begin(), states.end(), &SnapshotDumpState::SortPredicate);

	int total_size = 0;
	for (std::vector<SnapshotDumpState>::iterator iter = states.begin(); iter != states.end(); iter++)
	{
		SnapshotDumpState& dump = *iter;
		total_size += dump.size;
	}

	static int last_max_size = 0;
	if (total_size > last_max_size)
	{
		DBG_LOG("======================= LARGE SNAPSHOT STATE ==========================");
		int total_sub_size = 0;
		for (std::vector<SnapshotDumpState>::iterator iter = states.begin(); iter != states.end(); iter++)
		{
			SnapshotDumpState& dump = *iter;
			DBG_LOG("[%s] count=%i size=%i", dump.symbol->symbol->name, dump.count, dump.size);
			total_sub_size += dump.size;
		}
		DBG_LOG("### Total Snapshot State - %i bytes ###", total_sub_size);
		DBG_LOG("=======================================================================");

		last_max_size = total_size;
	}
}

void ActorReplicator::Server_Tick(const FrameTime& time)
{
	Platform* platform = Platform::Get();

	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	// Only update server if we actually have one -_-
	if (server == NULL)
	{
		return;
	}

	// If we are localhosting, client has already recieved initial update, as our
	// current state is the current update.
	if (client != NULL)
	{
		m_has_recieved_initial_update = true;
	}

	int tick_rate	= *EngineOptions::net_server_tick_rate;
	int target_rate = *EngineOptions::net_server_target_rate;

	double elapsed = Platform::Get()->Get_Ticks() - m_server_tick;
	if (elapsed >= tick_rate)
	{
		m_server_tick = Platform::Get()->Get_Ticks();

		// Calculate average tick rate.
		m_avg_server_tick_rate_accumulator.Push(elapsed);

		double average = 0.0f;
		for (int i = 0; i < m_avg_server_tick_rate_accumulator.Size(); i++)
		{
			average += m_avg_server_tick_rate_accumulator.Get(i);
		}

		m_avg_server_tick_rate = (average / m_avg_server_tick_rate_accumulator.Size());

		// Generate snapshots for the states of every actor.
		//float timer = platform->Get_Ticks();

		// Apply the server snapshot state (client states are non-interpolated).
		{
			STATISTICS_TIMED_SCOPE(g_state_interpolation_time);
			Apply_States(true);
		}

		// TODO: when sending snapshots to clients, send raw state, DONT use the interpolated values.
		Generate_Snapshots(true, m_server_tick);

		//float elapsed = platform->Get_Ticks() - timer;
		//DBG_LOG("Took %2.fms to generate snapshots for %i actors.", elapsed, m_actors.size());

		// Send updates to users.
		std::vector<GameNetUser*> users = manager->Get_Game_Net_Users();
		for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			GameNetUser* user = *iter;

			// Only send incremental update to a single local user. Don't
			// send any to localhost'ing users.
			if (user->Get_Local_User_Index() == 0 && 
				user->Get_Online_User()->Is_Local() == false &&
				user->Is_Synced())
			{
				Send_Update(*iter, user->Get_Full_Update_Pending(), tick_rate, target_rate, m_server_tick);
				user->Set_Full_Update_Pending(false);
			}
		}

		// Spawning/deleting should be done now, so we can clear this stuff out now.			
		for (std::vector<ScriptedActor*>::iterator iter = m_spawned_actors.begin(); iter != m_spawned_actors.end(); iter++)
		{
			ScriptedActor* actor = *iter;

			ActorReplicationInfo& info = actor->Get_Replication_Info(); 
			info.spawned = true;
	
		//	DBG_LOG("[Replication] [Server] Actor %i is now spawned.", info.unique_id);
		}
		for (std::vector<int>::iterator iter = m_despawned_actor_ids.begin(); iter != m_despawned_actor_ids.end(); iter++)
		{
		//	DBG_LOG("[Replication] [Server] Actor %i is now despawned.", (*iter));
		}

		// Clear some per-snapshot stuff up.
		m_pending_rpcs.clear();
		m_spawned_actors.clear();
		m_despawned_actor_ids.clear();
	}
}

void ActorReplicator::Client_Tick(const FrameTime& time)
{
	Platform* platform = Platform::Get();

	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	// Don't update client if we are hosting a server.
	if (server != NULL)
	{
		return;
	}

	// Only send our updates when we are in game.
	if (client->Get_Game_State() == GameNetClient_GameState::InGame &&
		m_has_recieved_initial_update == true)
	{
		int tick_rate	= *EngineOptions::net_client_tick_rate;
		int target_rate = *EngineOptions::net_client_target_rate;

		double elapsed = Platform::Get()->Get_Ticks() - m_client_tick;
		if (elapsed >= tick_rate)
		{
			m_client_tick = Platform::Get()->Get_Ticks();//m_client_tick_timer;

			// Generate snapshots for the states of every client-side actor.
			Generate_Snapshots(false, Get_Predicted_Server_Tick());

			// Send the update to the server.
			//static int print_idx = 0;
			//if (((print_idx++) % 10) == 0)
			//{
			//	DBG_LOG("SENDING STATE WITH PREDICTED TICK OF %.2f", Get_Predicted_Server_Tick());
			//}
			Send_Update(NULL, false, tick_rate, target_rate, Get_Predicted_Server_Tick());
		
			// Clear some per-snapshot stuff up.
			m_pending_rpcs.clear();
		}
	}
}

void ActorReplicator::Tick(const FrameTime& time)
{
	// Process events currently pending.
	{
		STATISTICS_TIMED_SCOPE(g_event_processing_time);
		Process_Pending_Events();
	}

	// Process pending RPC events.
	{
		STATISTICS_TIMED_SCOPE(g_rpc_processing_time);
		Process_Pending_RPCs();
	}

	// Update servers current world state.
	{
		STATISTICS_TIMED_SCOPE(g_server_tick_time);
		Server_Tick(time);
	}

	// Update the clients current world state.
	{
		STATISTICS_TIMED_SCOPE(g_client_tick_time);
		Client_Tick(time);
	}

	// Calculate current simulation tick.
	{
		Calculate_Simulation_Tick();
	}

	// Apply interpolation.
	{
		STATISTICS_TIMED_SCOPE(g_state_interpolation_time);
		Apply_States(false);
	}

	// Initial state sorted?
	if (m_has_recieved_initial_update_pending_spawn == true)
	{
		bool spawned_all = true;	
		for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
		{
			ScriptedActor* actor = *iter;
			if (actor->Get_Replication_Info().spawned == false)
			{
				spawned_all = false;
				break;
			}
		}

		if (spawned_all == true)
		{
			DBG_LOG("Spawned all '%i' actors. Sync complete.", m_actors.size());
			m_has_recieved_initial_update_pending_spawn = false;
			m_has_recieved_initial_update = true;
		}
	}

	g_avg_tick_rate.Set((float)m_avg_tick_rate);
}

//
// The simulation tick is the current point in time we are emulating this frame.
//
// Server:
//	For the server this is always the current time, as the server is always up to date.
//
// Client:
//	For the client this is a point in time in the past. This allows us to lerp between snapshots
//  given to us by the server
//
//	Tick is specified as the timestamp of the latest state minus the maximum theoretical latency between recieved 2 states.
//	Maximum latency should be set to: ping + tick_rate + (average_actual_tick_rate?).
//
//	We lerp between current and target tick rate to try and ease out judders when snapshots don't come in at expected
//  time intervals.
//
void ActorReplicator::Calculate_Simulation_Tick()
{
	Platform* platform = Platform::Get();

	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	//m_simulation_tick_timer += GameEngine::Get()->Get_Time()->Get_Frame_Time();

	if (server != NULL)
	{
		//double elapsed = Platform::Get()->Get_Ticks() - m_server_tick;

		m_simulation_tick = Platform::Get()->Get_Ticks();//m_server_tick;
	}
	else
	{	
		double time_since_last_tick		= platform->Get_Ticks() - m_last_tick_recieved_time;
		double rtt						= client->Get_Connection()->Get_Ping();

		// Do not like.
		double target_tick				= (m_last_tick_recieved + time_since_last_tick) - (m_avg_tick_rate * 1.25f); // + 25% for buffer room in case of jitter or tick rate variance.

		double old_last_simulation_calc_timer	= m_last_simulation_calc_timer;
		m_last_simulation_calc_timer			= Platform::Get()->Get_Ticks();
		m_simulation_tick						+= (m_last_simulation_calc_timer - old_last_simulation_calc_timer);

		// If we are drifting off the target tick, try and catch up over time.
		double drift = target_tick - m_simulation_tick;
	//	DBG_LOG("SIM=%.2f TARGET=%.2f DRIFT=%.2f", m_simulation_tick, target_tick, drift);
		m_simulation_tick += drift * (GameEngine::Get()->Get_Time()->Get_Delta_Seconds() * 1.0f);

		m_predicted_server_tick			= (m_last_tick_recieved + (rtt * 0.5f)) + time_since_last_tick;

		/*
		double time_lag				= m_avg_tick_rate * 2; // 2 tick in the past should be enough if updates are constant. We add +75% for buffer room.
		double time_since_last_tick = platform->Get_Ticks() - m_last_tick_recieved_time;
		double target_tick			= (m_last_tick_recieved + time_since_last_tick) - time_lag;
		double current_tick			= m_simulation_tick_base + (platform->Get_Ticks() - m_simulation_tick_timer);

		// Work out how much we are driving off target.
		double drift = (target_tick - current_tick);

		m_simulation_drift_avg_accumulator.Push(drift);
		double average_drift = 0.0f;
		for (int i = 0; i < m_simulation_drift_avg_accumulator.Size(); i++)
		{
			average_drift += m_simulation_drift_avg_accumulator.Get(i);
		}
		average_drift = (average_drift / m_simulation_drift_avg_accumulator.Size());

		if (m_simulation_tick_drift_adjustment_init == false || fabs(average_drift) > time_lag * 1.5f)
		{
			m_simulation_tick_drift_adjustment_init = true;
			m_simulation_tick_base = target_tick;
			m_simulation_tick_timer = Platform::Get()->Get_Ticks();
			DBG_LOG("Readjusted simulation tick due to average drift of %.1f", average_drift);
		}	

		double new_tick = m_simulation_tick_base + (platform->Get_Ticks() - m_simulation_tick_timer) + average_drift;
		double delta = (new_tick - m_simulation_tick);
		m_simulation_tick = new_tick;

		// Work out servers current tick by taking simulation and applying RTT.
		double ping						= client->Get_Connection()->Get_Ping();
		m_predicted_server_tick			= (m_last_tick_recieved + time_since_last_tick) + ping;
		*/
	}
}

double ActorReplicator::Get_Simulation_Tick()
{
	return m_simulation_tick;
}

double ActorReplicator::Get_Predicted_Server_Tick()
{
	return m_predicted_server_tick;
}

double ActorReplicator::Sanitize_User_Predicted_Tick(double tick, GameNetUser* user)
{
	// Assume current server tick. We don't have to deal with ping prediction etc this way.
	return m_server_tick;
	//return tick;

	//	double r = m_simulation_tick - *EngineOptions::net_client_tick_rate;
//	return r;

//	return m_simulation_tick;

/*
	//tick = user->Get_Base_Tick() + tick;

	// The tick a user gives us is offset back in time so we can lerp between thier states.
	//tick -= *EngineOptions::net_lerp_interval;

	// User should not be sending us future events, they should always be running behind 
	// or at the servers simulation.
	float max_offset = m_simulation_tick + *EngineOptions::net_lerp_interval + user->Get_Connection()->Get_Ping();
	if (tick > max_offset)
	{
		tick = max_offset;
	}

	return tick;
*/
}

void ActorReplicator::Process_Pending_Events()
{
	for (std::vector<ReplicationEvent>::iterator iter = m_pending_events.begin(); iter != m_pending_events.end(); )
	{
		ReplicationEvent& evt = *iter;
		if (true)//evt.Tick <= m_simulation_tick)
		{
			if (Process_Event(evt))
			{
				iter = m_pending_events.erase(iter);
				continue;
			}
			else
			{
				iter++;
			}
		}
		else
		{
			iter++;
		}
	}
}

bool ActorReplicator::Process_Event(ReplicationEvent& evt)
{
	Platform* platform = Platform::Get();

	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	g_event_count.Set(1);

	//DBG_LOG("[Replication] Processing event: unique_id=%i type=%i tick=%f", evt.Unique_ID, evt.Tick, evt.Type);

	switch (evt.Type)
	{
	case ReplicationEventType::Spawn:
		{
			if (server == NULL)
			{
				ScriptedActor* actor = Get_Actor_By_ID(evt.Unique_ID);
				ActorReplicationInfo& info = actor->Get_Replication_Info();

				if (info.decoded_snapshots.Size() > 0)
				{			
					DBG_LOG("[Replication] [Client] Spawning actor %i.", info.unique_id);

					// Grab the initial full snapshot.
					ActorSnapshot* shot = &info.decoded_snapshots.Get(0);
					DBG_ASSERT(shot->is_full);

					// Setup the actors script.
					actor->Setup_Script();
					
					CVirtualMachine* vm = GameEngine::Get()->Get_VM();
					CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

					// Deserialize the actors initial state.
					BinaryStream stream(shot->data.Buffer(), shot->data.Size());
					actor->Get_Script_Object().Get()->Deserialize(&stream, CVMObjectSerializeFlags::Full, 0x7FFFFFFF);
					actor->Setup_Event_Hooks();

					// Add to scene.	
					Scene* scene = GameEngine::Get()->Get_Scene();
					scene->Add_Actor(actor);
					scene->Add_Tickable(actor);
					
					// Change to default state.
					GameEngine::Get()->Get_VM()->Set_Default_State(actor->Get_Script_Object());

					info.spawned = true;

					return true;
				}
			}
			else
			{
				DBG_LOG("[Replication] [Server] Recieved illegal spawn request from client. Ignoring.");
				return true;
			}
			
			return false;
		}
	case ReplicationEventType::Despawn:
		{
			if (server == NULL)
			{
				ScriptedActor* actor = Get_Actor_By_ID(evt.Unique_ID);
				ActorReplicationInfo& info = actor->Get_Replication_Info();

				if (actor != NULL)
				{
					DBG_LOG("[Replication] [Client] Despawning actor %i.", evt.Unique_ID);
										
					info.spawned = false;

					Despawn_Actor(actor);
					Game::Get()->Get_Game_Scene()->Despawn_Children(actor);
				}
				else
				{
					DBG_LOG("[Replication] [Client] Attempt to despawn unknown actor '%i'", evt.Unique_ID);
				}
			}
			else
			{
				DBG_LOG("[Replication] [Server] Recieved illegal despawn request from client. Ignoring.");
			}

			return true;
		}
	case ReplicationEventType::RPC:
		{			
			// If actor is not spawned yet we need to wait for that to happen!
			ScriptedActor* actor = Get_Actor_By_ID(evt.Unique_ID);
			if (evt.Unique_ID != -1 && (actor == NULL || actor->Get_Replication_Info().spawned == false))
			{
				return false;
			}

			ReplicationEventRPCData spawn_data;
			BinaryStream spawn_data_stream(evt.Data.Buffer(), evt.Data.Size());
			spawn_data.Deserialize(&spawn_data_stream);
			
			// If server dispatch to other users.
			bool should_execute_local = false;

			for (std::vector<ReplicationEventRPCTarget>::iterator iter = spawn_data.Targets.begin(); iter != spawn_data.Targets.end(); )
			{
				ReplicationEventRPCTarget target = *iter;
				bool bIsLocal = false;

				if (target.Index == CVMRPCTarget::Server && server != NULL)
				{
					should_execute_local = true;
					bIsLocal = true;
				}
				else
				{
					NetUser* user = manager->Get_User_By_Net_ID(target.Index);
					if (user != NULL && user->Get_Online_User()->Is_Local())
					{
						should_execute_local = true;
						bIsLocal = true;
					}
				}			

				if (!bIsLocal)
				{
					iter++;
				}
				else
				{
					iter = spawn_data.Targets.erase(iter);
				}
			}

			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

			CVMLinkedSymbol* func_sym = vm->Find_Symbol(spawn_data.Symbol_Hash);

			// If client, assume we should execute.
			if (should_execute_local)
			{
				//DBG_LOG("[RPC EVENT] Running Local %s", func_sym->symbol->name);
				Invoke_RPC(evt);
			}

			// If server dispatch onto clients.
			if (server != NULL && spawn_data.Targets.size() > 0)
			{
				/*
				DBG_LOG("[RPC EVENT] Distributing %s to:", func_sym->symbol->name);
				for (unsigned int i = 0; i <  spawn_data.Targets.size(); i++)
				{
					NetUser* user = manager->Get_User_By_Net_ID(spawn_data.Targets[i].Index);
					DBG_LOG("[RPC EVENT] \t%i [%s]", spawn_data.Targets[i], user == NULL ? user->Get_Username().c_str() : "Unknown");
				}
				*/

				m_pending_rpcs.push_back(evt);
			}

			return true;
		}
	}
	
	return false;
}

void ActorReplicator::Recieved_Tick(double tick)
{
	if (abs(tick - m_last_tick_recieved) <= FLT_EPSILON)
	{
		return;
	}
	
	double time = Platform::Get()->Get_Ticks();
	double elapsed = time - m_last_tick_recieved_time;

	// Get rid of outlying tick rates to prevent out average going tits up.
	double target_rate = *EngineOptions::net_server_tick_rate * 3.0f;
	if (elapsed < 0 || elapsed >= target_rate)
	{
		elapsed = target_rate;
	}

	m_last_tick_recieved_time = time;
	m_last_tick_recieved = tick;

	//DBG_LOG("tick=%.2f elapsed=%.2f", tick, elapsed);

	/*
	if (elapsed < 30.0f)
	{
		DBG_LOG("WUT!?");
	}
	*/

	// Calculate average tick rate.
	m_avg_tick_rate_accumulator.Push(elapsed);

	double average = 0.0f;
	for (int i = 0; i < m_avg_tick_rate_accumulator.Size(); i++)
	{
		average += m_avg_tick_rate_accumulator.Get(i);
	}

	m_avg_tick_rate = (average / m_avg_tick_rate_accumulator.Size());


//	static float last_time = 0.0f;
//	if (m_simulation_tick < last_time)
//	{
//		DBG_LOG("TIME WENT BACKWARDS!");
//	}
//	last_time = m_simulation_tick;
	
	//DBG_LOG("Average-Tick-Rate: %.2f, Simulation-Tick: %.2f", m_avg_tick_rate, m_simulation_tick);
}

double ActorReplicator::Get_Average_Server_Tick_Rate()
{
	return m_avg_server_tick_rate;
}

void ActorReplicator::Send_Full_Update(GameNetUser* user)
{
	user->Set_Full_Update_Pending(true);
}

std::vector<int> ActorReplicator::Get_RPC_Targets(int target, int except)
{
	bool is_server = (GameNetManager::Get()->Game_Server() != NULL);
	std::vector<NetUser*> users = NetManager::Get()->Get_Net_Users();

	std::vector<int> targets;

	// Procuce a list of targets.
	switch (target)
	{
	case CVMRPCTarget::Server:
		{
			targets.push_back(target);
			break;
		}
	case CVMRPCTarget::Local:
		{
			for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
			{
				NetUser* user = *iter;
				if (user->Get_Online_User()->Is_Local() && user->Get_Local_User_Index() == 0)
				{
					targets.push_back(user->Get_Net_ID());
				}
			}

			if (is_server == true)
			{
				targets.push_back(CVMRPCTarget::Server);
			}

			break;
		}
	case CVMRPCTarget::All:
		{
			targets.push_back(CVMRPCTarget::Server);
			for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
			{
				NetUser* user = *iter;
				targets.push_back(user->Get_Net_ID());
			}
			break;
		}
	default:
		{
			targets.push_back(target);
			break;
		}
	}

	// Remove all targets we are excepting.
	switch (except)
	{
	case CVMRPCTarget::Server:
		{
			std::vector<int>::iterator iter = std::find(targets.begin(), targets.end(), CVMRPCTarget::Server);
			if (iter != targets.end())
				targets.erase(iter);					
			break;
		}
	case CVMRPCTarget::Local:
		{
			for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
			{
				NetUser* user = *iter;
				if (user->Get_Online_User()->Is_Local())
				{
					std::vector<int>::iterator iter2 = std::find(targets.begin(), targets.end(), user->Get_Net_ID());
					if (iter2 != targets.end())
						targets.erase(iter2);
				}
			}
			if (is_server == true)
			{
				std::vector<int>::iterator iter = std::find(targets.begin(), targets.end(), CVMRPCTarget::Server);
				if (iter != targets.end())
					targets.erase(iter);
			}
			break;
		}
	case CVMRPCTarget::All:
		{
			DBG_ASSERT(false); // Seriously, why. Pointless waste of time.
			break;
		}
	default:
		{
			std::vector<int>::iterator iter = std::find(targets.begin(), targets.end(), except);
			if (iter != targets.end())
				targets.erase(iter);
			break;
		}
	}

	return targets;
}

void ActorReplicator::Write_RPC_Value(BinaryStream* stream, CVMDataType* type, CVMValue& value)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	switch (type->type)
	{
	case CVMBaseDataType::Int:
		{
			stream->Write<s32>(value.int_value);
			break;
		}
	case CVMBaseDataType::Bool:
		{
			stream->Write<s8>(value.int_value);
			break;
		}
	case CVMBaseDataType::Float:		
		{
			stream->Write<f32>(value.float_value);
			break;
		}
	case CVMBaseDataType::String:
		{
			int len = value.string_value.Length();
			stream->Write<u16>(len); // 64k should really be enough for a string!
			stream->WriteBuffer(value.string_value.C_Str(), 0, len);
			break;
		}
	case CVMBaseDataType::Array:
		{
			CVMObject* obj = value.object_value.Get();
			stream->Write<u8>((obj != NULL));
			if (obj != NULL)
			{
				stream->Write<s32>(obj->m_data_size);
				for (int i = 0; i < obj->m_data_size; i++)
				{
					Write_RPC_Value(stream, type->element_type, obj->m_data[i]);
				}
			}
			break;
		}
	case CVMBaseDataType::Object:
		{
			CVMObject* obj = value.object_value.Get();
			stream->Write<u8>((obj != NULL));
			if (obj != NULL)
			{
				CVMLinkedSymbol* obj_sym = vm->Get_Symbol_Table_Entry(type->class_index);

				if (obj_sym->symbol->class_data->is_replicated  != 0)
				{
					ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(obj->Get_Meta_Data());
					stream->Write<u32>(actor->Get_Replication_Info().unique_id);
				}
				else
				{
					int final_version = 0x7FFFFFFF;
					obj->Serialize(stream, CVMObjectSerializeFlags::Full, &final_version);
				}
			}
			break;
		}
	}
}

void ActorReplicator::Read_RPC_Value(BinaryStream* stream, CVMDataType* type, CVMValue& value)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	switch (type->type)
	{
	case CVMBaseDataType::Int:
		{
			value.int_value = stream->Read<s32>();
			break;
		}
	case CVMBaseDataType::Bool:
		{
			value.int_value = stream->Read<s8>();
			break;
		}
	case CVMBaseDataType::Float:		
		{
			value.float_value = stream->Read<f32>();
			break;
		}
	case CVMBaseDataType::String:
		{
			int len = stream->Read<u16>();
			value.string_value = CVMString('\0', len);
			stream->ReadBuffer(value.string_value.Str(), 0, len);
			break;
		}
	case CVMBaseDataType::Array:
		{
			bool exists = (stream->Read<u8>() != 0);
			if (exists)
			{
				int elements = stream->Read<s32>();
				
				CVMObjectHandle arr = value.object_value;
				if (arr.Get() == NULL)
				{
					arr = vm->New_Array(vm->Get_Symbol_Table_Entry(type->class_index), elements);
				}
				else
				{
					arr.Get()->Resize(elements);
				}

				for (int i = 0; i < elements; i++)
				{
					Read_RPC_Value(stream, type->element_type, arr.Get()->m_data[i]);
				}

				value.object_value = arr;
			}
			else
			{
				value.object_value = NULL;
			}
			break;
		}
	case CVMBaseDataType::Object:
		{
			bool exists = (stream->Read<u8>() != 0);
			if (exists)
			{
				CVMObjectHandle obj = value.object_value;

				CVMLinkedSymbol* obj_sym = vm->Get_Symbol_Table_Entry(type->class_index);

				// If replicated, try and find replicated actor.
				if (obj_sym->symbol->class_data->is_replicated != 0)
				{
					int actor_id = stream->Read<u32>();

					ScriptedActor* actor = Get_Actor_By_ID(actor_id);
					obj = actor->Get_Script_Object();
				}
				else
				{
					obj = vm->New_Object(obj_sym, true);
					obj.Get()->Deserialize(stream, CVMObjectSerializeFlags::Full, 0x7FFFFFFF);
				}

				value.object_value = obj;
			}
			else
			{
				value.object_value = NULL;
			}
			break;
		}
	default:
		{
			DBG_ASSERT(false);
		}
	}
}

void ActorReplicator::Process_Pending_RPCs()
{
	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	// Both users can send rpc-events.
	// Always send the user rpc events.
	CVMRPCEvent rpc_evt;
	while (vm->Get_RPC_Event(rpc_evt))
	{			
		g_rpc_count.Set(1);

		// Dump parameters into a blob.
		BinaryStream param_stream;
		DBG_ASSERT(rpc_evt.parameters.size() == rpc_evt.symbol->symbol->method_data->parameter_count);
		int param_index = 0;
		for (std::vector<CVMValue>::iterator iter = rpc_evt.parameters.begin(); iter != rpc_evt.parameters.end(); iter++)
		{
			CVMValue& value = *iter;

			CVMLinkedSymbol* sym = vm->Get_Symbol_Table_Entry(rpc_evt.symbol->symbol->method_data->parameter_indexes[param_index]);
			DBG_ASSERT(sym != NULL);
			
			Write_RPC_Value(&param_stream, sym->symbol->variable_data->data_type, value);

			//value.Serialize(&param_stream, sym->symbol->variable_data->data_type);
			param_index++;
		}
		//data.Serialize(&param_stream);			

		// Create RPC event data.
		ReplicationEventRPCData data;
		data.Symbol_Hash = rpc_evt.symbol->symbol->unique_id;
		data.Parameters.Set(param_stream.Data(), param_stream.Length());
		
		// Set targets.
		bool bRunLocally = false;

		std::vector<int> targets = Get_RPC_Targets(rpc_evt.target, rpc_evt.except);
 		for (std::vector<int>::iterator iter = targets.begin(); iter != targets.end(); iter++)
		{
			ReplicationEventRPCTarget target;
			target.Index = *iter;

			NetUser* user = manager->Get_User_By_Net_ID(target.Index);

			// Don't add local users to replication event or we will end up running twice,
			// once now once when the server rebroadcasts to us.
			if (!(user != NULL && user->Get_Online_User()->Is_Local()))
			{
				if (!(target.Index == CVMRPCTarget::Server && server != NULL))
				{
					data.Targets.push_back(target);
				}
				else
				{
					bRunLocally = true;
				}
			}
			else
			{
				bRunLocally = true;
			}
		}

		// Dump data into a blob.
		BinaryStream data_stream;
		data.Serialize(&data_stream);			

		// Create the final event.
		ReplicationEvent evt;
		evt.Tick = Get_Simulation_Tick();
		evt.Type = ReplicationEventType::RPC;
		evt.Data.Set(data_stream.Data(), data_stream.Length());

		if (rpc_evt.symbol->symbol->method_data->is_static != 0)
		{
			evt.Unique_ID = -1;
		}
		else
		{
			if (rpc_evt.instance.object_value.Get() != NULL && rpc_evt.instance.object_value.Get()->Get_Meta_Data() != NULL)
			{
				ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(rpc_evt.instance.object_value.Get()->Get_Meta_Data());
				evt.Unique_ID = actor->Get_Replication_Info().unique_id;
			}
			else
			{
				DBG_LOG("Failed to replicate RPC, instance object appears to have been destroyed.");
				continue;
			}
		}

		// If target is local, we process this event locally.
		if (bRunLocally)
		{
			//DBG_LOG("[RPC] Running Local %s", rpc_evt.symbol->symbol->name);
			Invoke_RPC(evt);
		}

		if (data.Targets.size() > 0)
		{
			/*
			DBG_LOG("[RPC] Distributing %s to:", rpc_evt.symbol->symbol->name);
			for (unsigned int i = 0; i < data.Targets.size(); i++)
			{
				NetUser* user = manager->Get_User_By_Net_ID(data.Targets[i].Index);
				DBG_LOG("[RPC] \t%i [%s]", data.Targets[i].Index, user != NULL ? user->Get_Username().c_str() : "Unknown");
			}
			*/

			// Push onto event list.
			m_pending_rpcs.push_back(evt);
		}
	}

	// Check for pending GC.
	vm->Check_Pending_Collect();
}

void ActorReplicator::Invoke_RPC(ReplicationEvent& evt)
{
	STATISTICS_TIMED_SCOPE(g_rpc_execution_time);

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();		

	//DBG_LOG("Executing RPC event for object %i.", evt.Unique_ID);

	// Find context to execute on.
	CVMContext* context = NULL;
	CVMObjectHandle instance = NULL;
	
	if (evt.Unique_ID >= 0)
	{
		ScriptedActor* actor = Get_Actor_By_ID(evt.Unique_ID);
		if (actor == NULL)
			return;

		context = actor->Get_Script_Context();
		instance = actor->Get_Script_Object();
	}
	else
	{
		context = vm->Get_Static_Context();
		instance = NULL;
	}
	
	DBG_ASSERT(context != NULL);

	CVMContextLock lock = vm->Set_Context(context);
	CVMLinkedSymbol* func_sym = NULL;

	{
		STATISTICS_TIMED_SCOPE(g_rpc_decode_time);

		// Grab RPC data.
		ReplicationEventRPCData rpc_data;
		BinaryStream rpc_data_stream(evt.Data.Buffer(), evt.Data.Size());
		rpc_data.Deserialize(&rpc_data_stream);

		// Find the symbol.
		func_sym = vm->Find_Symbol(rpc_data.Symbol_Hash);
		DBG_ASSERT(func_sym != NULL);
	
		// Push in each parameter.
		BinaryStream param_stream(rpc_data.Parameters.Buffer(), rpc_data.Parameters.Size());
		for (int i = 0; i < func_sym->symbol->method_data->parameter_count; i++)
		{
			CVMDataType* type = vm->Get_Symbol_Table_Entry(func_sym->symbol->method_data->parameter_indexes[i])->symbol->variable_data->data_type;

			CVMValue value;
			Read_RPC_Value(&param_stream, type, value);

			vm->Push_Parameter(value);
		}
	}

	{
		STATISTICS_TIMED_SCOPE(g_rpc_invoke_time);

		//bool bPrint = (stricmp(func_sym->symbol->name, "RPC_Die") == 0);

		//if (bPrint)
		//	printf("\n[[[ Starting RPC: %s ]]]\n", func_sym->symbol->name);

		// Invoke.
		CVMValue instance_param = instance;
		CVMContext* ctx = vm->Get_Active_Context();

		//double time_start = Platform::Get()->Get_Ticks();

		//if (bPrint)
		//	vm->Set_Profiling_Enabled(true);
	
		vm->Invoke(func_sym, instance_param, false, false, true);
	
		//if (bPrint)
		//	vm->Set_Profiling_Enabled(false);

		//double time_end = Platform::Get()->Get_Ticks() - time_start;

		//if (bPrint)
		//	printf("[[[ RPC %s Took %.2f ms ]]]\n\n", func_sym->symbol->name, time_end);
	}
}

void ActorReplicator::Send_Events(GameNetUser* user, double tick, bool is_full)
{
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	GameNetClient* client = GameNetManager::Get()->Game_Client();

	std::vector<ReplicationEvent> events;
	
	// Only server sends spawn etc events.
	if (user != NULL)
	{
		// If this is a full update, send spawns events for all the actors.
		if (is_full == true)
		{
			for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
			{
				ScriptedActor* actor = *iter;
			
				ActorReplicationInfo& info = actor->Get_Replication_Info();
				
				GameNetUser* owner = actor->Get_Owner();

				ReplicationEventSpawnData data;
				data.Class_ID = actor->Get_Script_Object().Get()->Get_Symbol()->symbol->unique_id;
				data.Owner_ID = owner == NULL ? -1 : owner->Get_Net_ID();

				BinaryStream data_stream;
				data.Serialize(&data_stream);

				ReplicationEvent evt;
				evt.Tick = tick;
				evt.Type = ReplicationEventType::Spawn;
				evt.Unique_ID = info.unique_id;
				evt.Data.Set(data_stream.Data(), data_stream.Length());
				events.push_back(evt);
			}
		}
		else
		{
			// Always send the user new-actors.
			for (std::vector<ScriptedActor*>::iterator iter = m_spawned_actors.begin(); iter != m_spawned_actors.end(); iter++)
			{
				ScriptedActor* actor = *iter;
			
				ActorReplicationInfo& info = actor->Get_Replication_Info();
			
				GameNetUser* owner = actor->Get_Owner();

				ReplicationEventSpawnData data;
				data.Class_ID = actor->Get_Script_Object().Get()->Get_Symbol()->symbol->unique_id;
				data.Owner_ID = owner == NULL ? -1 : owner->Get_Net_ID();
			
				BinaryStream data_stream;
				data.Serialize(&data_stream);
			
				ReplicationEvent evt;
				evt.Tick = tick;
				evt.Type = ReplicationEventType::Spawn;
				evt.Unique_ID = info.unique_id;
				evt.Data.Set(data_stream.Data(), data_stream.Length());
				events.push_back(evt);
			}
		}
	}
	
	// Dump RPC events into packet. 
	// [WARNING] Order is important, make sure these come AFTER the spawn event for an entity :).
	for (std::vector<ReplicationEvent>::iterator iter = m_pending_rpcs.begin(); iter != m_pending_rpcs.end(); iter++)
	{
		ReplicationEvent& evt = *iter;				
		evt.Tick = tick;//Get_Simulation_Tick();
		events.push_back(evt);
	}

	if (user != NULL)
	{
		// Always send the user deleted actors.
		// [WARNING] Order is important, make sure these go after the RPC events so final RPC's
		//			 are fired before the actor is despawned.
		for (std::vector<int>::iterator iter = m_despawned_actor_ids.begin(); iter != m_despawned_actor_ids.end(); iter++)
		{
			ReplicationEvent evt;
			evt.Tick = tick;
			evt.Type = ReplicationEventType::Despawn;
			evt.Unique_ID = *iter;
			events.push_back(evt);
		}	
	}

	// Split state up into multiple packets to ensure we don't go over MTU's
	int total_packet_size = 0;
	int event_index = 0;
	while (event_index < (int)events.size())
	{
		int total_update_size = 0; 
		int event_count = 0;

		NetPacket_C2S_ReplicationEvents packet;

		// Packets are written in binary.
		BinaryStream blob_stream;

		// Keep adding states until packet size gets too big!
		while (event_index < (int)events.size())
		{
			ReplicationEvent& state = events.at(event_index);

			int size_increment = sizeof(u32) + sizeof(u32) + state.Data.Size();
			if (total_update_size + size_increment > max_packet_size)
			{
				DBG_ASSERT_STR(total_update_size > 0, "Individual event is larger than MTU. What the fuck are you trying to sync!?");
				break;
			}

			blob_stream.Write<u32>(state.Unique_ID);
			blob_stream.Write<f64>(state.Tick);
			blob_stream.Write<u8>(state.Type);
			blob_stream.Write<u16>(state.Data.Size());
			blob_stream.WriteBuffer(state.Data.Buffer(), 0, state.Data.Size());
			
		//	packet.Events.push_back(state);

			event_index++;
			event_count++;
			total_update_size += size_increment;
		}
		
		// Partial/full flag.
		packet.Partial = (events.size() != 0);
		
		// Create a blob for the packet.
		DataBuffer blob(blob_stream.Data(), blob_stream.Length());
		blob.Compress(packet.Compressed_Blob);

		//DBG_ONSCREEN_LOG(0x2000009, Color::Green, 10000.0f, "Compressed Event Blob: %i (%i events)", packet.Compressed_Blob.Size(), event_count);

		//DBG_LOG("Sending %i event bytes - %i events.", packet.Compressed_Blob.Size(), event_count);

		//DBG_LOG("Event Packet: data=%i ", packet.Compressed_Blob.Size());

		//DBG_LOG("\tEvent Packet: partial=%i size=%i events=%i", packet.Partial, total_update_size, packet.Events.size());
		
		// Send the state packet on unreliable channel. We can drop states if the network gets
		// saturated, better than waiting for resend - especially given a new state will be
		// there in the next few ms's.
		if (user != NULL)
			user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
		else
		{
			if (server != NULL)
				server->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
			else
				client->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
		}
	}
}

void ActorReplicator::Send_State(GameNetUser* user, bool is_full, int tick_rate, int target_rate, double tick)
{
	Platform* platform = Platform::Get();
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	GameNetClient* client = GameNetManager::Get()->Game_Client();
	GameNetUser* primary_user = static_cast<GameNetUser*>(GameNetManager::Get()->Get_Primary_Local_Net_User());

	int update_quota = (int)((float)target_rate / (1000.0f / tick_rate));

	// Now prioritise all other actors until we fill our update_quota (or just send all if this is a full update).
	std::priority_queue<PriorityQueueActor, std::vector<PriorityQueueActor>, PriorityQueueActor::Compare> priority_list;
		
	// Calculate scores for each actor and prioritise their states.
	for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		PriorityQueueActor pri;
		pri.actor = *iter;
		
		ActorReplicationInfo& actor_info = pri.actor->Get_Replication_Info();
		if (actor_info.spawned == false && user == NULL)
		{
			continue;
		}

		CVMLinkedSymbol* class_symbol = pri.actor->Get_Script_Object().Get()->Get_Symbol();
		CVMReplicationInfo* info = class_symbol->symbol->class_data->replication_info;

		GameNetUser* owner = pri.actor->Get_Owner();
		bool is_owner = (owner == NULL && server != NULL) || (owner != NULL && owner->Get_Online_User()->Is_Local());

		// Client: Only send actors with client controlled vars.
		// Server: Send info for all replicated actors.
		if ((user == NULL && info->has_client_controlled_vars != 0 && is_owner) || user != NULL)
		{
			pri.score = Calculate_Actor_Score(pri.actor, user, (float)tick_rate);
			priority_list.push(pri);
		}
	}

	// Keep sending actors until we reach out update-quota.
	std::vector<PriorityQueueActorState> states;
	int total_update_size = 0;

	while (priority_list.size() > 0)
	{
		PriorityQueueActor pqa = priority_list.top();
		priority_list.pop();

		ActorReplicationInfo& info = pqa.actor->Get_Replication_Info();
		
		// Send update if doing a full update, we have packet space left or this is a newly spawned object.
		if (is_full == false)
		{
			if (info.spawned == true)
			{
				if (!m_snapshot_manager.Should_Force_Update(pqa.actor, user))
				{
					// Update quota up?
					if (total_update_size >= update_quota)
					{
					//	DBG_LOG("SKIPPING UPDATE - Over quota!");
					//	break;
					}

					// If snapshot is same as previous, ignore this actor.
					if (!m_snapshot_manager.Has_Changed_State(pqa.actor, user))
					{
						continue;
					}
				}
			}
		}

		info.last_update_time = platform->Get_Ticks();

		PriorityQueueActorState evt;
		evt.actor = pqa;
		evt.full = false;

		// Send full state.
		if (is_full == true)
		{
			BinaryStream out_stream;
			
			CVirtualMachine* vm = GameEngine::Get()->Get_VM();
			CVMContextLock lock = vm->Set_Context(evt.actor.actor->Get_Script_Context());

			int serialize_version = 0;
			evt.actor.actor->Get_Script_Object().Get()->Serialize(&out_stream, CVMObjectSerializeFlags::Full, &serialize_version);
			evt.state.Set(out_stream.Data(), out_stream.Length());
			evt.full = true;
		}

		// Delta-encode snapshot with previous snapshot sent to the user.
		else
		{
			ActorSnapshot* last_snapshot = m_snapshot_manager.Get_Latest_Snapshot(pqa.actor);
			evt.state.Set(last_snapshot->data.Buffer(), last_snapshot->data.Size());
			evt.full = (last_snapshot != NULL && last_snapshot->is_full);
		}

		/*std::string class_name = pqa.actor->Get_Script_Object().Get()->Get_Symbol()->symbol->name;
		if (class_name == "Dungeon_Crawler_Mode" && !evt.full)
		{
			float multiplier = *reinterpret_cast<float*>(evt.state.Buffer() + (3 * 4));
			DBG_ONSCREEN_LOG(0x2000000, Color::Green, 10000.0f, "Last Sent Snapshot - Tick=%.1f, Multiplier=%.2f", tick, multiplier);
		}*/

		m_snapshot_manager.Encode_Snapshot(evt.state, pqa.actor, user == NULL ? primary_user : user);

		// Store in packet and move on.
		states.push_back(evt);	

		total_update_size += evt.state.Size();
	}
	
	// Split state up into multiple packets to ensure we don't go over MTU's
	//int total_state_size = 0;
	//int total_states = states.size();
	int compressed_state_size = 0;
	bool first_sent = false;
	while (states.size() > 0 || 
		   first_sent == false) // Always want to send something even if no states to ensure clients have the correct server tick.
	{
		int total_update_size = 0; 
		int total_states = 0;

		first_sent = true;

		NetPacket_C2S_ReplicationStates packet;
		packet.Tick = tick;

		// Packets are written in binary.
		BinaryStream state_stream;
		BinaryStream header_stream;

		// Keep adding states until packet size gets too big!
		while (states.size() > 0)
		{
			PriorityQueueActorState& state = states.at(states.size() - 1);

			int size_increment = sizeof(u32) + state.state.Size();
			if (total_update_size + size_increment > max_packet_size)
			{
				DBG_ASSERT_STR(total_update_size > 0, "Individual actor state is larger than MTU (%i/%i). What the fuck are you trying to sync!?", size_increment, max_packet_size);
				break;
			}

			DBG_ASSERT_STR(state.state.Size() <= STATE_SIZE_MAX_SIZE, "Individual actor state is larger than max size (%i / %i)", state.state.Size(), STATE_SIZE_MAX_SIZE);

			CompressedStateHeader header;
			header.unique_id	= state.actor.actor->Get_Replication_Info().unique_id;
			header.state_size	= state.state.Size();
			header.full			= state.full;
			header.Write(&header_stream);

			CompressedStateHeader decoded_header;
			header_stream.Seek(header_stream.Position() - 4);
			decoded_header.Read(&header_stream);

			DBG_ASSERT(header.unique_id == decoded_header.unique_id);
			DBG_ASSERT(header.state_size == decoded_header.state_size);

			state_stream.WriteBuffer(state.state.Buffer(), 0, state.state.Size());

			states.pop_back();

			total_update_size += size_increment;
			total_states++;
		}

		// Compress state blob - our delta-encoding should give it 
		// very high entropy so this should shrink it right down.
		DataBuffer header_compressed;
		packet.Compressed_Header_Data.Set(header_stream.Data(), header_stream.Length());
		packet.Compressed_Header_Data.Compress(header_compressed);
		packet.Compressed_Header_Data = header_compressed;

		// Compress state blob - our delta-encoding should give it 
		// very high entropy so this should shrink it right down.
		DataBuffer compressed;
		packet.Compressed_State_Data.Set(state_stream.Data(), state_stream.Length());
		packet.Compressed_State_Data.Compress(compressed);
		packet.Compressed_State_Data = compressed;
		
		// Partial/full flag.
		packet.Partial = (states.size() != 0);

		//BinaryStream s;
		//packet.Serialize(&s);

		//DBG_ONSCREEN_LOG(0x2000006, Color::Green, 10000.0f, "State=%i Header=%i Packet=%i", packet.Compressed_State_Data.Size(), packet.Compressed_Header_Data.Size(), s.Length());

		//DBG_LOG("Sending %i header bytes, %i state bytes - %i states.", header_compressed.Size(), compressed.Size(), total_states);

		// Send the state packet on unreliable channel. We can drop states if the network gets
		// saturated, better than waiting for resend - especially given a new state will be
		// there in the next few ms's.
		if (user != NULL)
			user->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
		else
		{
			if (server != NULL)
			{
				server->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
			}
			else
			{
				client->Get_Connection()->Send(&packet, NET_CONNECTION_CHANNEL_SYSTEM_RELIABLE, true);
			}
		}
	}

//	DBG_LOG("size=%i state=%i overhead=%i actors=%i", total_state_size, compressed_state_size, total_state_size - compressed_state_size, total_states);
}

void ActorReplicator::Send_Update(GameNetUser* user, bool is_full, int tick_rate, int target_rate, double tick)
{
	// Some debugging informations!
	if (user != NULL)
	{
		if (is_full == true)
		{
			DBG_LOG("[Replication] [Server] Sending full update to %s", user->Get_Username().c_str());
		}
		else
		{
		//	DBG_LOG("[Replication] [Server] Sending incremental update to %s", user->Get_Username().c_str());
		}
	}

	// Server->Client does spawning/deleting/hibernation, Client->Server does not.
	Send_Events(user, tick, is_full);

	// Send state updates.
	Send_State(user, is_full, tick_rate, target_rate, tick);
}

float ActorReplicator::Calculate_Actor_Score(ScriptedActor* actor, GameNetUser* user, double tick_rate)
{
	// Prioritisation works like this:
	//
	//	Actors are assigned a score based on the following algorithm:
	//		per_actor_priority	= 0-1 depending on user-defined priority value in script file.
	//		ideal_update_rate	= tick_rate	* per_actor_priority
	//		distance_to_user	= distance to closest local-user
	//		distance			= max(min_replication_distance_factor, min(0, distance_to_user - min_replication_distance) / max_replication_distance)
	//		time				= (time-since-last-update / ideal_update_rate)
	//		
	//		score = time * distance
	//	
	//	Actors are added to update packet until the quota is filled, 
	//  starting with those with the highest value. Actors that
	//	are currentl hibernating are ignored.
	//
	//  Should result in:
	//		Actors furthest away getting updated the least.
	//		Actors updated most recently being defered to update ones that haven't been.
	//		High priority actors (players etc) still getting updated at a steady rate.
	
	Platform* platform = Platform::Get();
	
	ActorReplicationInfo& replication_info = actor->Get_Replication_Info();
	CVMLinkedSymbol* class_symbol = actor->Get_Script_Object().Get()->Get_Symbol();
	CVMReplicationInfo* info = class_symbol->symbol->class_data->replication_info;

	float actor_priority			= (info->priority / 100.0f);
	float ideal_update_rate			= (float)(tick_rate * actor_priority);
	float distance_to_user			= 0.0f;// TODO
	float time_since_last_update	= (float)(platform->Get_Ticks() - replication_info.last_update_time);

	float distance					= 1.0f;// TODO Max(, Min());
	float time						= (time_since_last_update / ideal_update_rate);

	float score						= time * distance;

	return score;
}

bool ActorReplicator::Callback_EventsRecieved(NetPacket_C2S_ReplicationEvents* packet)
{	
	Platform* platform = Platform::Get();

	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();
	
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();			

	DataBuffer blob;
	blob.Reserve(packet->Compressed_Blob.Size());
	packet->Compressed_Blob.Decompress(blob);

	BinaryStream event_stream(blob.Buffer(), blob.Size());

	// Recieved server events.
	if (server == NULL)
	{
		while (!event_stream.IsEOF())
		{
			// If its a spawn event we need to create an "unspawned" actor immediately so we have somewhere for 
			// the states to go before its actual tick event.
			ReplicationEvent evt;
			evt.Unique_ID = event_stream.Read<u32>();
			evt.Tick = event_stream.Read<f64>();
			evt.Type = (ReplicationEventType::Type)event_stream.Read<u8>();
			int buffer_size = event_stream.Read<u16>();
			evt.Data.Reserve(buffer_size);
			event_stream.ReadBuffer(evt.Data.Buffer(),0,  buffer_size);

			if (evt.Type == ReplicationEventType::Spawn)
			{
				ReplicationEventSpawnData spawn_data;
				BinaryStream spawn_data_stream(evt.Data.Buffer(), evt.Data.Size());
				spawn_data.Deserialize(&spawn_data_stream);
				
				CVMLinkedSymbol* sym = vm->Find_Symbol(spawn_data.Class_ID);
				DBG_ASSERT(sym != NULL);
				
				DBG_LOG("[Replication] Creating dummy actor (0x%08x) '%s' (id=%i owner=%i) waiting for spawn.", spawn_data.Class_ID, sym->symbol->name, evt.Unique_ID, spawn_data.Owner_ID);

				NetUser* owner = manager->Get_User_By_Net_ID(spawn_data.Owner_ID);

				ScriptedActor* actor = new ScriptedActor(vm, sym);
				actor->Set_Owner(owner == NULL ? NULL : static_cast<GameNetUser*>(owner));

				ActorReplicationInfo& info = actor->Get_Replication_Info();
				info.unique_id = evt.Unique_ID;
				info.is_hibernating = false;
				info.spawned = false;
			}

			m_pending_events.push_back(evt);
		}
	}

	// Recieved client events.
	else
	{
		GameNetUser* user = static_cast<GameNetUser*>(manager->Get_Game_Net_User_By_Connection(packet->Get_Recieved_From()));
		if (user == NULL)
		{
			DBG_LOG("Failed to get user by connection. User left?");
			return true;
		}
		
		// If this is first state packet, set base tick.
		//if (user->Get_Base_Tick() == 0.0f)
		//{
		//	user->Set_Base_Tick(m_simulation_tick - *EngineOptions::net_lerp_interval);
		//	DBG_LOG("[Replication] [Server] User %i has base tick of %f", user->Get_Net_ID(), user->Get_Base_Tick());
		//}

		while (!event_stream.IsEOF())
		{
			// If its a spawn event we need to create an "unspawned" actor immediately so we have somewhere for 
			// the states to go before its actual tick event.
			ReplicationEvent evt;
			evt.Unique_ID = event_stream.Read<u32>();
			evt.Tick = Sanitize_User_Predicted_Tick(event_stream.Read<f64>(), user);
			evt.Type = (ReplicationEventType::Type)event_stream.Read<u8>();
			int buffer_size = event_stream.Read<u16>();
			evt.Data.Reserve(buffer_size);
			event_stream.ReadBuffer(evt.Data.Buffer(), 0, buffer_size);

			m_pending_events.push_back(evt);
		}
	}

	return true;
}

bool ActorReplicator::Callback_StatesRecieved(NetPacket_C2S_ReplicationStates* packet)
{
	Platform* platform = Platform::Get();

	GameNetManager* manager = GameNetManager::Get();
	GameNetServer* server = manager->Game_Server();
	GameNetClient* client = manager->Game_Client();

	// Recieved server state update.
	if (server == NULL)
	{
		GameNetUser* user = static_cast<GameNetUser*>(manager->Get_Primary_Local_Net_User());
		if (user == NULL)
		{
			DBG_LOG("Failed to get user by connection. User left?");
			return true;
		}

		// Add snapshot to snapshot manager.
		DataBuffer decompressed;
		packet->Compressed_State_Data.Decompress(decompressed);
//		int state_offset = 0;

		DataBuffer decompressed_header;
		packet->Compressed_Header_Data.Decompress(decompressed_header);
//		int state_offset = 0;

		BinaryStream stream(decompressed.Buffer(), decompressed.Size());
		BinaryStream header_stream(decompressed_header.Buffer(), decompressed_header.Size());

		while (!header_stream.IsEOF())
		{
			CompressedStateHeader header;
			//header_stream.ReadBuffer(&header, 0, sizeof(CompressedStateHeader));
			header.Read(&header_stream);

			DataBuffer state_buffer;
			state_buffer.Reserve(header.state_size, false);
			stream.ReadBuffer(state_buffer.Buffer(), 0, header.state_size);

			ScriptedActor* actor = Get_Actor_By_ID(header.unique_id);
			if (actor != NULL)
			{
				m_snapshot_manager.Decode_Snapshot(state_buffer, actor, user, header.full, packet->Tick);
			/*
				std::string class_name = actor->Get_Script_Symbol()->symbol->name;
				if (class_name == "Dungeon_Crawler_Mode")
				{
					DBG_ONSCREEN_LOG(0x2000001, Color::Green, 10000.0f, "Last Recv Snapshot - Tick=%.1f", packet->Tick);
					//DBG_LOG("Recv snapshot @ %.1f", packet->Tick);
				}
			*/
			}
			else
			{
				DBG_LOG("Failed to decode snapshot for unknown actor %i.", header.unique_id);
			}
		}

		// Recieved our first game-state? Yay.
		if (packet->Partial == false && m_has_recieved_initial_update == false && m_has_recieved_initial_update_pending_spawn == false)
		{
			m_has_recieved_initial_update_pending_spawn = true;
			DBG_LOG("Recieved initial update, sync will be complete following initial spawn.");
		}

		if (packet->Partial == false)
		{
			// Update server tick.
			m_server_tick = Max(m_server_tick, packet->Tick);

			// Store tick stats.
			Recieved_Tick(packet->Tick);
		}
	}

	// Recieved client state update.
	else
	{
		GameNetUser* user = static_cast<GameNetUser*>(manager->Get_Game_Net_User_By_Connection(packet->Get_Recieved_From()));
		if (user == NULL)
		{
			DBG_LOG("Failed to get user by connection. User left?");
			return true;
		}

		// If this is first state packet, set base tick.
		//if (user->Get_Base_Tick() == 0.0f)
		//{
		//	user->Set_Base_Tick(m_simulation_tick);
		//	DBG_LOG("[Replication] [Server] User %i has base tick of %f", user->Get_Net_ID(), m_simulation_tick);
		//}

		// Add snapshot to snapshot manager.
		DataBuffer decompressed;
		packet->Compressed_State_Data.Decompress(decompressed);
//		int state_offset = 0;

		DataBuffer decompressed_header;
		packet->Compressed_Header_Data.Decompress(decompressed_header);
//		int state_offset = 0;

		BinaryStream stream(decompressed.Buffer(), decompressed.Size());
		BinaryStream header_stream(decompressed_header.Buffer(), decompressed_header.Size());

		while (!header_stream.IsEOF())
		{
			CompressedStateHeader header;
			//header_stream.ReadBuffer(&header, 0, sizeof(CompressedStateHeader));
			header.Read(&header_stream);

			DataBuffer state_buffer;
			state_buffer.Reserve(header.state_size, false);
			stream.ReadBuffer(state_buffer.Buffer(), 0, header.state_size);

			ScriptedActor* actor = Get_Actor_By_ID(header.unique_id);

			if (actor != NULL)
			{
				// Make sure this user is the objects parent.
				if (actor->Get_Owner() == NULL || actor->Get_Owner()->Get_Connection() != packet->Get_Recieved_From())
				{
					DBG_LOG("[Replication] [Server] User attempted to send snapshot of object that they do not own!");
					continue;
				}

				double sanity_tick = Sanitize_User_Predicted_Tick(packet->Tick, user);
				
				//static int print_idx = 0;
				//if (((print_idx++) % 10) == 0)
				//{
				//DBG_ONSCREEN_LOG(0x30000001, Color::Orange, 1.0f, "Last Tick: %.2f", sanity_tick);
				//}

				// Decode the snapshot.
				m_snapshot_manager.Decode_Snapshot(state_buffer, actor, user, header.full, sanity_tick);
			}
		}
	}

	// Process any spawn states waiting for states.
	Process_Pending_Events();

	return true;
}

void ActorReplicator::Restart(bool bDoNotResetUsers)
{
	if (!bDoNotResetUsers)
	{
		std::vector<GameNetUser*> users = GameNetManager::Get()->Get_Game_Net_Users();
		for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			GameNetUser* user = *iter;
			user->Restart();
		}
	}

	for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		m_snapshot_manager.Purge_Actor(*iter);
	}
	//for (std::vector<ScriptedActor*>::iterator iter = m_purge_pending_actors.begin(); iter != m_purge_pending_actors.end(); iter++)
	//{
	//	m_snapshot_manager.Purge_Actor(*iter);
	//}

	m_pending_events.clear();
	m_pending_rpcs.clear();
	m_actors.clear();
	m_spawned_actors.clear();
	m_despawned_actor_ids.clear();
	//m_purge_pending_actors.clear();
	m_has_recieved_initial_update = false;
	m_has_recieved_initial_update_pending_spawn = false;
	m_unique_id = 0;

	m_simulation_tick_drift_adjustment_init = false;

	m_server_tick = 0;
	m_client_tick = 0;
	m_snapshot_count = 0;
	m_server_tick_timer = Platform::Get()->Get_Ticks();
	m_client_tick_timer = Platform::Get()->Get_Ticks();

	m_snapshot_manager.Restart();
}

bool ActorReplicator::Has_Recieved_Initial_Update()
{
	return m_has_recieved_initial_update;
}

void ActorReplicator::Apply_States(bool bDoNotInterpolateClientStates)
{
	// Applies the current state to all users.
	for (std::vector<ScriptedActor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		ActorReplicationInfo& info = actor->Get_Replication_Info();
	
		if (info.spawned == true)
		{
			Apply_Actor_State(actor, bDoNotInterpolateClientStates);
		}
	}
}

void ActorReplicator::Apply_Actor_State(ScriptedActor* actor, bool bDoNotInterpolateClientStates)
{
	GameNetServer* server = GameNetManager::Get()->Game_Server();
	
	GameNetUser* owner = actor->Get_Owner();
	bool is_owner = owner != NULL && owner->Get_Online_User()->Is_Local();

	ActorSnapshot* prev_snapshot = NULL;
	ActorSnapshot* next_snapshot = NULL;

	ActorReplicationInfo& info = actor->Get_Replication_Info();
	
	// If server and client-replicated then simulate lerp-interval behind.
	double target_tick = m_simulation_tick;

	// Don't interpolate client states if requested.
	if (server != NULL)
	{
		if (bDoNotInterpolateClientStates)
		{
			target_tick = m_server_tick;// - (*EngineOptions::net_client_tick_rate * 20.0f);
		}
		else
		{
			target_tick = m_simulation_tick - (*EngineOptions::net_client_tick_rate * 2.0f);
		}
	}

	// Can't do anything if we have no snapshots.D
	if (info.decoded_snapshots.Size() <= 0)
	{
		return;
	}

	// Get the before and after snapshots for the current simulation tick.
	int next_id = -1;
	int prev_id = -1;
	bool all_full = true;
	for (int i = 0; i < info.decoded_snapshots.Size(); i++)
	{
		ActorSnapshot& shot = info.decoded_snapshots.Peek(i);

		if (shot.is_full == true)
		{
			continue;
		}

		all_full = false;

		if (shot.tick > target_tick && (next_snapshot == NULL || shot.tick < next_snapshot->tick))
		{
			next_snapshot = &shot;
			next_id = i;
		}
		if (shot.tick <= target_tick && (prev_snapshot == NULL || shot.tick > prev_snapshot->tick))
		{
			prev_snapshot = &shot;
			prev_id = i;
		}

		if (next_snapshot != NULL && prev_snapshot != NULL)
		{
			break;
		}
	}

	// No in-front snapshots?
	if (next_snapshot == NULL)
	{
		next_snapshot = prev_snapshot;
		next_id = prev_id;
	}
	if (prev_snapshot == NULL)
	{
		prev_snapshot = next_snapshot;
		prev_id = next_id;
	}
	if (next_snapshot == NULL || prev_snapshot == NULL)
	{
		if (all_full == true)
		{
			next_snapshot = &info.decoded_snapshots.Peek(0);
			prev_snapshot = next_snapshot;
		}
		else
		{
			// Shouldn't really be able to get here.
			return;
		}
	}
	
	double range = (next_snapshot->tick - prev_snapshot->tick);
	double delta = Clamp((target_tick - prev_snapshot->tick) / range, 0.0f, 1.0f);
	if (next_snapshot->tick == prev_snapshot->tick)
	{
		delta = 1.0;
	}

	/*if (next_snapshot->tick != prev_snapshot->tick)
	{
		std::string n = actor->Get_Script_Object().Get()->Get_Symbol()->symbol->name;
		if (n == "Fodder_Zombie")
		{
		//	DBG_ONSCREEN_LOG(reinterpret_cast<int>(reinterpret_cast<int*>(actor)), Color::Orange, 1.0f, "Interpolating: Target=%.2f From=%.2f To=%.2f Delta=%.2f Range=%.2f/%.2f", target_tick, prev_snapshot->tick, next_snapshot->tick, delta, range, m_avg_tick_rate);
		}
	}*/

	//static int counter = 0;
	//if (((counter++) % 100) == 0)
	//{
	//	DBG_LOG("Lerping between %f and %f (tick=%f delta=%f)", prev_snapshot->tick, next_snapshot->tick, target_tick, delta);
	//}

//	if (actor->Get_Replication_Info().unique_id == 1)
//	{
//		DBG_LOG("tick=%f delta=%f prev=%f next=%f", target_tick, delta, prev_snapshot->tick, next_snapshot->tick);
//	}

	// Go through each replicated variable in the snapshot and apply it
	// based on the delta.
	BinaryStream prev_stream(prev_snapshot->data.Buffer(), prev_snapshot->data.Size());
	BinaryStream next_stream(next_snapshot->data.Buffer(), next_snapshot->data.Size());
	
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

	// Note: If we do this then we can end up in a situation where we do an initial object deserialize, the object updates, and then we stomp over the updated variables :(. This is what fucks up the dungeon door desync.
	if (all_full)
	{
		return;
	}

	//	if (all_full)
//	{
//		return;
//		actor->Get_Script_Object().Get()->Deserialize(&next_stream, CVMObjectSerializeFlags::Full, 0x7FFFFFFF);
//	}
//	else if (server != NULL)
	if (server != NULL)
	{
		actor->Get_Script_Object().Get()->Interpolate(&prev_stream, &next_stream, (float)delta, CVMObjectSerializeFlags::Client_Replicated, 0x7FFFFFFF, is_owner);
	}
	else
	{
		/*
		std::string class_name = actor->Get_Script_Object().Get()->Get_Symbol()->symbol->name;
		if (class_name == "Dungeon_Crawler_Mode")
		{
			float multiplier = *reinterpret_cast<float*>(next_snapshot->data.Buffer() + (3 * 4));
			DBG_ONSCREEN_LOG(0x2000002, Color::Green, 10000.0f, "Interp Snapshot - Tick=%.1f, Multiplier=%.2f", next_snapshot->tick, multiplier);
		//	DBG_LOG("Interp snapshot @ %.1f, Multiplier=%.2f", next_snapshot->tick, multiplier);
		}
		*/

		actor->Get_Script_Object().Get()->Interpolate(&prev_stream, &next_stream, (float)delta, CVMObjectSerializeFlags::Server_Replicated, 0x7FFFFFFF, is_owner);
	}
}