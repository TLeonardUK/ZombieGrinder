// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Tickable/EffectComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/IO/BinaryStream.h"

EffectComponent::EffectComponent()
	: m_visible(false)
	, m_paused(false)
	, m_spawned(false)
	, m_one_shot(true)
	, m_instigator(NULL)
	, m_type(NULL)
	, m_offset(0.0f, 0.0f, 0.0f)
	, m_angle_offset(0.0f, 0.0f, 0.0f)
	, m_modifier("")
	, m_meta_number(0)
	, m_sub_type(0)
	, m_ignore_spawn_collision(false)
{
	Set_Tick_Priority(TickPriority::Final);

	for (int i = 0; i < ParticleUpgradeModifiers::COUNT; i++)
	{
		m_upgrade_modifiers.push_back(0.0f);
	}
}

EffectComponent::~EffectComponent()
{
	m_handle.Dispose();
}

int EffectComponent::Get_ID()
{
	static int hash = StringHelper::Hash("EffectComponent");
	return hash;
}

bool EffectComponent::Is_Used_In_Demo()
{
	return false;
}

ScriptedActor* EffectComponent::Get_Instigator()
{
	return m_instigator;
}

void EffectComponent::Set_Instigator(ScriptedActor* frame)
{
	m_instigator = frame;
	if (frame != NULL)
	{
		frame->Alloc_Particle_Instigator_Handle();
	}
}

int EffectComponent::Get_Meta_Number()
{
	return m_meta_number;
}

void EffectComponent::Set_Meta_Number(int number)
{
	m_meta_number = number;
}

int EffectComponent::Get_Sub_Type()
{
	return m_sub_type;
}

void EffectComponent::Set_Sub_Type(int number)
{
	m_sub_type = number;
}

CVMObjectHandle EffectComponent::Get_Weapon_Type()
{
	return m_weapon_type;
}

void EffectComponent::Set_Weapon_Type(CVMObjectHandle number)
{
	m_weapon_type = number;
}

std::string EffectComponent::Get_Modifier()
{
	return m_modifier;
}

void  EffectComponent::Set_Modifier(std::string number)
{
	m_modifier = number;
}

ParticleFXHandle* EffectComponent::Get_Effect_Type()
{
	return m_type;
}

void EffectComponent::Set_Effect_Type(ParticleFXHandle* frame)
{
	m_type = frame;
	Restart();
}

std::vector<float> EffectComponent::Get_Upgrade_Modifiers()
{
	return m_upgrade_modifiers;
}

void EffectComponent::Set_Upgrade_Modifiers(std::vector<float> frame)
{
	m_upgrade_modifiers = frame;
}

void EffectComponent::Set_Visible(bool value)
{
	m_visible = value;
}

bool EffectComponent::Get_Visible()
{
	return m_visible;
}

void EffectComponent::Set_Paused(bool value)
{
	m_paused = value;
}

bool EffectComponent::Get_Paused()
{
	return m_paused;
}

void EffectComponent::Set_Ignore_Spawn_Collision(bool value)
{
	m_ignore_spawn_collision = value;
}

bool EffectComponent::Get_Ignore_Spawn_Collision()
{
	return m_ignore_spawn_collision;
}
void EffectComponent::Set_One_Shot(bool value)
{
	m_one_shot = value;
}

bool EffectComponent::Get_One_Shot()
{
	return m_one_shot;
}

void EffectComponent::Set_Offset(Vector3 value)
{
	m_offset = value;
}

Vector3 EffectComponent::Get_Offset()
{
	return m_offset;
}

void EffectComponent::Set_Angle_Offset(Vector3 value)
{
	m_angle_offset = value;
}

Vector3 EffectComponent::Get_Angle_Offset()
{
	return m_angle_offset;
}

bool EffectComponent::Get_Finished()
{
	return m_type == NULL || (m_spawned && m_handle.Is_Finished());
}

void EffectComponent::Restart()
{
	m_spawned = false;
}

void EffectComponent::Tick(const FrameTime& time)
{
	if (!m_active)
	{
		if (m_spawned)
		{
			m_handle.Dispose();
			m_spawned = false;
		}
		return;
	}

	ParticleManager* manager = ParticleManager::Get();

	Vector3 pos = m_parent->Get_Position() + m_offset;
 	float dir = m_parent->Get_Rotation().Z + m_angle_offset.Z;

	pos.Z = (float)m_parent->Get_Layer();

	if (m_type != NULL)
	{
		if (!m_spawned || (m_handle.Is_Finished() && m_one_shot == false))
		{
			m_handle.Dispose();
			m_handle = manager->Spawn(m_type, pos, dir);
			m_spawned = true;
		}

		if (!m_handle.Is_Finished())
		{
			m_handle.Set_Position(pos);
			m_handle.Set_Direction(dir);
			m_handle.Set_Instigator(m_instigator == NULL ? NULL : m_instigator->Get_Particle_Instigator_Handle());
			m_handle.Set_Paused(m_paused);
			m_handle.Set_Ignore_Spawn_Collision(m_ignore_spawn_collision);
			m_handle.Set_Visible(m_visible);
			m_handle.Set_Meta_Number(m_meta_number);
			m_handle.Set_Modifier(m_modifier);
			m_handle.Set_Sub_Type(m_sub_type);
			m_handle.Set_Weapon_Type(m_weapon_type.Get());
			m_handle.Set_Upgrade_Modifiers(m_upgrade_modifiers);
		}
	}
	else
	{
		m_handle.Dispose();
	}
}