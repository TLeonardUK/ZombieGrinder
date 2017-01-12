// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_ACTOR_REPLICATOR_
#define _GAME_NETWORK_ACTOR_REPLICATOR_

#include "Generic/Patterns/Singleton.h"
#include "Engine/Engine/FrameTime.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/SnapshotManager.h"

#include "Game/Network/ActorReplicationInfo.h"

class ScriptedActor;
class BinaryStream;

// 0.5m recycled should be more than enough.
#define UNIQUE_ID_MAX_BITS   19
#define MAX_UNIQUE_ID		 524288

// 4k should be enough for most actors?
#define STATE_SIZE_MAX_SIZE 4096

struct CompressedStateHeader
{
	// 00000000 00000000 00000000 00000000
	// 11111111 11111111 11100000 00000000 = unique_id
	// 00000000 00000000 00011111 11111110 = state_size;
	// 00000000 00000000 00000000 00000001 = full;

	u32  unique_id;//  : 19;
	u32  state_size;// : 12;
	bool full;		//    : 1;

	void Read(Stream* stream);
	void Write(Stream* stream);
};

class ActorReplicator : public Singleton<ActorReplicator>
{
	MEMORY_ALLOCATOR(ActorReplicator, "Network");

private:	
	PACKET_RECIEVE_CALLBACK(ActorReplicator, Callback_EventsRecieved, NetPacket_C2S_ReplicationEvents,  m_callback_events_recieved);	
	PACKET_RECIEVE_CALLBACK(ActorReplicator, Callback_StatesRecieved, NetPacket_C2S_ReplicationStates,  m_callback_states_recieved);
	
	std::vector<ScriptedActor*>		m_actors;
	std::vector<ScriptedActor*>		m_spawned_actors;
	std::vector<int>				m_despawned_actor_ids;
	//std::vector<ScriptedActor*>		m_purge_pending_actors;

	SnapshotManager					m_snapshot_manager;

	std::vector<ReplicationEvent>	m_pending_events;

	std::vector<ReplicationEvent>	m_pending_rpcs;

	int m_simulation_drifting_frames;
	bool m_simulation_tick_drift_adjustment_init;

	CircleBuffer<double, 120> m_simulation_drift_avg_accumulator; 

	double m_server_tick;
	double m_server_tick_timer;

	double m_client_tick;
	double m_client_tick_timer;

	int m_snapshot_count;

	bool m_has_recieved_initial_update;
	bool m_has_recieved_initial_update_pending_spawn;

	u32 m_unique_id;

	double m_simulation_tick;

	CircleBuffer<double, 60>	m_avg_tick_rate_accumulator;
	double m_avg_tick_rate;

	CircleBuffer<double, 60>	m_avg_server_tick_rate_accumulator;
	double m_avg_server_tick_rate;

	double m_last_simulation_calc_timer;

	double m_last_tick_recieved_time;
	double m_last_tick_recieved;

	double m_predicted_server_tick;
	double m_simulation_tick_base;
	double m_simulation_tick_timer;

	enum
	{
		max_packet_size = 1000
	};

private:	
	void Send_Update(GameNetUser* user, bool is_full, int tick_rate, int target_rate, double tick);

	u32 Generate_Unique_ID();

	void Dump_Snapshot_States();

//	ReplicationActorState Create_Actor_State(ScriptedActor* actor);
//	ReplicationSpawnEvent Create_Spawn_Event(ScriptedActor* actor);
	float Calculate_Actor_Score(ScriptedActor* actor, GameNetUser* user, double tick_rate);

	void Client_Tick(const FrameTime& time);
	void Server_Tick(const FrameTime& time);
	
	//void Purge_Old_Actors();
	bool Should_Generate_Shapshot(bool server, ScriptedActor* actor);
	void Generate_Snapshots(bool server, double tick);

	void Send_Events(GameNetUser* user, double tick, bool is_full);
	void Send_State(GameNetUser* user, bool is_full, int tick_rate, int target_rate, double tick);

	void Calculate_Simulation_Tick();
	void Process_Pending_Events();

	void Recieved_Tick(double tick);
	
	bool Process_Event(ReplicationEvent& evt);

	double Sanitize_User_Predicted_Tick(double tick, GameNetUser* user);

	void Apply_States(bool bDoNotInterpolateClientStates);
	void Apply_Actor_State(ScriptedActor* actor, bool bDoNotInterpolateClientStates);

	void Process_Pending_RPCs();
	std::vector<int> Get_RPC_Targets(int target, int except);
	void Invoke_RPC(ReplicationEvent& evt);
	void Write_RPC_Value(BinaryStream* stream, CVMDataType* type, CVMValue& value);
	void Read_RPC_Value(BinaryStream* stream, CVMDataType* type, CVMValue& value);

public:

	double Get_Average_Server_Tick_Rate();
	ActorReplicator();
	~ActorReplicator();
	
	ScriptedActor* Get_Actor_By_ID(u32 id);

	// Add/Remove actors to replicate.
	void Spawn_Actor(ScriptedActor* actor);
	void Despawn_Actor(ScriptedActor* actor);
	//void Remove_Actor(ScriptedActor* actor);
	
	void Purge_Actor(ScriptedActor* actor, bool delete_at_end = true);

	void Despawn_Users_Actors(GameNetUser* user);

	// Send periodic updates.
	void Tick(const FrameTime& time);

	// Clears all actors, and resets everything ready for 
	// a new map/game.
	void Restart(bool bDoNotResetUsers = false);

	// Returns true if the local client has recieved an initial update.
	bool Has_Recieved_Initial_Update();

	// Sends a full update to the given user.
	void Send_Full_Update(GameNetUser* user);

	// Gets current simulation tick.	
	double Get_Simulation_Tick();

	// Gets the predicted server tick.
	double Get_Predicted_Server_Tick();

};

#endif