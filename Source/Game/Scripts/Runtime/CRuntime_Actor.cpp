// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Actor.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Scene/Actors/Components/Collision/CollisionComponent.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/GameNetUser.h"

void CRuntime_Actor::Hibernate(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Hibernate();
}

void CRuntime_Actor::Wake_Up(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->WakeUp();
}

int CRuntime_Actor::Is_Hibernating(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Is_Hibernating() ? 1 : 0;
}

void CRuntime_Actor::Set_Tag(CVirtualMachine* vm, CVMValue self, CVMString pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Tag(pos.C_Str());
}

CVMString CRuntime_Actor::Get_Tag(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Tag().c_str();
}

void CRuntime_Actor::Set_Link(CVirtualMachine* vm, CVMValue self, CVMString pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Link(pos.C_Str());
}

CVMString CRuntime_Actor::Get_Link(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Link().c_str();
}

void CRuntime_Actor::Set_Enabled(CVirtualMachine* vm, CVMValue self, int pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Enabled(pos != 0);
}

int CRuntime_Actor::Get_Enabled(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Enabled() ? 1 : 0;
}

void CRuntime_Actor::Set_Activated_By_Player(CVirtualMachine* vm, CVMValue self, int pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Activated_By_Player(pos != 0);
}

int CRuntime_Actor::Get_Activated_By_Player(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Activated_By_Player() ? 1 : 0;
}

int CRuntime_Actor::Get_Net_ID(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated ? actor->Get_Replication_Info().unique_id : -1;
}

int CRuntime_Actor::Get_Unique_ID(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Map_ID();
}

void CRuntime_Actor::Set_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMObject* obj = pos.Get();
	actor->Set_Position(Vector3(
			obj->Get_Slot(0).float_value,
			obj->Get_Slot(1).float_value,
			obj->Get_Slot(2).float_value));
}

