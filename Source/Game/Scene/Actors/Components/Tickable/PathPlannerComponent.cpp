// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Tickable/PathPlannerComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/Network/NetManager.h"

PathPlannerComponent::PathPlannerComponent()
	: m_target_position(0.0f, 0.0f, 0.f)
	, m_source_position(0.0f, 0.0f, 0.0f)
	, m_regenerate_delta(32.0f)
	, m_collision_group(CollisionGroup::NONE)
	, m_generating_handle()
	, m_active_handle()
	, m_movement_vector(0.0f, 0.0f, 0.0f)
	, m_target_node_index(1)
	, m_in_los(false)
	, m_los_radius(8.0f)
	, m_client_side(false)
	, m_at_target(false)
{
	m_los_check_offset = rand() % 255;

	memset(m_los_check, 0, sizeof(CollisionHandle*) * 3);
}

PathPlannerComponent::~PathPlannerComponent()
{
	m_active_handle.Dispose();
	m_generating_handle.Dispose();
}

int PathPlannerComponent::Get_ID()
{
	static int hash = StringHelper::Hash("PathPlannerComponent");
	return hash;
}

bool PathPlannerComponent::Is_Used_In_Demo()
{
	return false;
}

bool PathPlannerComponent::Has_Path()
{
	return m_active_handle.Is_Valid() && m_active_handle.Has_Path();
}

void PathPlannerComponent::Set_Target_Position(Vector3 val)
{
	m_target_position = val;
}

Vector3 PathPlannerComponent::Get_Target_Position()
{
	return m_target_position;
}

void PathPlannerComponent::Set_Source_Position(Vector3 val)
{
	m_source_position = val;
}

Vector3 PathPlannerComponent::Get_Source_Position()
{
	return m_source_position;
}

void PathPlannerComponent::Set_Regenerate_Delta(float val)
{
	m_regenerate_delta = val;
}

float PathPlannerComponent::Get_Regenerate_Delta()
{
	return m_regenerate_delta;
}

void PathPlannerComponent::Set_Collision_Group(CollisionGroup::Type val)
{
	m_collision_group = val;
}

CollisionGroup::Type PathPlannerComponent::Get_Collision_Group()
{
	return m_collision_group;
}

Vector3 PathPlannerComponent::Get_Movement_Vector()
{
	return m_movement_vector;
}

void PathPlannerComponent::Set_Client_Side(bool val)
{
	m_client_side = val;
}

bool PathPlannerComponent::Get_Client_Side()
{
	return m_client_side;
}

bool PathPlannerComponent::Get_Target_In_LOS()
{
	return m_in_los;
}

bool PathPlannerComponent::Get_At_Target()
{
	return m_at_target;
}

void PathPlannerComponent::Regenerate()
{
	if (m_generating_handle.Is_Valid())
	{
		m_generating_handle.Dispose();
	}

	m_generating_handle = PathManager::Get()->Create_Handle(
		m_source_position, 
		m_target_position, 
		m_collision_group
	);

	DBG_ASSERT(m_generating_handle.Is_Valid());

	m_generate_position = m_target_position;
}

