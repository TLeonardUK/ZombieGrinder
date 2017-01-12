// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/Components/CRuntime_ParticleCollectorComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Tickable/ParticleCollectorComponent.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Particles/ParticleFXHandle.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

CVMObjectHandle CRuntime_ParticleCollectorComponent::Create(CVirtualMachine* vm, CVMValue self)
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(vm->Get_Active_Context()->MetaData);

	ParticleCollectorComponent* component = new ParticleCollectorComponent();
	actor->Add_Component(component);
	scene->Add_Tickable(component);

	CVMObject* obj = self.object_value.Get();
	obj->Set_Meta_Data(component);

	return obj;
}

void CRuntime_ParticleCollectorComponent::Set_Is_Paused(CVirtualMachine* vm, CVMValue self, int val)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Paused(!!val);
}

int CRuntime_ParticleCollectorComponent::Get_Is_Paused(CVirtualMachine* vm, CVMValue self)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Paused();
}

void CRuntime_ParticleCollectorComponent::Set_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Offset(val.Get()->To_Vec3());
}

CVMObjectHandle CRuntime_ParticleCollectorComponent::Get_Offset(CVirtualMachine* vm, CVMValue self)
{	
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec3(component->Get_Offset());
}

void CRuntime_ParticleCollectorComponent::Set_Radius(CVirtualMachine* vm, CVMValue self, float val)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Radius(val);
}

float CRuntime_ParticleCollectorComponent::Get_Radius(CVirtualMachine* vm, CVMValue self)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Radius();
}

void CRuntime_ParticleCollectorComponent::Set_Strength(CVirtualMachine* vm, CVMValue self, float val)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Strength(val);
}

float CRuntime_ParticleCollectorComponent::Get_Strength(CVirtualMachine* vm, CVMValue self)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Strength();
}

int CRuntime_ParticleCollectorComponent::Get_Collected(CVirtualMachine* vm, CVMValue self, int type)
{
	ParticleCollectorComponent* component = reinterpret_cast<ParticleCollectorComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Collected((ParticleFX_EmitterCollectionType::Type)type);
}

void CRuntime_ParticleCollectorComponent::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("ParticleCollectorComponent", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<void,int>("ParticleCollectorComponent", "Set_Is_Paused", &Set_Is_Paused);
	vm->Get_Bindings()->Bind_Method<int>("ParticleCollectorComponent", "Get_Is_Paused", &Get_Is_Paused);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("ParticleCollectorComponent", "Set_Offset", &Set_Offset);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("ParticleCollectorComponent", "Get_Offset", &Get_Offset);
	vm->Get_Bindings()->Bind_Method<void,float>("ParticleCollectorComponent", "Set_Radius", &Set_Radius);
	vm->Get_Bindings()->Bind_Method<float>("ParticleCollectorComponent", "Get_Radius", &Get_Radius);
	vm->Get_Bindings()->Bind_Method<void,float>("ParticleCollectorComponent", "Set_Strength", &Set_Strength);
	vm->Get_Bindings()->Bind_Method<float>("ParticleCollectorComponent", "Get_Strength", &Get_Strength);
	vm->Get_Bindings()->Bind_Method<int,int>("ParticleCollectorComponent", "Get_Collected", &Get_Collected);
}
