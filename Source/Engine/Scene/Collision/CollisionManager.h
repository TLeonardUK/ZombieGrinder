// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_COLLISION_COLLISIONMANAGER_
#define _ENGINE_SCENE_COLLISION_COLLISIONMANAGER_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Patterns/Singleton.h"
#include "Engine/Tasks/TaskManager.h"

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/StackAllocator.h"

#include "Generic/Math/Math.h"

#include "Engine/Scene/Collision/CollisionHashTable.h"

class CollisionSimulationTask;
class CollisionManager;
struct CollisionHandle;

struct CollisionShape
{
	enum Type
	{
		Rectangle,
		Oval,
		Line,
		Circle
	};
};

struct CollisionType
{
	enum Type
	{
		Non_Solid,
		Solid,
	};
};

struct CollisionGroup
{
	enum Type
	{
		NONE		= 0,
		Player		= 1,
		Enemy		= 2,
		Environment	= 4,
		Particle	= 8,
		Volume		= 16,
		Damage		= 32,
		All			= 0xFFFFFFFF,
	};
};

struct CollisionDamageType
{
	enum Type
	{
		NONE		= 0,
		Projectile	= 1,
		Melee		= 2,
		Fire		= 3,
		Ice			= 4,
		Explosive	= 5,
		Acid		= 6,

		// Gross, these need to go somewhere else.
		Buff_Ammo	= 7,
		Buff_Health	= 8,
		Buff_Damage	= 9,
		Buff_Price	= 10,
		Buff_Reload	= 11,
		Buff_ROF	= 12,
		Buff_Speed	= 13,
		Buff_XP		= 14,
	};
};

struct CollisionEventType
{
	enum Type
	{
		Touch
	};
};

struct PotentialCollision
{
	CollisionHandle*	Handle;
	Vector3				Intersection_Point;
};

struct CollisionEvent
{
	u8							Type;
	CollisionHandle*			From;
	CollisionHandle*			To;
	Vector3						Point;

	CollisionEvent*				Next;
};

struct CollisionHandle
{
private:
	friend class CollisionManager;
	friend class CollisionHashTable;
	friend class CollisionSimulationTask;

	CollisionShape::Type		m_shape;
	CollisionType::Type			m_type;
	CollisionGroup::Type		m_group;
	CollisionGroup::Type		m_collides_with;

	Vector3						m_new_position;
	Vector3						m_old_position;
	Vector3						m_start_position;
	Rect2D						m_area;
	bool						m_has_moved;
	CollisionEvent*				m_first_event;
	int							m_event_count;

	bool						m_was_touched;
	bool						m_was_considered;
	bool						m_damage_accepted;

	void*						m_meta_data;
	void*						m_instigator;
	int							m_instigator_incarnation;

	Vector3						m_velocity;
	Vector3						m_velocity_increment;
	bool						m_has_been_velocity_affected;

	bool						m_smoothed;

	int							m_added_index;

public:
	void* Get_Meta_Data()
	{
		return m_meta_data;
	}

	bool Has_Been_Velocity_Affected()
	{
		return m_has_been_velocity_affected;
	}

	void* Get_Instigator()
	{
		return m_instigator;
	}

	int Get_Instigator_Incarnation()
	{
		return m_instigator_incarnation;
	}

	CollisionShape::Type Get_Shape()
	{
		return m_shape;
	}

	CollisionType::Type Get_Type()
	{
		return m_type;
	}

	CollisionGroup::Type Get_Group()
	{
		return m_group;
	}

	CollisionGroup::Type Get_Collides_With()
	{
		return m_collides_with;
	}

	Vector3 Get_Center()
	{
		return Vector3
		(
			m_new_position.X + m_area.X + (m_area.Width * 0.5f),
			m_new_position.Y + m_area.Y + (m_area.Height * 0.5f),
			0.0f
		);
	}

	Vector3 Get_Old_Center()
	{
		return Vector3
			(
			m_old_position.X + m_area.X + (m_area.Width * 0.5f),
			m_old_position.Y + m_area.Y + (m_area.Height * 0.5f),
			0.0f
			);
	}

	bool Has_Collided()
	{
		return (m_first_event != NULL);

		/*
		CollisionEvent* handle = m_first_event;
		while (handle != NULL)
		{
			if (handle->Type == CollisionEventType::Touch)
				return true;

			handle = handle->Next;
		}
		return false;
		*/
	}

