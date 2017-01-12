// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/Components/CRuntime_CollisionComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Collision/CollisionComponent.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

CVMObjectHandle CRuntime_CollisionComponent::Create(CVirtualMachine* vm, CVMValue self)
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(vm->Get_Active_Context()->MetaData);

	CollisionComponent* component = new CollisionComponent();
	component->Set_Script_Object(self.object_value);

	actor->Add_Component(component);
	scene->Add_Tickable(component);

	CVMObject* obj = self.object_value.Get();
	obj->Set_Meta_Data(component);

	return obj;
}

void CRuntime_CollisionComponent::Set_Enabled(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Enabled(visible != 0);
}

int CRuntime_CollisionComponent::Get_Enabled(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Enabled() ? 1 : 0;
}

void CRuntime_CollisionComponent::Set_Blocks_Path(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Blocks_Path(visible != 0);
}

int CRuntime_CollisionComponent::Get_Blocks_Path(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Blocks_Path() ? 1 : 0;
}

void CRuntime_CollisionComponent::Set_Blocks_Path_Spawns(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Blocks_Path_Spawns(visible != 0);
}

int CRuntime_CollisionComponent::Get_Blocks_Path_Spawns(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Blocks_Path_Spawns() ? 1 : 0;
}

void CRuntime_CollisionComponent::Set_Smoothed(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Smoothed(visible != 0);
}

int CRuntime_CollisionComponent::Get_Smoothed(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Smoothed() ? 1 : 0;
}

void CRuntime_CollisionComponent::Set_Shape(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Shape((CollisionShape::Type)visible);
}

int CRuntime_CollisionComponent::Get_Shape(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Shape();
}

void CRuntime_CollisionComponent::Set_Type(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Type((CollisionType::Type)visible);
}

int CRuntime_CollisionComponent::Get_Type(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Type();
}

void CRuntime_CollisionComponent::Set_Group(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Group((CollisionGroup::Type)visible);
}

int CRuntime_CollisionComponent::Get_Group(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Group();
}

void CRuntime_CollisionComponent::Set_Collides_With(CVirtualMachine* vm, CVMValue self, int visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	component->Set_Collides_With((CollisionGroup::Type)visible);
}

int CRuntime_CollisionComponent::Get_Collides_With(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Get_Collides_With();
}

void CRuntime_CollisionComponent::Set_Area(CVirtualMachine* vm, CVMValue self, CVMObjectHandle visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	CVMObject* obj = visible.Get();
	
	Rect2D c;
	c.X = obj->Get_Slot(0).float_value;
	c.Y = obj->Get_Slot(1).float_value;
	c.Width = obj->Get_Slot(2).float_value;
	c.Height = obj->Get_Slot(3).float_value;

	component->Set_Area(c);
}

CVMObjectHandle CRuntime_CollisionComponent::Get_Area(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Rect2D area = component->Get_Area();

	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	result.Get()->Resize(4);
	result.Get()->Get_Slot(0).float_value = area.X;
	result.Get()->Get_Slot(1).float_value = area.Y;
	result.Get()->Get_Slot(2).float_value = area.Width;
	result.Get()->Get_Slot(3).float_value = area.Height;

	return result;
}

void CRuntime_CollisionComponent::Set_Velocity(CVirtualMachine* vm, CVMValue self, CVMObjectHandle visible)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMObject* obj = visible.Get();
	component->Set_Velocity(obj->To_Vec2());
}

CVMObjectHandle CRuntime_CollisionComponent::Get_Velocity(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return vm->Create_Vec2(component->Get_Velocity());
}


int CRuntime_CollisionComponent::Is_Colliding(CVirtualMachine* vm, CVMValue self)
{
	CollisionComponent* component = reinterpret_cast<CollisionComponent*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return component->Is_Colliding() ? 1 : 0;
}

void CRuntime_CollisionComponent::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("CollisionComponent", "Create",	&Create);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Shape", &Set_Shape);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Shape", &Get_Shape);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Type",	&Set_Type);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Type",	&Get_Type);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Enabled",	&Set_Enabled);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Enabled",	&Get_Enabled);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Blocks_Paths",	&Set_Blocks_Path);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Blocks_Paths",	&Get_Blocks_Path);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Blocks_Path_Spawns",	&Set_Blocks_Path_Spawns);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Blocks_Path_Spawns",	&Get_Blocks_Path_Spawns);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Group",	&Set_Group);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Group",	&Get_Group);
	vm->Get_Bindings()->Bind_Method<void, int>				("CollisionComponent", "Set_Collides_With",	&Set_Collides_With);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Get_Collides_With",	&Get_Collides_With);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("CollisionComponent", "Set_Area",	&Set_Area);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("CollisionComponent", "Get_Area",	&Get_Area);
	vm->Get_Bindings()->Bind_Method<void, int>("CollisionComponent", "Set_Smoothed", &Set_Smoothed);
	vm->Get_Bindings()->Bind_Method<int>("CollisionComponent", "Get_Smoothed", &Get_Smoothed);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("CollisionComponent", "Set_Velocity",	&Set_Velocity);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("CollisionComponent", "Get_Velocity",	&Get_Velocity);
	vm->Get_Bindings()->Bind_Method<int>					("CollisionComponent", "Is_Colliding",	&Is_Colliding);
}

