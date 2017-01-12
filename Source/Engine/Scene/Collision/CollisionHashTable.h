// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_COLLISION_COLLISIONHASHTABLE_
#define _ENGINE_SCENE_COLLISION_COLLISIONHASHTABLE_

#include "Generic/Types/Vector3.h"

#include <vector>
#include <cstdlib>
#include <cstring>

struct CollisionHandle;

struct CollisionHashTableEntry
{
	CollisionHandle*	handle;
	int					nonpersistent_count;
	int					persistent_count;
	int					incarnation;
//	int					added_index;
};

class CollisionHashTable
{
private:
	int m_grid_resolution;
	int m_grid_range;
	int m_grid_span;

	int m_bucket_size;
	int m_bucket_count;
	int m_total_entries;

	int m_incarnation;

	CollisionHashTableEntry* m_buckets;

	//int* m_bucket_added_flags;
	int m_get_request;

	INLINE int Bucket_Index(int x, int y) const
	{
		int fx = abs(x) % m_grid_span;
		int fy = abs(y) % m_grid_span;
		return (fx * m_grid_span) + fy;
	}

	INLINE void Trim_Duplicates(CollisionHandle** results, int& result_count);
	
public:
	CollisionHashTable();
	~CollisionHashTable();

	void Clear();
	void Clear_Non_Persistent();
	void Clear_Flags();

	void Resize(int grid_resolution, int grid_range, int max_grid_entries);

	void Add(CollisionHandle* handle, bool persistent);
	
	int Find(CollisionHandle* handle, CollisionHandle** results, int results_size);

};

#endif

