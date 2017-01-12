// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_NETWORK_ACTOR_REPLICATION_INFO_
#define _GAME_NETWORK_ACTOR_REPLICATION_INFO_

#include "Generic/Types/CircleBuffer.h"

struct ActorSnapshot
{
	double		tick;
	bool		is_full;
	DataBuffer	data;
};

struct LastActorBufferInfo
{
	DataBuffer buffer;
};

// Contains some general meta-data used when replicating actors.
struct ActorReplicationInfo
{
	u32							unique_id;
	bool						spawned;
	bool						is_hibernating;
	double						last_update_time;

	int							despawn_snapshot_count;
								
	bool						bPurged;

	enum
	{
		max_snapshots = 8
	};

	int													snapshot_count;
	CircleBuffer<ActorSnapshot, max_snapshots>			snapshots;
	HashTable<LastActorBufferInfo, u32>					per_user_encoded_buffers;
	
	CircleBuffer<ActorSnapshot, max_snapshots>			decoded_snapshots;
	HashTable<LastActorBufferInfo, u32>					per_user_decoded_buffers;

	ActorReplicationInfo()
		: unique_id(0)
		, spawned(false)
		, is_hibernating(false)
		, last_update_time(0.0f)
		, despawn_snapshot_count(0)
		, snapshot_count(0)
		, bPurged(false)
	{
	}
};

#endif


