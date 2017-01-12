// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/Components/CRuntime_EffectComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Tickable/EffectComponent.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Particles/ParticleFXHandle.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

CVMObjectHandle CRuntime_EffectComponent::Create(CVirtualMachine* vm, CVMValue self)
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(vm->Get_Active_Context()->MetaData);

	EffectComponent* component = new EffectComponent();
	actor->Add_Component(component);
	scene->Add_Tickable(component);

	CVMObject* obj = self.object_value.Get();
	obj->Set_Meta_Data(component);

	return obj;
}

void CRuntime_EffectComponent::Set_Visible(CVirtualMachine* vm, CVMValue self, int val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Visible(val != 0);
}

int CRuntime_EffectComponent::Get_Visible(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Visible();
}

void CRuntime_EffectComponent::Set_Paused(CVirtualMachine* vm, CVMValue self, int val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Paused(val != 0);
}

int CRuntime_EffectComponent::Get_Paused(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Paused();
}

void CRuntime_EffectComponent::Set_Ignore_Spawn_Collision(CVirtualMachine* vm, CVMValue self, int val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Ignore_Spawn_Collision(val != 0);
}

int CRuntime_EffectComponent::Get_Ignore_Spawn_Collision(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Ignore_Spawn_Collision();
}

void CRuntime_EffectComponent::Set_One_Shot(CVirtualMachine* vm, CVMValue self, int val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_One_Shot(val != 0);
}

int CRuntime_EffectComponent::Get_One_Shot(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_One_Shot();
}

void CRuntime_EffectComponent::Set_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Vector3 offset(
		val.Get()->Get_Slot(0).float_value,
		val.Get()->Get_Slot(1).float_value,
		val.Get()->Get_Slot(2).float_value
	);
	component->Set_Offset(offset);
}

CVMObjectHandle CRuntime_EffectComponent::Get_Offset(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Vector3 offset = component->Get_Offset();
	return vm->Create_Vec3(offset.X, offset.Y, offset.Z);
}

void CRuntime_EffectComponent::Set_Angle_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Vector3 offset(
		val.Get()->Get_Slot(0).float_value,
		val.Get()->Get_Slot(1).float_value,
		val.Get()->Get_Slot(2).float_value
		);
	component->Set_Angle_Offset(offset);
}

CVMObjectHandle CRuntime_EffectComponent::Get_Angle_Offset(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Vector3 offset = component->Get_Angle_Offset();
	return vm->Create_Vec3(offset.X, offset.Y, offset.Z);
}


int CRuntime_EffectComponent::Get_Finished(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Finished();
}

void CRuntime_EffectComponent::Set_Effect_Name(CVirtualMachine* vm, CVMValue self, CVMString val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Set_Effect_Type(ResourceFactory::Get()->Get_ParticleFX(val.C_Str()));
}

CVMString CRuntime_EffectComponent::Get_Effect_Name(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ParticleFXHandle* frame = component->Get_Effect_Type();
	return frame != NULL ? frame->Get()->Get_Name().c_str() : "";
}

void CRuntime_EffectComponent::Set_Instigator(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	if (val.Get() != NULL)
	{
		ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(val.Get()->Get_Meta_Data());	
		component->Set_Instigator(actor);
	}
	else
	{
		component->Set_Instigator(NULL);
	}
}

CVMObjectHandle CRuntime_EffectComponent::Get_Instigator(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ScriptedActor* actor = component->Get_Instigator();
	return actor != NULL ? actor->Get_Script_Object() : NULL;
}

void CRuntime_EffectComponent::Set_Modifier(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Modifier(value.C_Str());
}

CVMString CRuntime_EffectComponent::Get_Modifier(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Modifier().c_str();
}

void CRuntime_EffectComponent::Set_Meta_Number(CVirtualMachine* vm, CVMValue self, int value)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Meta_Number(value);
}

int CRuntime_EffectComponent::Get_Meta_Number(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Meta_Number();
}

void CRuntime_EffectComponent::Set_Sub_Type(CVirtualMachine* vm, CVMValue self, int value)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Sub_Type(value);
}

