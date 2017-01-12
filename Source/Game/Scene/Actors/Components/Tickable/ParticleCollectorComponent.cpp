// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Tickable/ParticleCollectorComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/IO/BinaryStream.h"

ParticleCollectorComponent::ParticleCollectorComponent()
	: m_paused(false)
	, m_position(0, 0, 0)
	, m_radius(48.0f)
	, m_strength(100.0f)
	, m_handle()
{
}

ParticleCollectorComponent::~ParticleCollectorComponent()
{
	m_handle.Dispose();
}

int ParticleCollectorComponent::Get_ID()
{
	static int hash = StringHelper::Hash("ParticleCollectorComponent");
	return hash;
}

bool ParticleCollectorComponent::Is_Used_In_Demo()
{
	return false;
}

void ParticleCollectorComponent::Tick(const FrameTime& time)
{	
	if (m_handle.Is_Finished())
	{
		m_handle = ParticleManager::Get()->Create_Collector(m_position, m_radius, m_strength);
	}

	m_handle.Set_Position(Get_Parent()->Get_Position() + m_position);
	m_handle.Set_Paused(m_paused);
}