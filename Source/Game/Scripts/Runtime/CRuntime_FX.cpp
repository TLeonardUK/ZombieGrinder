// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_FX.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Particles/ParticleManager.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Engine/Scene/Map/Map.h"

void CRuntime_FX::Spawn_Untracked(CVirtualMachine* vm, CVMValue self, CVMString effect_name, CVMObjectHandle pos, float dir, int layer, CVMObjectHandle instigator, int meta_number, CVMString modifier, int subtype, CVMObjectHandle weapon_type, CVMObjectHandle upgrade_modifiers)
{
	ParticleFXHandle* pfx = ResourceFactory::Get()->Get_ParticleFX(effect_name.C_Str());
	if (pfx == NULL)
	{
		DBG_LOG("Attempted to spawn invalid effect '%s'.", effect_name.C_Str());
		return;
	}

	Vector3 p;
	p.X = pos.Get()->Get_Slot(0).float_value;
	p.Y = pos.Get()->Get_Slot(1).float_value;
	p.Z = (float)layer;

	std::vector<float> upgrade_modifiers_vec;
	for (int i = 0; i < ParticleUpgradeModifiers::COUNT; i++)
	{
		float val = 0.0f;

		if (upgrade_modifiers.Get() != NULL)
		{
			val = upgrade_modifiers.Get()->Get_Slot(i).float_value;
		}

		upgrade_modifiers_vec.push_back(val);
	}

	ParticleEffectHandle handle = ParticleManager::Get()->Spawn(pfx, p, dir, meta_number, modifier.C_Str(), subtype, weapon_type.Get(), upgrade_modifiers_vec);

	if (instigator.Get() != NULL)
	{
		ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(instigator.Get()->Get_Meta_Data());
		actor->Alloc_Particle_Instigator_Handle();
		handle.Set_Instigator(actor->Get_Particle_Instigator_Handle());
	}
}

void CRuntime_FX::Invoke_Game_Trigger(CVirtualMachine* vm, CVMObjectHandle instigator)
{
	if (instigator.Get() != NULL)
	{
		ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(instigator.Get()->Get_Meta_Data());
		ParticleManager::Get()->Invoke_Game_Trigger(actor->Get_Particle_Instigator_Handle());
	}
}

void CRuntime_FX::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void,CVMString,CVMObjectHandle,float,int,CVMObjectHandle,int,CVMString,int,CVMObjectHandle, CVMObjectHandle>("FX", "Spawn_Untracked", &Spawn_Untracked);
	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle>("FX", "Invoke_Game_Trigger", &Invoke_Game_Trigger);
}