	void Clear_Collided()
	{
		m_event_count = 0;
		m_first_event = NULL;
	}

	Vector3 Get_Intersection_Point()
	{
		/*
		CollisionEvent* handle = m_first_event;
		while (handle != NULL)
		{
			if (handle->Type == CollisionEventType::Touch)
				return handle->Point;

			handle = handle->Next;
		}
		*/
		if (m_first_event != NULL)
			return m_first_event->Point;

		return Vector3(0.0f, 0.0f, 0.0f);
	}

	Vector3 Get_Closest_Intersection_Point(Vector3 base)
	{
		Vector3 result = Vector3(0.0f, 0.0f, 0.0f);
		float result_distance = 0.0f;

		CollisionEvent* evt = m_first_event;
		for (int i = 0; i < m_event_count; i++)
		{
			float dist = (base - evt->Point).Length();
			if (i == 0 || dist < result_distance)
			{
				result = evt->Point;
				result_distance = dist;
			}
			evt = evt->Next;
		}

		return result;
	}
	
	Vector3 Get_Position()
	{
		return m_new_position;
	}

	Vector3 Get_Start_Position()
	{
		return m_start_position;
	}

	Rect2D Get_Area()
	{
		return m_area;
	}

	bool Get_Damage_Accepted_Flag()
	{
		return m_damage_accepted;
	}

	void Set_Damage_Accepted_Flag()
	{
		m_damage_accepted = true;
	}

	void Clear_Damage_Accepted_Flag()
	{
		m_damage_accepted = false;
	}

	int Get_Events(CollisionEvent*& first_event)
	{
		first_event = m_first_event;
		return m_event_count;
	}

};

class CollisionSimulationTask : public Task
{
public:
	void Run();
};

class CollisionManager : 
	public Singleton<CollisionManager>
{
	MEMORY_ALLOCATOR(CollisionManager, "Scene");
	
public:
	enum
	{
		max_persistent_handles	= 8 * 1024,		// Usually map tiles
		max_collision_handles	= 4 * 1024,		// Dynamic objects like players/enemies/projectiles/etc.
		max_active_handles		= 2 * 1024,		// Maximum moving handles (ones we check for collision on).
		grid_range				= 2500,		
		grid_resolution			= 32,			
		max_grid_entries		= 64,
		max_events				= max_active_handles * 8, // 8 for each active handle, probably excessive as all hell. 
	};

private:

	TaskID m_task_id;
	CollisionSimulationTask* m_task;

	CollisionHandle m_persistent_handles[max_persistent_handles];
	int m_persistent_handle_count;
	bool m_persistent_changed;
	int m_persistent_change_count;

	int m_write_buffer;

	CollisionHandle m_handle_buffers[2][max_collision_handles];
	int m_handle_count;
	int m_write_handle_count;

	CollisionHandle* m_active_handles[2][max_active_handles];
	int m_active_handle_count;
	int m_write_active_handle_count;

	CollisionHashTable m_tree;

	StackAllocator<CollisionEvent, max_events> m_event_stacks[2];

	float m_simulate_delta;

	bool m_show_collision;

	std::vector<Line2D> m_debug_lines;
	std::vector<Vector3> m_debug_points;

protected:
	friend class CollisionSimulationTask;

	void Simulate();

	int Get_Collisions(CollisionHandle* primary, PotentialCollision* results, int results_size);

public:
	CollisionManager();
	~CollisionManager();

	void Reset();

	bool Get_Show_Collision();
	void Set_Show_Collision(bool toggle);
	
	CollisionHandle* Create_Handle(
		CollisionType::Type type,				// Type of collision.
		CollisionShape::Type shape,				// Shape of collision bounds.
		CollisionGroup::Type group,				// Used to match up collision with other handles.
		CollisionGroup::Type collides_with,		// Groups of collision we collide with.
		Rect2D area,							// Shape area.
		Vector3 new_position,					// New target position.
		Vector3 old_position,					// Last good position.
		bool persistent = false,				// Persists for the entire level.
		void* meta_data = NULL,					// Metadata stored in the handle, useful for linking up handles to objects.	
		void* instigator = NULL,				// Mainly used for damage events.
		int instigator_incarnation = -1,		// Checks if instigator is active.
		Vector2 velocity = Vector2(0.0f, 0.0f),
		bool smoothed = false);

	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time);

};

#endif

