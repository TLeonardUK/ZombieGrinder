// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_PARTICLECOLLECTORCOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_PARTICLECOLLECTORCOMPONENT_

#include "Engine/Scene/Tickable.h"
#include "Engine/Scene/Collision/CollisionManager.h"
#include "Engine/Particles/ParticleManager.h"
#include "Engine/Scene/Pathing/PathManager.h"
#include "Game/Scene/Actors/Components/Tickable/TickableComponent.h"

class ScriptedActor;

class ParticleCollectorComponent : public TickableComponent
{
	MEMORY_ALLOCATOR(ParticleCollectorComponent, "Scene");

private:
	ParticleCollectorHandle m_handle;
	bool	m_paused;
	Vector3 m_position;
	float	m_radius;
	float	m_strength;

public:
	ParticleCollectorComponent();
	virtual ~ParticleCollectorComponent();

	virtual int Get_ID();
	virtual bool Is_Used_In_Demo();

	void Set_Paused(bool val)
	{
		m_paused = val;
	}

	bool Get_Paused()
	{
		return m_paused;
	}

	void Set_Offset(Vector3 val)
	{
		m_position = val;
	}

	Vector3 Get_Offset()
	{
		return m_position;
	}

	void Set_Radius(float val)
	{
		m_radius = val;
	}

	float Get_Radius()
	{
		return m_radius;
	}

	void Set_Strength(float val)
	{
		m_strength = val;
	}

	float Get_Strength()
	{
		return m_strength;
	}

	int Get_Collected(ParticleFX_EmitterCollectionType::Type type)
	{
		return m_handle.Get_Collected(type);
	}

	std::string	Get_Script_Class_Name()
	{
		return "ParticleCollectorComponent";
	}

	void Tick(const FrameTime& time);

};

#endif

