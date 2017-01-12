// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/Components/CRuntime_BoidComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Tickable/BoidComponent.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/GameEngine.h"

#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

CVMObjectHandle CRuntime_BoidComponent::Create(CVirtualMachine* vm, CVMValue self)
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(vm->Get_Active_Context()->MetaData);

	BoidComponent* component = new BoidComponent();
	component->Set_Script_Object(self.object_value);

	actor->Add_Component(component);
	scene->Add_Tickable(component);

	CVMObject* obj = self.object_value.Get();
	obj->Set_Meta_Data(component);

	return obj;
}

void CRuntime_BoidComponent::Set_Center(CVirtualMachine* vm, CVMValue self, CVMObjectHandle val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMObject* obj = val.Get();

	Vector2 c;
	c.X = obj->Get_Slot(0).float_value;
	c.Y = obj->Get_Slot(1).float_value;

	component->Set_Center(c);
}

CVMObjectHandle CRuntime_BoidComponent::Get_Center(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Vector2 area = component->Get_Center();

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	result.Get()->Resize(2);
	result.Get()->Get_Slot(0).float_value = area.X;
	result.Get()->Get_Slot(1).float_value = area.Y;

	return result;
}

void CRuntime_BoidComponent::Set_Neighbour_Distance(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Neighbour_Distance(val);
}

float CRuntime_BoidComponent::Get_Neighbour_Distance(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Neighbour_Distance();
}

void CRuntime_BoidComponent::Set_Seperation_Distance(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Seperation_Distance(val);
}

float CRuntime_BoidComponent::Get_Seperation_Distance(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Seperation_Distance();
}

void CRuntime_BoidComponent::Set_Maximum_Speed(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Maximum_Speed(val);
}

float CRuntime_BoidComponent::Get_Maximum_Speed(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Maximum_Speed();
}

void CRuntime_BoidComponent::Set_Maximum_Force(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Maximum_Force(val);
}

float CRuntime_BoidComponent::Get_Maximum_Force(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Maximum_Force();
}

void CRuntime_BoidComponent::Set_Cohesion_Weight(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Cohesion_Weight(val);
}

float CRuntime_BoidComponent::Get_Cohesion_Weight(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Cohesion_Weight();
}

void CRuntime_BoidComponent::Set_Avoidance_Weight(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Avoidance_Weight(val);
}

float CRuntime_BoidComponent::Get_Avoidance_Weight(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Avoidance_Weight();
}

void CRuntime_BoidComponent::Set_Seperation_Weight(CVirtualMachine* vm, CVMValue self, float val)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Seperation_Weight(val);
}

float CRuntime_BoidComponent::Get_Seperation_Weight(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Seperation_Weight();
}

void CRuntime_BoidComponent::Set_Enabled(CVirtualMachine* vm, CVMValue self, int visible)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Boid_Enabled(visible != 0);
}

int CRuntime_BoidComponent::Get_Enabled(CVirtualMachine* vm, CVMValue self)
{
	BoidComponent* component = reinterpret_cast<BoidComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Boid_Enabled() ? 1 : 0;
}

void CRuntime_BoidComponent::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("BoidComponent", "Create", &Create);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("BoidComponent", "Set_Center", &Set_Center);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("BoidComponent", "Get_Center", &Get_Center);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Neighbour_Distance", &Set_Neighbour_Distance);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Neighbour_Distance", &Get_Neighbour_Distance);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Seperation_Distance", &Set_Seperation_Distance);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Seperation_Distance", &Get_Seperation_Distance);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Maximum_Speed", &Set_Maximum_Speed);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Maximum_Speed", &Get_Maximum_Speed);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Maximum_Force", &Set_Maximum_Force);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Maximum_Force", &Get_Maximum_Force);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Cohesion_Weight", &Set_Cohesion_Weight);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Cohesion_Weight", &Get_Cohesion_Weight);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Avoidance_Weight", &Set_Avoidance_Weight);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Avoidance_Weight", &Get_Avoidance_Weight);
	vm->Get_Bindings()->Bind_Method<void,float>("BoidComponent", "Set_Seperation_Weight", &Set_Seperation_Weight);
	vm->Get_Bindings()->Bind_Method<float>("BoidComponent", "Get_Seperation_Weight", &Get_Seperation_Weight);
	vm->Get_Bindings()->Bind_Method<void, int>("BoidComponent", "Set_Enabled", &Set_Enabled);
	vm->Get_Bindings()->Bind_Method<int>("BoidComponent", "Get_Enabled", &Get_Enabled);
}
