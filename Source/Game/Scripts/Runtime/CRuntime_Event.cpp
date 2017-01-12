// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Event.h"
#include "Game/Scripts/ScriptEventListener.h"
#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Runner/Game.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

void CRuntime_Event::Fire_Global(CVirtualMachine* vm, CVMValue self)
{
	CVMLinkedSymbol* symbol = self.object_value.GetNullCheck(vm)->Get_Symbol();
	ScriptEventListener::Fire_Global_Custom(symbol->event_name_hash, self.object_value);
}

void CRuntime_Event::Fire_Global_By_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle type_of_actor)
{
	GameScene* scene = Game::Get()->Get_Game_Scene();

	CVMLinkedSymbol* symbol = self.object_value.GetNullCheck(vm)->Get_Symbol();
	CVMLinkedSymbol* type_symbol = vm->Get_Symbol_Table_Entry(type_of_actor.Get()->Get_Slot(0).int_value);

	std::vector<ScriptedActor*> actors;	
	scene->Get_Scripted_Actors_Of_Type(type_symbol, actors);

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		actor->Get_Event_Listener()->Fire_Custom(symbol->event_name_hash, self.object_value);
	}
}

void CRuntime_Event::Fire_In_Radius(CVirtualMachine* vm, CVMValue self, CVMObjectHandle position, float radius)
{
	GameScene* scene = Game::Get()->Get_Game_Scene();

	CVMLinkedSymbol* symbol = self.object_value.GetNullCheck(vm)->Get_Symbol();

	Vector3 pos;
	pos.X = position.Get()->Get_Slot(0).float_value;
	pos.Y = position.Get()->Get_Slot(1).float_value;
	pos.Z = position.Get()->Get_Slot(2).float_value;

	std::vector<ScriptedActor*> actors;	
	scene->Get_Scripted_Actors_In_Radius(pos, radius, actors);

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		
		float distance = (actor->Get_Position() - pos).Length();

		actor->Get_Event_Listener()->Fire_Custom(symbol->event_name_hash, self.object_value);
	}
}

void CRuntime_Event::Fire_In_Radius_By_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle type_of_actor, CVMObjectHandle position, float radius)
{
	GameScene* scene = Game::Get()->Get_Game_Scene();

	CVMLinkedSymbol* symbol = self.object_value.GetNullCheck(vm)->Get_Symbol();
	CVMLinkedSymbol* type_symbol = vm->Get_Symbol_Table_Entry(type_of_actor.Get()->Get_Slot(0).int_value);

	Vector3 pos;
	pos.X = position.Get()->Get_Slot(0).float_value;
	pos.Y = position.Get()->Get_Slot(1).float_value;
	pos.Z = position.Get()->Get_Slot(2).float_value;

	std::vector<ScriptedActor*> actors;	
	scene->Get_Scripted_Actors_In_Radius(pos, radius, type_symbol, actors);

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		actor->Get_Event_Listener()->Fire_Custom(symbol->event_name_hash, self.object_value);
	}
}

void CRuntime_Event::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void>("Event", "Fire_Global", &Fire_Global);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Event", "Fire_Global_By_Type", &Fire_Global_By_Type);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,float>("Event", "Fire_In_Radius", &Fire_In_Radius);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,CVMObjectHandle,float>("Event", "Fire_In_Radius_By_Type", &Fire_In_Radius_By_Type);
}
