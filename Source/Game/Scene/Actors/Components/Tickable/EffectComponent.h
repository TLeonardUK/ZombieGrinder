// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_EFFECTCOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_EFFECTCOMPONENT_

#include "Engine/Scene/Tickable.h"

#include "Engine/Particles/ParticleFXHandle.h"
#include "Engine/Particles/ParticleManager.h"

#include "Game/Scene/Actors/Components/Tickable/TickableComponent.h"
#include "Engine/Scene/Animation.h"

#include "XScript/VirtualMachine/CVMObject.h"

class ScriptedActor;

class EffectComponent : public TickableComponent
{
	MEMORY_ALLOCATOR(EffectComponent, "Scene");

private:
	ParticleFXHandle*		m_type;
	ParticleEffectHandle	m_handle;
	bool					m_visible;
	bool					m_paused;
	bool					m_ignore_spawn_collision;
	bool					m_spawned;
	bool					m_one_shot;
	int						m_meta_number;
	int						m_sub_type;
	std::string				m_modifier;
	ScriptedActor*			m_instigator;
	Vector3					m_offset;
	Vector3					m_angle_offset;
	CVMObjectHandle			m_weapon_type;
	std::vector<float>		m_upgrade_modifiers;

public:
	EffectComponent();
	virtual ~EffectComponent();

	std::string	Get_Script_Class_Name()
	{
		return "EffectComponent";
	}

	virtual int Get_ID();
	virtual bool Is_Used_In_Demo();

	int					Get_Meta_Number		();
	void				Set_Meta_Number		(int number);

	int					Get_Sub_Type		();
	void				Set_Sub_Type		(int number);

	CVMObjectHandle		Get_Weapon_Type();
	void				Set_Weapon_Type(CVMObjectHandle number);

	std::string			Get_Modifier		();
	void				Set_Modifier		(std::string number);

	ScriptedActor*		Get_Instigator		();
	void				Set_Instigator		(ScriptedActor* frame);

	ParticleFXHandle*	Get_Effect_Type		();
	void				Set_Effect_Type		(ParticleFXHandle* frame);

	std::vector<float>	Get_Upgrade_Modifiers();
	void				Set_Upgrade_Modifiers(std::vector<float> frame);

	void				Set_Visible			(bool value);
	bool				Get_Visible			();

	void				Set_Paused			(bool value);
	bool				Get_Paused			();

	void				Set_Ignore_Spawn_Collision			(bool value);
	bool				Get_Ignore_Spawn_Collision			();

	void				Set_One_Shot		(bool value);
	bool				Get_One_Shot		();

	void				Set_Offset			(Vector3 value);
	Vector3				Get_Offset			();

	void				Set_Angle_Offset			(Vector3 value);
	Vector3				Get_Angle_Offset			();

	bool				Get_Finished		();

	void				Restart				();

	void				Tick				(const FrameTime& time);

};

#endif

