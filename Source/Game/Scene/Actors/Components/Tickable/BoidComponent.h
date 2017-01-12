// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_BOIDCOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_BOIDCOMPONENT_

#include "Game/Scene/Actors/Components/Drawable/DrawableComponent.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

struct AtlasFrame;
struct AtlasAnimation;
struct CollisionHandle;
class ScriptedActor;

class BoidComponent : public Component, public Tickable
{
	MEMORY_ALLOCATOR(BoidComponent, "Scene");

private:
	CVMGCRoot m_script_object;

	Vector2 m_center;
	float m_neighbour_distance;
	float m_seperation_distance;
	float m_maximum_speed;
	float m_maximum_force;
	float m_cohesion_weight;
	float m_avoidance_weight;
	float m_seperation_weight;

	bool m_boid_enabled;

	bool m_registered;

public:
	BoidComponent();
	virtual ~BoidComponent();

	virtual int Get_ID();
	virtual bool Is_Used_In_Demo();

	void Set_Script_Object(CVMObjectHandle handle)
	{
		m_script_object = handle;
	}

	std::string	Get_Script_Class_Name()
	{
		return "CollisionComponent";
	}

	void Set_Boid_Enabled(bool bEnabled)
	{
		m_boid_enabled = bEnabled;
	}

	bool Get_Boid_Enabled()
	{
		return m_boid_enabled;
	}

	void Set_Center(Vector2 val)
	{
		m_center = val;
	}

	Vector2 Get_Center()
	{
		return m_center;
	}

	void Set_Neighbour_Distance(float val)
	{
		m_neighbour_distance = val;
	}

	float Get_Neighbour_Distance()
	{
		return m_neighbour_distance;
	}

	void Set_Seperation_Distance(float val)
	{
		m_seperation_distance = val;
	}

	float Get_Seperation_Distance()
	{
		return m_seperation_distance;
	}

	void Set_Maximum_Speed(float val)
	{
		m_maximum_speed = val;
	}

	float Get_Maximum_Speed()
	{
		return m_maximum_speed;
	}

	void Set_Maximum_Force(float val)
	{
		m_maximum_force = val;
	}

	float Get_Maximum_Force()
	{
		return m_maximum_force;
	}

	void Set_Cohesion_Weight(float val)
	{
		m_cohesion_weight = val;
	}

	float Get_Cohesion_Weight()
	{
		return m_cohesion_weight;
	}

	void Set_Avoidance_Weight(float val)
	{
		m_avoidance_weight = val;
	}

	float Get_Avoidance_Weight()
	{
		return m_avoidance_weight;
	}

	void Set_Seperation_Weight(float val)
	{
		m_seperation_weight = val;
	}

	float Get_Seperation_Weight()
	{
		return m_seperation_weight;
	}

	void Tick(const FrameTime& time);
	void Collect_Deferred(const FrameTime& time);
	void Create_Deferred(const FrameTime& time);

};

#endif

