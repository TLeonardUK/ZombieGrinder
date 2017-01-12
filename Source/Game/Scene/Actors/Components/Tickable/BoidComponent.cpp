// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Tickable/BoidComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/Scene/Boids/BoidManager.h"

BoidComponent::BoidComponent()
	: m_script_object(NULL)
	, m_center(0.0f, 0.0f)
	, m_neighbour_distance(128.0f)
	, m_seperation_distance(64.0f)
	, m_maximum_speed(2.0f)
	, m_maximum_force(0.8f)
	, m_cohesion_weight(1.0f)
	, m_avoidance_weight(1.0f)
	, m_seperation_weight(2.0f)
	, m_boid_enabled(true)
{
	BoidManager::Get()->Register_Boid(this);
	m_registered = true;
}

BoidComponent::~BoidComponent()
{
	m_script_object = NULL;

	BoidManager::Get()->Unregister_Boid(this);
	m_registered = false;

	Scene* scene = GameEngine::Get()->Get_Scene();
	if (scene)
	{
		scene->Remove_Tickable(this);
	}
}

int BoidComponent::Get_ID()
{
	static int hash = StringHelper::Hash("BoidComponent");
	return hash;
}

bool BoidComponent::Is_Used_In_Demo()
{
	return false;
}

void BoidComponent::Collect_Deferred(const FrameTime& time)
{
}

void BoidComponent::Tick(const FrameTime& time)
{
	if (!m_active || !m_boid_enabled)
	{
		if (m_registered)
		{
			BoidManager::Get()->Unregister_Boid(this);
			m_registered = false;
		}
	}
	else
	{
		if (!m_registered)
		{
			BoidManager::Get()->Register_Boid(this);
			m_registered = true;
		}
	}
}

void BoidComponent::Create_Deferred(const FrameTime& time)
{
}