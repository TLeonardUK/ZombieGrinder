// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_SNAPSHOT_MANAGER_
#define _GAME_NETWORK_SNAPSHOT_MANAGER_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/DataBuffer.h"
#include "Engine/Engine/FrameTime.h"
#include "Game/Network/GameNetUser.h"

#include "Game/Scene/Actors/ScriptedActor.h"

class SnapshotManager
{
	MEMORY_ALLOCATOR(SnapshotManager, "Network");

private:	
	int m_snapshot_counter;

	// Gets the last snapshot that was encoded for the given user using Encode_Snapshot.
	bool Get_Last_Encoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer);

	// Sets the last encoded snapshot.
	// There should be no reason to do this manually, its done by Encode_Snapshot, but here incase.
	void Set_Last_Encoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer);

	// Gets the last snapshot that was encoded for the given user using Encode_Snapshot.
	bool Get_Last_Decoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer);

	// Sets the last encoded snapshot.
	// There should be no reason to do this manually, its done by Encode_Snapshot, but here incase.
	void Set_Last_Decoded_Buffer(ScriptedActor* actor, GameNetUser* user, DataBuffer& buffer);

public:
	SnapshotManager();
	~SnapshotManager();

	// Purges all snapshots created for the given actor.
	void Purge_Actor(ScriptedActor* actor);
	void Init_Actor(ScriptedActor* actor);

	// Gets the number of snapshots that have been produced since snapshot was reset.
	int Get_Snapshot_Count();

	// Increment snapshot counter.
	void Increment_Snapshot_Counter();

	// Creates a new snapshot for the given actor.
	void Generate_Snapshot(ScriptedActor* actor, double tick);

	// Encodes the latest snapshot for the actor in a delta-encoded format to be
	// sent to the given user.
	// Returns false if snapshot is un-encoded (because its the first one after spawn), or
	// true otherwise.
	void Encode_Snapshot(DataBuffer& destination, ScriptedActor* actor, GameNetUser* user);

	// Decodes the latest snapshot recieved for the actor from a delta-encoded format.
	void Decode_Snapshot(DataBuffer& source, ScriptedActor* actor, GameNetUser* user, bool bFull, double tick);

	// Gets the latest snapshot for the given user.
	ActorSnapshot* Get_Latest_Snapshot(ScriptedActor* actor);

	// Returns true if an update should be forced this tick for the gien actor/user. This occurs
	// if the last update sent to the user is about to be evicted from the circle buffer (which would make it un-decodable by the user).
	bool Should_Force_Update(ScriptedActor* actor, GameNetUser* user);

	// Returns true if the actors state has not changed since last snapshot.
	bool Has_Changed_State(ScriptedActor* actor, GameNetUser* user);

	// Clears all actors, and resets everything ready for 
	// a new map/game.
	void Restart();

};

#endif