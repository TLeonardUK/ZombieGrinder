// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Network/SnapshotManager.h"

#include "Engine/IO/BinaryStream.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "XScript/VirtualMachine/CSymbol.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Engine/EngineOptions.h"

#include "Generic/Helper/PersistentLogHelper.h"

SnapshotManager::SnapshotManager()
	: m_snapshot_counter(0)
{
}

SnapshotManager::~SnapshotManager()
{
}

void SnapshotManager::Purge_Actor(ScriptedActor* actor)
{
	ActorReplicationInfo& info = actor->Get_Replication_Info();
	info.snapshots.Resize(0);
	info.decoded_snapshots.Resize(0);
	info.per_user_encoded_buffers.Clear();
	info.per_user_decoded_buffers.Clear();
}

void SnapshotManager::Init_Actor(ScriptedActor* actor)
{
	ActorReplicationInfo& info = actor->Get_Replication_Info();
	if (info.snapshot_count == 0 &&
		info.decoded_snapshots.Size() == 0 &&
		info.snapshots.Size() == 0)
	{
		info.snapshots.Resize(*EngineOptions::net_snapshot_history);
		info.decoded_snapshots.Resize(*EngineOptions::net_snapshot_history);
	}
}

void SnapshotManager::Generate_Snapshot(ScriptedActor* actor, double tick)
{
	GameNetServer* server = GameNetManager::Get()->Game_Server();

	BinaryStream stream;
	int serialize_version = 0;

	Init_Actor(actor);
	
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();		
	CVMContext* context = actor->Get_Script_Context();	
	CVMContextLock lock = vm->Set_Context(context);

	ActorReplicationInfo& info = actor->Get_Replication_Info();

	ActorSnapshot result;
	result.tick = tick;
	result.is_full = false;

	if (server == NULL)
	{
		actor->Get_Script_Object().Get()->Serialize(&stream, CVMObjectSerializeFlags::Client_Replicated, &serialize_version);
	}
	else
	{
		if (info.snapshot_count == 0)
		{
			result.is_full = true;
			actor->Get_Script_Object().Get()->Serialize(&stream, CVMObjectSerializeFlags::Full, &serialize_version);
		}
		else
		{
			actor->Get_Script_Object().Get()->Serialize(&stream, CVMObjectSerializeFlags::Server_Replicated, &serialize_version);
		}
	}

	result.data.Set(stream.Data(), stream.Length());
	
	info.snapshots.Push(result);
	info.snapshot_count++;

	stream.Close();
}

void SnapshotManager::Increment_Snapshot_Counter()
{
	m_snapshot_counter++;
}

int SnapshotManager::Get_Snapshot_Count()
{
	return m_snapshot_counter;
}

ActorSnapshot* SnapshotManager::Get_Latest_Snapshot(ScriptedActor* actor)
{
	return &actor->Get_Replication_Info().snapshots.Peek();
}

bool SnapshotManager::Get_Last_Encoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer)
{
	if (user == NULL)
	{
		return false;
	}

	LastActorBufferInfo info;
	if (!actor->Get_Replication_Info().per_user_encoded_buffers.Get(user->Get_Net_ID(), info))
	{
		return false;
	}

	buffer = info.buffer;
	return true;
}

void SnapshotManager::Set_Last_Encoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer)
{
	LastActorBufferInfo info;
	info.buffer = buffer;

	actor->Get_Replication_Info().per_user_encoded_buffers.Remove(user->Get_Net_ID());
	actor->Get_Replication_Info().per_user_encoded_buffers.Set(user->Get_Net_ID(), info);
}

bool SnapshotManager::Get_Last_Decoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer)
{
	if (user == NULL)
	{
		return false;
	}

	LastActorBufferInfo info;
	if (!actor->Get_Replication_Info().per_user_decoded_buffers.Get(user->Get_Net_ID(), info))
	{
		return false;
	}

	buffer = info.buffer;
	return true;
}

void SnapshotManager::Set_Last_Decoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer)
{
	LastActorBufferInfo info;
	info.buffer = buffer;

	actor->Get_Replication_Info().per_user_decoded_buffers.Remove(user->Get_Net_ID());
	actor->Get_Replication_Info().per_user_decoded_buffers.Set(user->Get_Net_ID(), info);
}

void SnapshotManager::Encode_Snapshot(DataBuffer& destination, ScriptedActor* actor, GameNetUser* user)
{
	DataBuffer last_encoded_buffer;
	DataBuffer result;

	// Encode with previous snapshot sent to the user.
	if (Get_Last_Encoded_Buffer(actor, user, last_encoded_buffer))
	{
		result = destination.Delta_Encode(last_encoded_buffer);
	}
	// Encode with empty buffer if this is first send
	else
	{
		DataBuffer empty_buffer;
		result = destination.Delta_Encode(empty_buffer);
	}

	Set_Last_Encoded_Buffer(actor, user, destination);
	destination = result;
}

bool SnapshotManager::Has_Changed_State(ScriptedActor* actor, GameNetUser* user)
{
	if (actor->Get_Replication_Info().snapshots.Size() <= 1)
	{
		return true;
	}

	DataBuffer last_encoded_buffer;
	if (!Get_Last_Encoded_Buffer(actor, user, last_encoded_buffer))
	{
		return true;
	}

	ActorSnapshot* next_snapshot = &actor->Get_Replication_Info().snapshots.Peek();

	if (next_snapshot == NULL) 
	{
		return true;
	}

	if (last_encoded_buffer.Size() != next_snapshot->data.Size())
	{
		return true;
	}

	for (int i = 0; i < next_snapshot->data.Size(); i++)
	{
		if (last_encoded_buffer.Buffer()[i] != next_snapshot->data.Buffer()[i])
		{
			return true;
		}
	}

	return false;
}

void SnapshotManager::Decode_Snapshot(DataBuffer& source, ScriptedActor* actor, GameNetUser* user, bool bFull, double tick)
{
	ActorReplicationInfo& info = actor->Get_Replication_Info();

	// Make sure actor is initialised.
	Init_Actor(actor);

	ActorSnapshot snapshot;
	snapshot.tick = tick;
	snapshot.is_full = bFull;

	// Decode using last snapshot.
	DataBuffer last_decoded_buffer;
	if (Get_Last_Decoded_Buffer(actor, user, last_decoded_buffer))
	{
		snapshot.data = source.Delta_Decode(last_decoded_buffer);
	}

	// If first snapshot, use empty buffer.
	else
	{
		DataBuffer empty_buffer;
		snapshot.data = source.Delta_Decode(last_decoded_buffer);
	}

	// Store the last encoded snapshot, and the last decoded buffer.
	info.decoded_snapshots.Push(snapshot);

	Set_Last_Decoded_Buffer(actor, user, snapshot.data);
}

bool SnapshotManager::Should_Force_Update(ScriptedActor* actor, GameNetUser* user)
{
	if (user == NULL)
	{
		return false;
	}

	// Not encoded any buffers for the user yet? First sync then!
	if (!actor->Get_Replication_Info().per_user_encoded_buffers.Contains(user->Get_Net_ID()))
	{
		return true;
	}

	return false;
}

void SnapshotManager::Restart()
{
	m_snapshot_counter = 0;
}