void PathPlannerComponent::Tick(const FrameTime& time)
{	
	static const int LOS_CHECK_INTERVAL = 20;
	static const float AT_END_DISTANCE = 10.0f;

	m_at_target = false;
	
	// Do not do path planning on clients, this is a server job.
	if (!NetManager::Get()->Is_Server() && !m_client_side)
	{
		if (m_active_handle.Is_Valid())
		{
			m_active_handle.Dispose();
		}
		return;
	}

	// Generated new path? Swap the handles.
	if (m_generating_handle.Is_Valid() && m_generating_handle.Has_Path())
	{
		if (m_active_handle.Is_Valid())
		{
			m_active_handle.Dispose();
		}
		m_active_handle = m_generating_handle;
		m_generating_handle.Invalidate();

		m_target_node_index = 1;
		m_in_los = false;

		DBG_ASSERT(m_active_handle.Is_Valid());
		DBG_ASSERT(!m_generating_handle.Is_Valid());
	}

	// Moved enough since last generation?
	float movement_delta = (m_generate_position - m_target_position).Length();
	bool has_set_movement_vector = false;

	// Time to regenerate?
	if ((!m_generating_handle.Is_Valid() && !m_active_handle.Is_Valid()) || movement_delta > m_regenerate_delta)
	{
		if (!m_in_los)
		{
			Regenerate();
		}
	}

	// Calculate movement vector to next node.
	if (m_active_handle.Is_Valid() && 
		m_active_handle.Is_Complete() && 
		m_active_handle.Has_Path())
	{
		Path* path = m_active_handle.Get_Path();
		PathGraphNode** nodes;
		int node_count = path->Get_Nodes(nodes);

		// If we are in LOS of the target, just head straight there.
		bool los_collided = false;
		bool los_exists = false;

		for (int i = 0; i < 3; i++)
		{
			if (m_los_check[i] != NULL)
			{
				if (m_los_check[i]->Has_Collided())
				{
					los_collided = true;
				}

				m_los_check[i] = NULL;
				los_exists = true;
			}
		}

		if (los_exists == true)
		{
			m_in_los = !los_collided;
		}

		// When checking for LOS we do it by doing 3 line checks in the following format
		//
		//		.
		//			.
		//				.
		//
		// Its not 100% accurate but gives a reasonably cheap LOS check that ensures we can
		// make it round corners smoothly.
		int check_index = time.Get_Frame() + m_los_check_offset;
		if ((check_index % LOS_CHECK_INTERVAL) == 0)
		{
			for (int i = 0; i < 3; i++)
			{
				float offset_x = 0.0f;
				float offset_y = 0.0f;

				if (i == 0)
				{
					offset_x -= m_los_radius;
					offset_y -= m_los_radius;
				}
				else if (i == 2)
				{
					offset_x += m_los_radius;
					offset_y += m_los_radius;
				}

				m_los_check[i] = CollisionManager::Get()->Create_Handle
					(
					CollisionType::Non_Solid,
					CollisionShape::Line,
					CollisionGroup::Volume,
					CollisionGroup::Environment,
					Rect2D
					(
					m_source_position.X + offset_x,
					m_source_position.Y + offset_y,
					m_target_position.X,
					m_target_position.Y
					),
					Vector3(0.0f, 0.0f, 0.0f),
					Vector3(0.0f, 0.0f, 0.0f),
					false
					);
			}
		}

		if (!m_in_los)
		{
			// Move onto the next node.
			if (m_target_node_index >= node_count)
			{
				if ((m_target_position - m_source_position).Length() < AT_END_DISTANCE)
				{
					m_at_target = true;
				}

				if (m_active_handle.Is_Valid())
				{
					m_active_handle.Dispose();
				}

				Regenerate();

				return;
			}
			else
			{
				PathGraphNode* last_target = nodes[m_target_node_index - 1];
				PathGraphNode* target = nodes[m_target_node_index];
				Vector3 last_target_position = Vector3(last_target->position.X, last_target->position.Y, 0.0f);
				Vector3 target_position = Vector3(target->position.X, target->position.Y, 0.0f);

				Vector3 heading = (target_position - last_target_position).Normalize();
				Vector3 movement_vector = (target_position - m_source_position).Normalize();
				Vector3 unnormalised_movement_vector = (target_position - m_source_position);

				float dot = movement_vector.Dot(heading);
			
				// Wrong direction? We're need to move onto next target.
				if (dot <= 0.0f)
				{
					// Use last movement vector for this.
					// TODO: Probably should calculate next one correctly, but meh.
					m_target_node_index++;
				}
				else
				{
					m_movement_vector = unnormalised_movement_vector;
				}

				has_set_movement_vector = true;
			}
		}
		else
		{
			Vector3 next_heading = (m_target_position - m_source_position).Normalize();;
			float dot = next_heading.Dot(m_movement_vector);

			// Going in other direction from previous frame and withing radius check.
			m_at_target = ((dot <= 0.0f) && (m_target_position - m_source_position).Length() < AT_END_DISTANCE);
		}
	}

	// If not movement vector just head towards target.
	if (!has_set_movement_vector)
	{
		m_movement_vector = (m_target_position - m_source_position);
	}
}