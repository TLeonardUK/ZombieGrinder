// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Collision/CollisionManager.h"

CollisionHashTable::CollisionHashTable()
	: m_grid_resolution(0)
	, m_grid_range(0)
	, m_grid_span(0)
	, m_bucket_size(0)
	, m_bucket_count(0)
	, m_buckets(NULL)
	, m_total_entries(0)
	, m_incarnation(0)
	, m_get_request(0)
{
}

CollisionHashTable::~CollisionHashTable()
{
	SAFE_DELETE_ARRAY(m_buckets);
}

void CollisionHashTable::Clear()
{
	memset(m_buckets, 0, sizeof(CollisionHashTableEntry) * m_total_entries);

	/*
	for (int i = 0; i < m_total_entries; i++)
	{
		CollisionHashTableEntry& handle = m_buckets[i];
		handle.index = i;
	}
	*/
}

void CollisionHashTable::Clear_Non_Persistent()
{
	m_incarnation++;

//	for (int i = 0; i < m_total_entries; i += m_bucket_size)
//	{
//		CollisionHashTableEntry& handle = m_buckets[i];
//		handle.nonpersistent_count = 0;
//	}
}

void CollisionHashTable::Clear_Flags()
{
	// Only bother doing this if we are about to wrap.
	if ((m_get_request + 0x10000000) < m_get_request)
	{
		for (int i = 0; i < m_total_entries; i++)
		{
			CollisionHashTableEntry& handle = m_buckets[i];
			if (handle.handle)
			{
				handle.handle->m_added_index = 0;
			}
		}
		m_get_request = 0;
	}
}

void CollisionHashTable::Resize(int grid_resolution, int grid_range, int max_grid_entries)
{
	m_grid_resolution	= grid_resolution;
	m_grid_range		= grid_range;
	m_grid_span			= grid_range / grid_resolution;

	m_bucket_count		= (m_grid_span * m_grid_span);
	m_bucket_size		= max_grid_entries;
	m_total_entries		= m_bucket_count * (m_bucket_size * 2);
	m_buckets			= new CollisionHashTableEntry[m_total_entries];
	//m_bucket_added_flags = new int[m_total_entries];

	for (int i = 0; i < m_total_entries; i++)
	{
		CollisionHashTableEntry& handle = m_buckets[i];
		//handle.index = i;
		handle.nonpersistent_count = 0;
		handle.persistent_count = 0;
		handle.handle = NULL;
	}

	DBG_LOG("sizeof(CollisionHashTable) = %i kb", (m_total_entries * sizeof(CollisionHashTableEntry)) / 1024);
}

void CollisionHashTable::Add(CollisionHandle* handle, bool persistent)
{
	DBG_ASSERT(handle != NULL);

	float min_x_px = handle->m_new_position.X + handle->m_area.X;
	float min_y_px = handle->m_new_position.Y + handle->m_area.Y;
	float max_x_px = handle->m_new_position.X + handle->m_area.X + handle->m_area.Width;
	float max_y_px = handle->m_new_position.Y + handle->m_area.Y + handle->m_area.Height;

	// Line has to be treated differently as its area is actually xy=start zw=end
	if (handle->m_shape == CollisionShape::Line)
	{
		min_x_px = Min(handle->m_new_position.X + handle->m_area.X, handle->m_new_position.X + handle->m_area.Width); 
		min_y_px = Min(handle->m_new_position.Y + handle->m_area.Y, handle->m_new_position.Y + handle->m_area.Height); 
		max_x_px = Max(handle->m_new_position.X + handle->m_area.X, handle->m_new_position.X + handle->m_area.Width); 
		max_y_px = Max(handle->m_new_position.Y + handle->m_area.Y, handle->m_new_position.Y + handle->m_area.Height); 
	}

	int min_x = (int)(floorf(min_x_px / m_grid_resolution)) - 1;
	int min_y = (int)(floorf(min_y_px / m_grid_resolution)) - 1;
	int max_x = (int)floorf(max_x_px / m_grid_resolution) + 1;
	int max_y = (int)floorf(max_y_px / m_grid_resolution) + 1;

	// Add to all grids the collision overlaps.
	for (int x = min_x; x <= max_x; x++)
	{
		for (int y = min_y; y <= max_y; y++)
		{
			int bucket_index = Bucket_Index(x, y);
			int bucket_offset = bucket_index * (m_bucket_size * 2);
			DBG_ASSERT(bucket_offset >= 0 && bucket_offset < m_total_entries);

			CollisionHashTableEntry& bucket = m_buckets[bucket_offset];

			if (persistent == true)
			{
				if (bucket.persistent_count >= m_bucket_size)
				{
					DBG_LOG("[WARNING] Collision hash table full at %f,%f,%f. Collision ignored.", handle->m_new_position.X, handle->m_new_position.Y, handle->m_new_position.Z);
					return;
				}

				int entry_offset = (bucket_offset + m_bucket_size) + bucket.persistent_count;
				DBG_ASSERT(entry_offset >= 0 && entry_offset < m_total_entries);

				CollisionHashTableEntry& entry = m_buckets[entry_offset];
				entry.handle = handle;

				bucket.persistent_count++;
			}
			else
			{
				if (bucket.incarnation != m_incarnation)
				{
					bucket.incarnation = m_incarnation;
					bucket.nonpersistent_count = 0;
				}

				if (bucket.nonpersistent_count >= m_bucket_size)
				{
					return;
				}

				int entry_offset = bucket_offset + bucket.nonpersistent_count;
				DBG_ASSERT(entry_offset >= 0 && entry_offset < m_total_entries);

				CollisionHashTableEntry& entry = m_buckets[entry_offset];
				entry.handle = handle;

				bucket.nonpersistent_count++;
			}
		}
	}
}