CVMObjectHandle CRuntime_Actor::Get_Position(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec3_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Vector3 position = actor->Get_Position();

	CVMObjectHandle result = vm->New_Object(vec3_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(3);
	obj->Get_Slot(0).float_value = position.X;
	obj->Get_Slot(1).float_value = position.Y;
	obj->Get_Slot(2).float_value = position.Z;

 	return result;
}

void CRuntime_Actor::Set_Rotation(CVirtualMachine* vm, CVMValue self, float pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Rotation(Vector3(0, 0, pos));
}

float CRuntime_Actor::Get_Rotation(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Rotation().Z;
}

void CRuntime_Actor::Set_Scale(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{	
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMObject* obj = pos.Get();
	actor->Set_Scale(Vector3(
			obj->Get_Slot(0).float_value,
			obj->Get_Slot(1).float_value,
			1.0f));
}

CVMObjectHandle CRuntime_Actor::Get_Scale(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec2_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Vector3 scale = actor->Get_Scale();

	CVMObjectHandle result = vm->New_Object(vec2_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(2);
	obj->Get_Slot(0).float_value = scale.X;
	obj->Get_Slot(1).float_value = scale.Y;

	return result;
}

void CRuntime_Actor::Set_Layer(CVirtualMachine* vm, CVMValue self, int pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Layer(pos);
}

int CRuntime_Actor::Get_Layer(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Layer();
}

void CRuntime_Actor::Set_Depth_Bias(CVirtualMachine* vm, CVMValue self, float pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Depth_Bias(pos);
}

float CRuntime_Actor::Get_Depth_Bias(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (float)actor->Get_Depth_Bias();
}

void CRuntime_Actor::Set_Depth_Y_Offset(CVirtualMachine* vm, CVMValue self, float pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Depth_Y_Offset(pos);
}

float CRuntime_Actor::Get_Depth_Y_Offset(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (float)actor->Get_Depth_Y_Offset();
}

void CRuntime_Actor::Set_Bounding_Box(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMObject* obj = pos.Get();
	actor->Set_Bounding_Box(Rect2D(
			obj->Get_Slot(0).float_value,
			obj->Get_Slot(1).float_value,
			obj->Get_Slot(2).float_value,
			obj->Get_Slot(3).float_value));
}

CVMObjectHandle CRuntime_Actor::Get_Bounding_Box(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Rect2D bb = actor->Get_Bounding_Box();

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(4);
	obj->Get_Slot(0).float_value = bb.X;
	obj->Get_Slot(1).float_value = bb.Y;
	obj->Get_Slot(2).float_value = bb.Width;
	obj->Get_Slot(3).float_value = bb.Height;

	return result;
}

CVMObjectHandle CRuntime_Actor::Get_World_Bounding_Box(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Rect2D bb = actor->Get_Bounding_Box();
	Vector3 pos = actor->Get_Position();

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(4);
	obj->Get_Slot(0).float_value = pos.X + bb.X;
	obj->Get_Slot(1).float_value = pos.Y + bb.Y;
	obj->Get_Slot(2).float_value = bb.Width;
	obj->Get_Slot(3).float_value = bb.Height;

	return result;
}
/*
CVMObjectHandle CRuntime_Actor::Get_Screen_Bounding_Box(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Rect2D bb = actor->Get_Screen_Bounding_Box();

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(4);
	obj->Get_Slot(0).float_value = bb.X;
	obj->Get_Slot(1).float_value = bb.Y;
	obj->Get_Slot(2).float_value = bb.Width;
	obj->Get_Slot(3).float_value = bb.Height;

	return result;
}
*/
CVMObjectHandle CRuntime_Actor::Get_Center(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Vector3 pos = actor->Get_World_Center();

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(3);
	obj->Get_Slot(0).float_value = pos.X;
	obj->Get_Slot(1).float_value = pos.Y;
	obj->Get_Slot(2).float_value = pos.Z;

	return result;
}

CVMObjectHandle CRuntime_Actor::Get_Collision_Center(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CollisionComponent* component = actor->Get_Component<CollisionComponent*>();
	if (component != NULL)
	{
		Rect2D area = component->Get_Area();
		Vector3 pos = actor->Get_Position() + Vector3(area.X + (area.Width * 0.5f), area.Y + (area.Height * 0.5f), 0.0f);

		CVMObjectHandle result = vm->New_Object(vec4_class, false);
		CVMObject* obj = result.Get();
		obj->Resize(3);
		obj->Get_Slot(0).float_value = pos.X;
		obj->Get_Slot(1).float_value = pos.Y;
		obj->Get_Slot(2).float_value = pos.Z;

		return result;
	}
	else
	{
		Vector3 pos = actor->Get_World_Center();
		CVMObjectHandle result = vm->New_Object(vec4_class, false);
		CVMObject* obj = result.Get();
		obj->Resize(3);
		obj->Get_Slot(0).float_value = pos.X;
		obj->Get_Slot(1).float_value = pos.Y;
		obj->Get_Slot(2).float_value = pos.Z;

		return result;
	}
}

CVMObjectHandle CRuntime_Actor::Get_Collision_Box(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CollisionComponent* component = actor->Get_Component<CollisionComponent*>();
	if (component != NULL)
	{
		Rect2D area = component->Get_Area();

		CVMObjectHandle result = vm->New_Object(vec4_class, false);
		CVMObject* obj = result.Get();
		obj->Resize(4);
		obj->Get_Slot(0).float_value = component->Get_Area().X;
		obj->Get_Slot(1).float_value = component->Get_Area().Y;
		obj->Get_Slot(2).float_value = component->Get_Area().Width;
		obj->Get_Slot(3).float_value = component->Get_Area().Height;

		return result;
	}
	else
	{
		Vector3 pos = actor->Get_World_Center();
		CVMObjectHandle result = vm->New_Object(vec4_class, false);
		CVMObject* obj = result.Get();
		obj->Resize(4);
		obj->Get_Slot(0).float_value = actor->Get_Bounding_Box().X;
		obj->Get_Slot(1).float_value = actor->Get_Bounding_Box().Y;
		obj->Get_Slot(2).float_value = actor->Get_Bounding_Box().Width;
		obj->Get_Slot(3).float_value = actor->Get_Bounding_Box().Height;

		return result;
	}
}

CVMObjectHandle CRuntime_Actor::Get_World_Collision_Box(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CollisionComponent* component = actor->Get_Component<CollisionComponent*>();
	if (component != NULL)
	{
		Rect2D area = component->Get_Area();

		CVMObjectHandle result = vm->New_Object(vec4_class, false);
		CVMObject* obj = result.Get();
		obj->Resize(4);
		obj->Get_Slot(0).float_value = actor->Get_Position().X + component->Get_Area().X;
		obj->Get_Slot(1).float_value = actor->Get_Position().Y + component->Get_Area().Y;
		obj->Get_Slot(2).float_value = component->Get_Area().Width;
		obj->Get_Slot(3).float_value = component->Get_Area().Height;

		return result;
	}
	else
	{
		Vector3 pos = actor->Get_World_Center();
		CVMObjectHandle result = vm->New_Object(vec4_class, false);
		CVMObject* obj = result.Get();
		obj->Resize(4);
		obj->Get_Slot(0).float_value = actor->Get_Position().X + actor->Get_Bounding_Box().X;
		obj->Get_Slot(1).float_value = actor->Get_Position().Y + actor->Get_Bounding_Box().Y;
		obj->Get_Slot(2).float_value = actor->Get_Bounding_Box().Width;
		obj->Get_Slot(3).float_value = actor->Get_Bounding_Box().Height;

		return result;
	}
}

void CRuntime_Actor::Set_Owner(CVirtualMachine* vm, CVMValue self, CVMObjectHandle owner)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	GameNetUser* netuser = reinterpret_cast<GameNetUser*>(owner.Get()->Get_Meta_Data());	
	actor->Set_Owner(netuser);
}

CVMObjectHandle CRuntime_Actor::Get_Owner(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	GameNetUser* newuser = actor->Get_Owner();
	
	if (newuser != NULL)
	{	
		return newuser->Get_Script_Object();
	//	CVMLinkedSymbol* ret_type = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	//	return vm->New_Object(ret_type, false, actor->Get_Owner());
	}
	else
	{
		return NULL;
	}
}

void CRuntime_Actor::Set_Parent(CVirtualMachine* vm, CVMValue self, CVMObjectHandle owner)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ScriptedActor* parent = owner.Get() == NULL ? NULL : reinterpret_cast<ScriptedActor*>(owner.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	actor->Set_Parent(parent);
}

CVMObjectHandle CRuntime_Actor::Get_Parent(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ScriptedActor* parent = actor->Get_Parent();

	if (parent != NULL)
	{	
		return parent->Get_Script_Object();
	}	
	else
	{
		return NULL;
	}
}

void CRuntime_Actor::MoveTo(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMObject* obj = pos.Get();
	actor->MoveTo(Vector3(
			obj->Get_Slot(0).float_value,
			obj->Get_Slot(1).float_value,
			obj->Get_Slot(2).float_value));
}

void CRuntime_Actor::Teleport(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMObject* obj = pos.Get();
	actor->Teleport(Vector3(
		obj->Get_Slot(0).float_value,
		obj->Get_Slot(1).float_value,
		obj->Get_Slot(2).float_value));
}

void CRuntime_Actor::LimitMovement(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	CVMObject* obj = pos.Get();
	actor->LimitMovement(Rect2D(
		obj->Get_Slot(0).float_value,
		obj->Get_Slot(1).float_value,
		obj->Get_Slot(2).float_value,
		obj->Get_Slot(3).float_value));
}

void CRuntime_Actor::Deactivate_Components(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	actor->Deactivate_Components();
}

CVMObjectHandle CRuntime_Actor::Get_Last_Frame_Movement(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	return vm->Create_Vec3(actor->Get_Last_Frame_Movement());
}

void CRuntime_Actor::Set_Tick_Offscreen(CVirtualMachine* vm, CVMValue self, int pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Tick_Offscreen(pos != 0);
}

int CRuntime_Actor::Get_Tick_Offscreen(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return actor->Get_Tick_Offscreen() ? 1 : 0;
}

void CRuntime_Actor::Set_Tick_Priority(CVirtualMachine* vm, CVMValue self, int pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Tick_Priority((TickPriority::Type)pos);
}

int CRuntime_Actor::Get_Tick_Priority(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (int)actor->Get_Tick_Priority();
}

void CRuntime_Actor::Set_Tick_Area(CVirtualMachine* vm, CVMValue self, CVMObjectHandle pos)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	actor->Set_Tick_Area(Rect2D(
		pos.GetNullCheck(vm)->Get_Slot(0).float_value,
		pos.GetNullCheck(vm)->Get_Slot(1).float_value,
		pos.GetNullCheck(vm)->Get_Slot(2).float_value,
		pos.GetNullCheck(vm)->Get_Slot(3).float_value));
}

CVMObjectHandle CRuntime_Actor::Get_Tick_Area(CVirtualMachine* vm, CVMValue self)
{
	ScriptedActor* actor = reinterpret_cast<ScriptedActor*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* vec4_class = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	Rect2D area = actor->Get_Tick_Area();

	CVMObjectHandle result = vm->New_Object(vec4_class, false);
	CVMObject* obj = result.Get();
	obj->Resize(4);
	obj->Get_Slot(0).float_value = area.X;
	obj->Get_Slot(1).float_value = area.Y;
	obj->Get_Slot(2).float_value = area.Width;
	obj->Get_Slot(3).float_value = area.Height;

	return result;
}

void CRuntime_Actor::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Net_ID",			&Get_Net_ID);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Unique_ID",		&Get_Unique_ID);

	vm->Get_Bindings()->Bind_Method<void,CVMString>			("Actor", "Set_Link",			&Set_Link);
	vm->Get_Bindings()->Bind_Method<CVMString>				("Actor", "Get_Link",			&Get_Link);
	vm->Get_Bindings()->Bind_Method<void,CVMString>			("Actor", "Set_Tag",			&Set_Tag);
	vm->Get_Bindings()->Bind_Method<CVMString>				("Actor", "Get_Tag",			&Get_Tag);

	vm->Get_Bindings()->Bind_Method<void,int>				("Actor", "Set_Enabled",		&Set_Enabled);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Enabled",		&Get_Enabled);
	vm->Get_Bindings()->Bind_Method<void,int>				("Actor", "Set_Activated_By_Player",		&Set_Activated_By_Player);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Activated_By_Player",		&Get_Activated_By_Player);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Position",		&Get_Position);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Set_Position",		&Set_Position);
	vm->Get_Bindings()->Bind_Method<void,float>				("Actor", "Set_Rotation",		&Set_Rotation);
	vm->Get_Bindings()->Bind_Method<float>					("Actor", "Get_Rotation",		&Get_Rotation);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Set_Scale",			&Set_Scale);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Scale",			&Get_Scale);
	vm->Get_Bindings()->Bind_Method<void,float>				("Actor", "Set_Depth_Bias",		&Set_Depth_Bias);
	vm->Get_Bindings()->Bind_Method<float>					("Actor", "Get_Depth_Bias",		&Get_Depth_Bias);
	vm->Get_Bindings()->Bind_Method<void,float>				("Actor", "Set_Depth_Y_Offset",	&Set_Depth_Y_Offset);
	vm->Get_Bindings()->Bind_Method<float>					("Actor", "Get_Depth_Y_Offset",	&Get_Depth_Y_Offset);
	vm->Get_Bindings()->Bind_Method<void,int>				("Actor", "Set_Layer",			&Set_Layer);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Layer",			&Get_Layer);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Set_Bounding_Box",	&Set_Bounding_Box);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Bounding_Box",	&Get_Bounding_Box);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Center",			&Get_Center);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Collision_Center", &Get_Collision_Center);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Collision_Box", &Get_Collision_Box);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_World_Collision_Box", &Get_World_Collision_Box);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_World_Bounding_Box",	&Get_World_Bounding_Box);
	//vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Screen_Bounding_Box",	&Get_Screen_Bounding_Box);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Set_Owner",			&Set_Owner);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Owner",			&Get_Owner);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Set_Parent",			&Set_Parent);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Parent",			&Get_Parent);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Last_Frame_Movement",			&Get_Last_Frame_Movement);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "MoveTo",					&MoveTo);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Teleport",				&Teleport);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "LimitMovement",			&LimitMovement);

	vm->Get_Bindings()->Bind_Method<void>					("Actor", "Deactivate_Components",	&Deactivate_Components);

	vm->Get_Bindings()->Bind_Method<void>					("Actor", "Hibernate",				&Hibernate);
	vm->Get_Bindings()->Bind_Method<void>					("Actor", "Wake_Up",				&Wake_Up);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Is_Hibernating",			&Is_Hibernating);

	vm->Get_Bindings()->Bind_Method<void,int>				("Actor", "Set_Tick_Offscreen",		&Set_Tick_Offscreen);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Tick_Offscreen",		&Get_Tick_Offscreen);

	vm->Get_Bindings()->Bind_Method<void,int>				("Actor", "Set_Tick_Priority",		&Set_Tick_Priority);
	vm->Get_Bindings()->Bind_Method<int>					("Actor", "Get_Tick_Priority",		&Get_Tick_Priority);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Actor", "Set_Tick_Area",			&Set_Tick_Area);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Actor", "Get_Tick_Area",			&Get_Tick_Area);
}

