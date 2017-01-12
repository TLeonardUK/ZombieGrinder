// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_PATHPLANNERCOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_PATHPLANNERCOMPONENT_

#include "Engine/Scene/Tickable.h"
#include "Engine/Scene/Collision/CollisionManager.h"
#include "Engine/Scene/Pathing/PathManager.h"
#include "Game/Scene/Actors/Components/Tickable/TickableComponent.h"

class ScriptedActor;

class PathPlannerComponent : public TickableComponent
{
	MEMORY_ALLOCATOR(PathPlannerComponent, "Scene");

private:
	Vector3 m_target_position;
	Vector3 m_source_position;
	float m_regenerate_delta;
	CollisionGroup::Type m_collision_group;

	PathHandle m_generating_handle;
	PathHandle m_active_handle;

	Vector3 m_generate_position;

	Vector3 m_movement_vector;

	int m_target_node_index;

	CollisionHandle* m_los_check[3];
	bool m_in_los;
	float m_los_radius;
	int m_los_check_offset;

	bool m_at_target;

	bool m_client_side;

public:
	PathPlannerComponent();
	virtual ~PathPlannerComponent();

	virtual int Get_ID();
	virtual bool Is_Used_In_Demo();

	std::string	Get_Script_Class_Name()
	{
		return "PathPlannerComponent";
	}

	void Regenerate();

	bool Has_Path();

	bool Get_At_Target();

	void Set_Target_Position(Vector3 val);
	Vector3 Get_Target_Position();

	void Set_Source_Position(Vector3 val);
	Vector3 Get_Source_Position();

	void Set_Regenerate_Delta(float val);
	float Get_Regenerate_Delta();

	void Set_Collision_Group(CollisionGroup::Type val);
	CollisionGroup::Type Get_Collision_Group();

	void Set_Client_Side(bool val);
	bool Get_Client_Side();

	bool Get_Target_In_LOS();

	Vector3 Get_Movement_Vector();

	void Tick(const FrameTime& time);

};

#endif