void CollisionHashTable::Trim_Duplicates(CollisionHandle** results, int& result_count)
{
	for (int i = 0; i < result_count; i++)
	{
		CollisionHandle* handle_a = results[i];

		bool bIsDuplicate = false;

		for (int j = 0; j < result_count; j++)
		{
			if (i != j)
			{
				CollisionHandle* handle_b = results[j];
				if (handle_a == handle_b)
				{
					bIsDuplicate = true;
					break;
				}
			}
		}

		if (bIsDuplicate)
		{
			results[i] = results[result_count - 1];
			result_count--;
			i--;
		}
	}
}
	
int CollisionHashTable::Find(CollisionHandle* handle, CollisionHandle** results, int results_size)
{
	DBG_ASSERT(handle != NULL);

	int result_count = 0;
	
	float min_x_px = handle->m_new_position.X + handle->m_area.X;
	float min_y_px = handle->m_new_position.Y + handle->m_area.Y;
	float max_x_px = handle->m_new_position.X + handle->m_area.X + handle->m_area.Width;
	float max_y_px = handle->m_new_position.Y + handle->m_area.Y + handle->m_area.Height;

	// Line has to be treated differently as its area is actually xy=start zw=end
	if (handle->m_shape == CollisionShape::Line)
	{
		min_x_px = Min(handle->m_new_position.X + handle->m_area.X, handle->m_new_position.X + handle->m_area.Width); 
		min_y_px = Min(handle->m_new_position.Y + handle->m_area.Y, handle->m_new_position.Y + handle->m_area.Height); 
		max_x_px = Max(handle->m_new_position.X + handle->m_area.X, handle->m_new_position.X + handle->m_area.Width); 
		max_y_px = Max(handle->m_new_position.Y + handle->m_area.Y, handle->m_new_position.Y + handle->m_area.Height); 
	}

	int min_x = (int)(floorf(min_x_px / m_grid_resolution));
	int min_y = (int)(floorf(min_y_px / m_grid_resolution));
	int max_x = (int)ceilf(max_x_px / m_grid_resolution) + 1;
	int max_y = (int)ceilf(max_y_px / m_grid_resolution) + 1;
	
	//DBG_LOG("SEARCHING: %i,%i->%i,%i", min_x, min_y, max_x, max_y);

	int get_request = ++m_get_request;

	for (int y = min_y; y <= max_y; y++)
	{
		for (int x = min_x; x <= max_x; x++)
		{
			int bucket_index = Bucket_Index(x, y);
			int bucket_offset = bucket_index * (m_bucket_size * 2);
			DBG_ASSERT(bucket_offset >= 0 && bucket_offset < m_total_entries);

			CollisionHashTableEntry& bucket = m_buckets[bucket_offset];

			// Grab all persistent handles.
			for (int i = 0; i < bucket.persistent_count; i++)
			{
				int entry_offset = (bucket_offset + m_bucket_size) + i;
				DBG_ASSERT(entry_offset >= 0 && entry_offset < m_total_entries);

				CollisionHashTableEntry& entry = m_buckets[entry_offset];
				
				bool ethereal = (handle->m_type != CollisionType::Solid || entry.handle->m_type != CollisionType::Solid);

				if (entry.handle->m_added_index < get_request &&
					entry.handle != handle &&
					(handle->m_collides_with & entry.handle->m_group) != 0 &&
					(ethereal || (entry.handle->m_collides_with & handle->m_group) != 0)) // If solid collision must be mutual.
				{
					//entry.handle->m_was_touched = true;
					entry.handle->m_added_index = get_request;
					results[result_count++] = entry.handle;

					/*
					for (int p = 0; p < result_count; p++)
					{
						for (int q = 0; q < result_count; q++)
						{
							if (p != q)
							{
								if (results[p] == results[q])
								{
									DBG_LOG("B Duplicate Result Found! Shit! %i", get_request);
								}
							}
						}
					}
					*/

					if (result_count >= results_size)
					{
						return result_count;
					}
				}
			}

			// And all the non-persistent handles.
			if (bucket.incarnation != m_incarnation)
			{
				bucket.incarnation = m_incarnation;
				bucket.nonpersistent_count = 0;
			}

			for (int i = 0; i < bucket.nonpersistent_count; i++)
			{
				int entry_offset = bucket_offset + i;
				DBG_ASSERT(entry_offset >= 0 && entry_offset < m_total_entries);

				CollisionHashTableEntry& entry = m_buckets[entry_offset];

				bool ethereal = (handle->m_type != CollisionType::Solid || entry.handle->m_type != CollisionType::Solid);

				if (entry.handle->m_added_index < get_request &&
					entry.handle != handle &&
					(handle->m_collides_with & entry.handle->m_group) != 0 &&
					(ethereal || (entry.handle->m_collides_with & handle->m_group) != 0)) // If solid collision must be mutual.
				{
					//entry.handle->m_was_touched = true;
					entry.handle->m_added_index = get_request;
					results[result_count++] = entry.handle;

					/*
					for (int p = 0; p < result_count; p++)
					{
						for (int q = 0; q < result_count; q++)
						{
							if (p != q)
							{
								if (results[p] == results[q])
								{
									DBG_LOG("A Duplicate Result Found! Shit! %i", get_request);
								}
							}
						}
					}
					*/

					if (result_count >= results_size)
					{
						return result_count;
					}
				}
			}
		}
	}

	return result_count;
}