int CRuntime_EffectComponent::Get_Sub_Type(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Sub_Type();
}

void CRuntime_EffectComponent::Set_Weapon_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Weapon_Type(value);
}

CVMObjectHandle CRuntime_EffectComponent::Get_Weapon_Type(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Weapon_Type();
}

void CRuntime_EffectComponent::Set_Upgrade_Modifiers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	std::vector<float> upgrade_modifiers_vec;
	for (int i = 0; i < ParticleUpgradeModifiers::COUNT; i++)
	{
		float val = 0.0f;

		if (value.Get() != NULL)
		{
			val = value.Get()->Get_Slot(i).float_value;
		}

		upgrade_modifiers_vec.push_back(val);
	}

	component->Set_Upgrade_Modifiers(upgrade_modifiers_vec);
}

CVMObjectHandle CRuntime_EffectComponent::Get_Upgrade_Modifiers(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	std::vector<float> upgrade_modifiers_vec = component->Get_Upgrade_Modifiers();

	CVMObjectHandle handle = vm->New_Array(vm->FloatArray_Class, ParticleUpgradeModifiers::COUNT);

	for (int i = 0; i < ParticleUpgradeModifiers::COUNT; i++)
	{
		handle.Get()->Get_Slot(i).float_value = upgrade_modifiers_vec[i];
	}

	return handle;
}

void CRuntime_EffectComponent::Restart(CVirtualMachine* vm, CVMValue self)
{
	EffectComponent* component = reinterpret_cast<EffectComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Restart();
}

void CRuntime_EffectComponent::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("EffectComponent", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<void,int>("EffectComponent", "Set_Visible", &Set_Visible);
	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_Visible", &Get_Visible);
	vm->Get_Bindings()->Bind_Method<void,int>("EffectComponent", "Set_Paused", &Set_Paused);
	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_Paused", &Get_Paused);
	vm->Get_Bindings()->Bind_Method<void,int>("EffectComponent", "Set_One_Shot", &Set_One_Shot);
	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_One_Shot", &Get_One_Shot);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("EffectComponent", "Set_Offset", &Set_Offset);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("EffectComponent", "Get_Offset", &Get_Offset);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("EffectComponent", "Set_Angle_Offset", &Set_Angle_Offset);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("EffectComponent", "Get_Angle_Offset", &Get_Angle_Offset);

	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_Finished", &Get_Finished);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("EffectComponent", "Set_Effect_Name", &Set_Effect_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>("EffectComponent", "Get_Effect_Name", &Get_Effect_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("EffectComponent", "Set_Instigator", &Set_Instigator);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("EffectComponent", "Get_Instigator", &Get_Instigator);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("EffectComponent", "Set_Modifier", &Set_Modifier);
	vm->Get_Bindings()->Bind_Method<CVMString>("EffectComponent", "Get_Modifier", &Get_Modifier);
	vm->Get_Bindings()->Bind_Method<void,int>("EffectComponent", "Set_Meta_Number", &Set_Meta_Number);
	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_Meta_Number", &Get_Meta_Number);
	
	vm->Get_Bindings()->Bind_Method<void,int>("EffectComponent", "Set_Sub_Type", &Set_Sub_Type);
	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_Sub_Type", &Get_Sub_Type);

	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("EffectComponent", "Set_Weapon_Type", &Set_Weapon_Type);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("EffectComponent", "Get_Weapon_Type", &Get_Weapon_Type);

	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("EffectComponent", "Set_Upgrade_Modifiers", &Set_Upgrade_Modifiers);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("EffectComponent", "Get_Upgrade_Modifiers", &Get_Upgrade_Modifiers);

	vm->Get_Bindings()->Bind_Method<void,int>("EffectComponent", "Set_Ignore_Spawn_Collision", &Set_Ignore_Spawn_Collision);
	vm->Get_Bindings()->Bind_Method<int>("EffectComponent", "Get_Ignore_Spawn_Collision", &Get_Ignore_Spawn_Collision);

	vm->Get_Bindings()->Bind_Method<void>("EffectComponent", "Restart", &Restart);
}
