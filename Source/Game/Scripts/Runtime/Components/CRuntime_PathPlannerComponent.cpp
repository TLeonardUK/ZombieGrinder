// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/Components/CRuntime_PathPlannerComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Tickable/PathPlannerComponent.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Game/Scene/GameScene.h"

CVMObjectHandle CRuntime_PathPlannerComponent::Create(CVirtualMachine* vm, CVMValue self)
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(vm->Get_Active_Context()->MetaData);

	PathPlannerComponent* component = new PathPlannerComponent();
	actor->Add_Component(component);
	scene->Add_Tickable(component);

	CVMObject* obj = self.object_value.Get();
	obj->Set_Meta_Data(component);

	return obj;
}

int CRuntime_PathPlannerComponent::Get_Has_Path(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Has_Path();
}

void CRuntime_PathPlannerComponent::Set_Target_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Set_Target_Position(value.Get()->To_Vec3());
}

CVMObjectHandle CRuntime_PathPlannerComponent::Get_Target_Position(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec3(component->Get_Target_Position());
}

void CRuntime_PathPlannerComponent::Set_Source_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Set_Source_Position(value.Get()->To_Vec3());
}

CVMObjectHandle CRuntime_PathPlannerComponent::Get_Source_Position(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec3(component->Get_Source_Position());
}

void CRuntime_PathPlannerComponent::Set_Regenerate_Delta(CVirtualMachine* vm, CVMValue self, float value)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Regenerate_Delta(value);
}

float CRuntime_PathPlannerComponent::Get_Regenerate_Delta(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Regenerate_Delta();
}

void CRuntime_PathPlannerComponent::Set_Collision_Group(CVirtualMachine* vm, CVMValue self, int value)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Collision_Group((CollisionGroup::Type)value);
}

int CRuntime_PathPlannerComponent::Get_Collision_Group(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Collision_Group();
}

void CRuntime_PathPlannerComponent::Set_Client_Side(CVirtualMachine* vm, CVMValue self, int value)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Client_Side(value ? true : false);
}

int CRuntime_PathPlannerComponent::Get_Client_Side(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Client_Side() ? 1 : 0;
}

int CRuntime_PathPlannerComponent::Get_Target_In_LOS(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Target_In_LOS() ? 1 : 0;
}

int CRuntime_PathPlannerComponent::Get_At_Target(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_At_Target() ? 1 : 0;
}

CVMObjectHandle CRuntime_PathPlannerComponent::Get_Movement_Vector(CVirtualMachine* vm, CVMValue self)
{
	PathPlannerComponent* component = reinterpret_cast<PathPlannerComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec3(component->Get_Movement_Vector());
}

void CRuntime_PathPlannerComponent::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("PathPlannerComponent", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<int>("PathPlannerComponent", "Get_Has_Path", &Get_Has_Path);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("PathPlannerComponent", "Set_Target_Position", &Set_Target_Position);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("PathPlannerComponent", "Get_Target_Position", &Get_Target_Position);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("PathPlannerComponent", "Set_Source_Position", &Set_Source_Position);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("PathPlannerComponent", "Get_Source_Position", &Get_Source_Position);
	vm->Get_Bindings()->Bind_Method<void,float>("PathPlannerComponent", "Set_Regenerate_Delta", &Set_Regenerate_Delta);
	vm->Get_Bindings()->Bind_Method<float>("PathPlannerComponent", "Get_Regenerate_Delta", &Get_Regenerate_Delta);
	vm->Get_Bindings()->Bind_Method<void,int>("PathPlannerComponent", "Set_Collision_Group", &Set_Collision_Group);
	vm->Get_Bindings()->Bind_Method<int>("PathPlannerComponent", "Get_Collision_Group", &Get_Collision_Group);
	vm->Get_Bindings()->Bind_Method<void,int>("PathPlannerComponent", "Set_Client_Side", &Set_Client_Side);
	vm->Get_Bindings()->Bind_Method<int>("PathPlannerComponent", "Get_Client_Side", &Get_Client_Side);
	vm->Get_Bindings()->Bind_Method<int>("PathPlannerComponent", "Get_Target_In_LOS", &Get_Target_In_LOS);
	vm->Get_Bindings()->Bind_Method<int>("PathPlannerComponent", "Get_At_Target", &Get_At_Target);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("PathPlannerComponent", "Get_Movement_Vector", &Get_Movement_Vector);
}
