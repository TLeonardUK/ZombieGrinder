// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Scene.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Game/Runner/Game.h"
#include "Game/Runner/GameMode.h"

#include "Engine/Engine/EngineOptions.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/Display/GfxDisplay.h"

#include "Game/Scene/GameScene.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/ActorReplicator.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetServer.h"
#include "Game/Network/GameNetClient.h"

#include <float.h>

CVMObjectHandle CRuntime_Scene::Get_Active_Game_Mode(CVirtualMachine* vm)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	ScriptedActor* actor = Game::Get()->Get_Game_Scene()->Find_Actor_By_Unique_ID(Game::Get()->Get_Game_Mode()->Get_Active_Net_ID());
	if (actor != NULL)
		return actor->Get_Script_Object();
	else
		return NULL;
}

int CRuntime_Scene::Is_Spawned(CVirtualMachine* vm, CVMObjectHandle actor)
{
	return actor.Get()->Get_Meta_Data() != NULL;
}

void CRuntime_Scene::Set_Active_Game_Mode(CVirtualMachine* vm, CVMObjectHandle actor)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif
	vm->Assert(GameNetManager::Get()->Game_Server() != NULL);

	ScriptedActor* sa = reinterpret_cast<ScriptedActor*>(actor.Get()->Get_Meta_Data());
	Game::Get()->Get_Game_Mode()->Set_Active_Net_ID(sa->Get_Replication_Info().unique_id);
}

CVMObjectHandle CRuntime_Scene::Get_Map_Tint(CVirtualMachine* vm)
{
	return vm->Create_Vec4(GameEngine::Get()->Get_Map()->Get_Color_Tint());
}

void CRuntime_Scene::Set_Map_Tint(CVirtualMachine* vm, CVMObjectHandle actor)
{
	GameEngine::Get()->Get_Map()->Set_Color_Tint(actor.Get()->To_Vec4());
}

CVMObjectHandle CRuntime_Scene::Get_Ambient_Lighting(CVirtualMachine* vm)
{
	return vm->Create_Vec4(RenderPipeline::Get()->Get_Ambient_Lighting());
}

void CRuntime_Scene::Set_Ambient_Lighting(CVirtualMachine* vm, CVMObjectHandle actor)
{
	RenderPipeline::Get()->Set_Ambient_Lighting(actor.Get()->To_Vec4());
}

CVMObjectHandle CRuntime_Scene::Get_Clear_Color(CVirtualMachine* vm)
{
	Color c = RenderPipeline::Get()->Get_Game_Clear_Color();
	return vm->Create_Vec4(c.To_Vector4());
}

void CRuntime_Scene::Set_Clear_Color(CVirtualMachine* vm, CVMObjectHandle actor)
{
	Vector4 v = actor.Get()->To_Vec4();
	RenderPipeline::Get()->Set_Game_Clear_Color(Color(v.X * 255, v.Y * 255, v.Z * 255, v.W * 255));
}


CVMObjectHandle CRuntime_Scene::Find_Actor_By_Field(CVirtualMachine* vm, CVMObjectHandle type, int field_offset, int value)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	std::vector<ScriptedActor*> matches = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		if (actor->Get_Script_Object().Get()->Get_Slot(field_offset).int_value == value)
		{
			return actor->Get_Script_Object();
		}
	}

	return NULL;
}

CVMObjectHandle CRuntime_Scene::Find_Actors(CVirtualMachine* vm, CVMObjectHandle type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	std::vector<ScriptedActor*> matches = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());
	
	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Enabled_Actors(CVirtualMachine* vm, CVMObjectHandle type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	std::vector<ScriptedActor*> matches = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	int count = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++)
	{
		if ((*iter)->Get_Enabled())
		{
			count++;
		}
	}

	CVMObjectHandle handle = vm->New_Array(return_value, count);

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++)
	{
		if ((*iter)->Get_Enabled())
		{
			handle.Get()->Get_Slot(index++).object_value = (*iter)->Get_Script_Object();
		}
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Actors_In_Radius(CVirtualMachine* vm, CVMObjectHandle type, float radius, CVMObjectHandle center)
{	
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	Vector3 real_center;
	real_center.X = center.Get()->Get_Slot(0).float_value;
	real_center.Y = center.Get()->Get_Slot(1).float_value;
	real_center.Z = center.Get()->Get_Slot(2).float_value;

	std::vector<ScriptedActor*> actors =  Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	std::vector<ScriptedActor*> matches;
	//matches.reserve(actors.size());

	float radius_sqr = radius * radius;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		float distance = (actor->Get_Collision_Center() - real_center).Length_Squared();
		if (distance <= radius_sqr)
		{
			matches.push_back(actor);
		}
	}

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Actors_In_Area(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle inBBOX)
{	
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	std::vector<ScriptedActor*> actors =  Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	std::vector<ScriptedActor*> matches;
	//matches.reserve(actors.size());

	Rect2D bbox(
		inBBOX.Get()->Get_Slot(0).float_value,
		inBBOX.Get()->Get_Slot(1).float_value,
		inBBOX.Get()->Get_Slot(2).float_value,
		inBBOX.Get()->Get_Slot(3).float_value
	);

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		if (actor->Get_World_Bounding_Box().Intersects(bbox))
		{
			matches.push_back(actor);
		}
	}

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Actors_With_Collision_In_Area(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle inBBOX)
{	
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	std::vector<ScriptedActor*> actors =  Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	std::vector<ScriptedActor*> matches;
	//matches.reserve(actors.size());

	Rect2D bbox(
		inBBOX.Get()->Get_Slot(0).float_value,
		inBBOX.Get()->Get_Slot(1).float_value,
		inBBOX.Get()->Get_Slot(2).float_value,
		inBBOX.Get()->Get_Slot(3).float_value
		);

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		Vector3 full = actor->Get_Collision_Center();
		Vector2 col_center = Vector2(full.X, full.Y);

		if (bbox.Intersects(col_center))
		{
			matches.push_back(actor);
		}
	}

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Actors_Distance_From_Cameras(CVirtualMachine* vm, CVMObjectHandle type, float min_distance, float max_distance)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	std::vector<ScriptedActor*> actors =  Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	std::vector<ScriptedActor*> matches;
	//matches.reserve(actors.size());

	GameNetManager* net_manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = net_manager->Get_Game_Net_Users();

	std::vector<Vector3> camera_centers;
	camera_centers.reserve(users.size());

	// Find center of screen.
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		Rect2D view = user->Get_Estimated_Viewport();
		Vector2 center = view.Center();
		camera_centers.push_back(Vector3(center.X, center.Y, 0.0f));
	}

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		float minimum_distance = 0.0f;
		float maximum_distance = 0.0f;

		for (unsigned int i = 0; i < camera_centers.size(); i++)
		{
			float distance = (actor->Get_Collision_Center() - camera_centers.at(i)).Length_Squared();

			if (distance < minimum_distance || i == 0)
			{
				minimum_distance = distance;
			}
			if (distance > maximum_distance || i == 0)
			{
				maximum_distance = distance;
			}
		}

		if (minimum_distance >= (min_distance * min_distance) && maximum_distance <= (max_distance * max_distance))
		{
			matches.push_back(actor);
		}
	}

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Actors_In_FOV(CVirtualMachine* vm, CVMObjectHandle type, float radius, CVMObjectHandle center, float fov, CVMObjectHandle heading)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	Vector3 real_center;
	real_center.X = center.Get()->Get_Slot(0).float_value;
	real_center.Y = center.Get()->Get_Slot(1).float_value;
	real_center.Z = center.Get()->Get_Slot(2).float_value;

	Vector3 real_heading;
	real_heading.X = heading.Get()->Get_Slot(0).float_value;
	real_heading.Y = heading.Get()->Get_Slot(1).float_value;
	real_heading.Z = heading.Get()->Get_Slot(2).float_value;

	std::vector<ScriptedActor*> actors =  Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	std::vector<ScriptedActor*> matches;
	matches.reserve(actors.size());

	float half_fov = fov * 0.5f;
	float half_delta = 1.0f - (half_fov / PI);
	float dot_target = (half_delta * 2.0f) - 1.0f;

	float radius_sqr = radius * radius;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		Vector3 actor_center = actor->Get_Collision_Center();
		float distance = (actor_center - real_center).Length_Squared();

		if (distance > 0.0f && distance <= radius_sqr)
		{
			Vector3 vec_to_target = (actor_center - real_center).Normalize();
			float dot = vec_to_target.Dot(real_heading);

			if (dot >= dot_target)
			{
				matches.push_back(actor);
			}
		}
	}

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Find_Closest_Actor(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle center)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	Vector3 real_center;
	real_center.X = center.Get()->Get_Slot(0).float_value;
	real_center.Y = center.Get()->Get_Slot(1).float_value;
	real_center.Z = center.Get()->Get_Slot(2).float_value;

	std::vector<ScriptedActor*> actors =  Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	float closest_dist = 0.0f;
	ScriptedActor* closest_actor = NULL;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		float distance = (actor->Get_Collision_Center() - real_center).Length_Squared();
		if (distance <= closest_dist || closest_actor == NULL)
		{
			closest_actor = actor;
			closest_dist = distance;
		}
	}

	return closest_actor == NULL ? NULL : closest_actor->Get_Script_Object();
}

CVMObjectHandle CRuntime_Scene::Find_Closest_Actor_Filtered(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle center, CVMObjectHandle except)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());

	Vector3 real_center;
	real_center.X = center.Get()->Get_Slot(0).float_value;
	real_center.Y = center.Get()->Get_Slot(1).float_value;
	real_center.Z = center.Get()->Get_Slot(2).float_value;

	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	float closest_dist = 0.0f;
	ScriptedActor* closest_actor = NULL;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		if (actor->Get_Script_Object().Get() == except.Get())
		{
			continue;
		}

		float distance = (actor->Get_Collision_Center() - real_center).Length_Squared();
		if (distance <= closest_dist || closest_actor == NULL)
		{
			closest_actor = actor;
			closest_dist = distance;
		}
	}

	return closest_actor == NULL ? NULL : closest_actor->Get_Script_Object();
}

CVMObjectHandle CRuntime_Scene::Find_Actors_By_Tag(CVirtualMachine* vm, CVMString type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	std::vector<std::string> links;
	StringHelper::Split(type.C_Str(), ',', links);

	std::vector<ScriptedActor*> matches;
	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Get_Scripted_Actors();

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor =*iter;

		std::vector<std::string> tags = actor->Get_Tags();

		bool bFound = false;
		for (std::vector<std::string>::iterator tag_iter = tags.begin(); tag_iter != tags.end(); tag_iter++)
		{
			if (std::find(links.begin(), links.end(), *tag_iter) != links.end())
			{
				bFound = true;
				break;
			}
		}

		if (bFound)
		{
			matches.push_back(actor);
		}	
	}

	CVMLinkedSymbol* return_value = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle handle = vm->New_Array(return_value, matches.size());

	int index = 0;
	for (std::vector<ScriptedActor*>::iterator iter = matches.begin(); iter != matches.end(); iter++, index++)
	{
		handle.Get()->Get_Slot(index).object_value = (*iter)->Get_Script_Object();
	}

	return handle;
}

CVMObjectHandle CRuntime_Scene::Spawn(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle owner)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());
	GameNetUser* o = owner.Get() == NULL ? NULL : reinterpret_cast<GameNetUser*>(owner.Get()->Get_Meta_Data());		

	vm->Assert(class_symbol->symbol->class_data->is_replicated == false || GameNetManager::Get()->Game_Server() != NULL, "Attempt to spawn replicated class '%s' on client!", class_symbol->symbol->name);

	return Game::Get()->Get_Game_Scene()->Spawn(class_symbol, o)->Get_Script_Object();
}

void CRuntime_Scene::Despawn(CVirtualMachine* vm, CVMObjectHandle obj)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	ScriptedActor* sa = reinterpret_cast<ScriptedActor*>(obj.Get()->Get_Meta_Data_NullCheck(vm));

	Game::Get()->Get_Game_Scene()->Despawn(sa);
}

CVMObjectHandle CRuntime_Scene::Get_Bounds(CVirtualMachine* vm)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* vec4_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	
	Rect2D bounds = Game::Get()->Get_Scene()->Get_Boundries();

	CVMObjectHandle result = vm->New_Object(vec4_symbol, false);
	CVMObject* obj_result = result.Get();
	obj_result->Resize(4);
	obj_result->Get_Slot(0) = bounds.X;
	obj_result->Get_Slot(1) = bounds.Y;
	obj_result->Get_Slot(2) = bounds.Width;
	obj_result->Get_Slot(3) = bounds.Height;

	return result;
}
/*
CVMObjectHandle CRuntime_Scene::Get_Active_Camera(CVirtualMachine* vm)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	GameMode* mode = Game::Get()->Get_Game_Mode();
	return mode == NULL ? NULL : mode->Get_Camera();
}
*/
CVMObjectHandle CRuntime_Scene::Get_Net_ID_Actor(CVirtualMachine* vm, int id)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	ScriptedActor* actor = ActorReplicator::Get()->Get_Actor_By_ID(id);
	return actor == NULL ? NULL : actor->Get_Script_Object();
}

CVMObjectHandle CRuntime_Scene::Get_Unique_ID_Actor(CVirtualMachine* vm, int id)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	ScriptedActor* actor = Game::Get()->Get_Game_Scene()->Get_Actor_By_Map_ID(id);
	return actor == NULL ? NULL : actor->Get_Script_Object();
}

float CRuntime_Scene::Distance_To_Closest_Actor_Of_Type(CVirtualMachine* vm, CVMObjectHandle center, CVMObjectHandle type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	Vector3 real_center = center.Get()->To_Vec3();
	
	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	int count = 0;

	float lowest_distance = FLT_MAX;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		float distance = (real_center - actor->Get_World_Center()).Length();
		if (distance < lowest_distance || lowest_distance == FLT_MAX)
		{
			lowest_distance = distance;
		}
	}

	return lowest_distance;
}

float CRuntime_Scene::Distance_To_Furthest_Actor_Of_Type(CVirtualMachine* vm, CVMObjectHandle center, CVMObjectHandle type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.Get()->Get_Meta_Data());	

	Vector3 real_center = center.Get()->To_Vec3();

	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);
	int count = 0;

	float highest_distance = FLT_MIN;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;

		float distance = (real_center - actor->Get_World_Center()).Length();
		if (distance > highest_distance || highest_distance == FLT_MIN)
		{
			highest_distance = distance;
		}
	}

	return highest_distance;
}

float CRuntime_Scene::Distance_To_Enemy_Actors(CVirtualMachine* vm, CVMObjectHandle center, int team_index)
{
	Vector3 real_center = center.Get()->To_Vec3();

	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Get_Scripted_Actors();

	float distance = FLT_MAX;

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		if (actor->Get_Owner() != NULL &&
			actor->Get_Owner()->Get_State()->Team_Index != team_index)
		{
			float d = (actor->Get_World_Center() - real_center).Length();
			if (d < distance)
			{
				distance = d;
			}
		}
	}

	return distance;
}

int CRuntime_Scene::Count_Actors_By_Type(CVirtualMachine* vm, CVMObjectHandle type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.GetNullCheck(vm)->Get_Meta_Data());	

	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	/*
	int count = 0;

	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(*iter);
		if (actor != NULL)
		{
			if (vm->Is_Class_Derived_From(actor->Get_Script_Object().Get()->Get_Symbol(), class_symbol))
			{
				count++;
			}
		}
	}
	*/

	return actors.size();
}

int CRuntime_Scene::Are_Actors_Colliding(CVirtualMachine* vm, CVMObjectHandle type, CVMObjectHandle bbox)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.GetNullCheck(vm)->Get_Meta_Data());	

	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	Rect2D rect_bbox = Rect2D(bbox.Get()->To_Vec4());

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		if (actor->Get_World_Bounding_Box().Intersects(rect_bbox))
		{
			return 1;
		}
	}

	return 0;
}

int CRuntime_Scene::Are_Actors_Visible(CVirtualMachine* vm, CVMObjectHandle type)
{
#ifdef VM_ALLOW_MULTITHREADING
	MutexLock lock(vm->Get_Global_Mutex());
#endif

	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(type.GetNullCheck(vm)->Get_Meta_Data());	

	std::vector<ScriptedActor*> actors = Game::Get()->Get_Game_Scene()->Find_Derived_Actors(class_symbol);

	GameNetManager* net_manager = GameNetManager::Get();
	std::vector<GameNetUser*> users = net_manager->Get_Game_Net_Users();

	std::vector<Rect2D> camera_views;
	camera_views.reserve(users.size());

	// Find center of screen.
	for (std::vector<GameNetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = *iter;
		camera_views.push_back(user->Get_Estimated_Viewport());
	}

	for (std::vector<ScriptedActor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		ScriptedActor* actor = *iter;
		Rect2D actor_bbox = actor->Get_World_Bounding_Box();

		for (std::vector<Rect2D>::iterator iter2 = camera_views.begin(); iter2 != camera_views.end(); iter2++)
		{
			if (actor_bbox.Intersects(*iter2))
			{
				return 1;
			}
		}
	}

	return 0;
}


/*
CVMObjectHandle CRuntime_Scene::Get_Possessed_Actors(CVirtualMachine* vm)
{
	GameNetManager* manager = GameNetManager::Get();
	std::vector<NetUser*> users = manager->Get_Net_Users();

	CVMLinkedSymbol* array_type = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle arr = vm->New_Array(array_type, 0);

	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		ScriptedActor* actor = user->Get_Possession_Actor();
		if (actor != NULL)
		{
			arr.Get()->Add_Last(actor->Get_Script_Object());
		}
	}

	return arr;
}

CVMObjectHandle CRuntime_Scene::Get_Local_Possessed_Actors(CVirtualMachine* vm)
{	
	GameNetManager* manager = GameNetManager::Get();
	std::vector<NetUser*> users = manager->Get_Local_Net_Users();

	CVMLinkedSymbol* array_type = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);
	CVMObjectHandle arr = vm->New_Array(array_type, 0);

	for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
	{
		GameNetUser* user = static_cast<GameNetUser*>(*iter);
		ScriptedActor* actor = user->Get_Possession_Actor();
		if (user->Get_Online_User()->Is_Local() && actor != NULL)
		{
			arr.Get()->Add_Last(actor->Get_Script_Object());
		}
	}

	return arr;
}
*/

CVMObjectHandle CRuntime_Scene::Get_Camera_Screen_Viewport(CVirtualMachine* vm, CVMObjectHandle cam)
{
	GameMode* mode = Game::Get()->Get_Game_Mode();
	int camera_index = 0;

	for (int i = 0; i < mode->Get_Camera_Count(); i++)
	{
		if (mode->Get_Camera(i).Get() == cam.Get())
		{
			camera_index = i;
			break;
		}
	}

	Rect2D viewport = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + camera_index))->Get_Screen_Viewport();

	return vm->Create_Vec4(viewport.X, viewport.Y, viewport.Width, viewport.Height);
}

CVMObjectHandle CRuntime_Scene::Get_Camera_Screen_Viewport_Index(CVirtualMachine* vm, int index)
{
	Rect2D viewport = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + index))->Get_Screen_Viewport();
	return vm->Create_Vec4(viewport.X, viewport.Y, viewport.Width, viewport.Height);
}

int CRuntime_Scene::Get_Camera_Index(CVirtualMachine* vm, CVMObjectHandle cam)
{
	GameMode* mode = Game::Get()->Get_Game_Mode();

	for (int i = 0; i < mode->Get_Camera_Count(); i++)
	{
		if (mode->Get_Camera(i).Get() == cam.Get())
		{
			return i;
		}
	}

	return 0;
}

CVMObjectHandle CRuntime_Scene::Get_Camera_Screen_Resolution(CVirtualMachine* vm, CVMObjectHandle cam)
{
	GameMode* mode = Game::Get()->Get_Game_Mode();
	int camera_index = 0;

	for (int i = 0; i < mode->Get_Camera_Count(); i++)
	{
		if (mode->Get_Camera(i).Get() == cam.Get())
		{
			camera_index = i;
			break;
		}
	}

	Rect2D viewport = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + camera_index))->Get_Screen_Viewport();

	float scale_x = viewport.Width / GfxDisplay::Get()->Get_Width();
	float scale_y = viewport.Height / GfxDisplay::Get()->Get_Height();

	float aspect = viewport.Width / viewport.Height;

	// Only scale if we are the long-thin split-screen viewports.
	if (aspect <= 2)
	{
		scale_x = 1.0f;
		scale_y = 1.0f;
	}

	float res_x = ((float)*EngineOptions::render_game_width) * scale_x;
	float res_y = ((float)*EngineOptions::render_game_height) * scale_y;

	return vm->Create_Vec2(res_x, res_y);
}

int CRuntime_Scene::Get_Camera_Count(CVirtualMachine* vm)
{
	return Game::Get()->Get_Game_Mode()->Get_Camera_Count();
}

float CRuntime_Scene::Get_Map_Time(CVirtualMachine* vm)
{
	return GameNetManager::Get()->Get_Server_State().Elapsed_Map_Time;
}

int CRuntime_Scene::Get_Map_Dungeon_Level(CVirtualMachine* vm)
{
	return GameNetManager::Get()->Get_Current_Map_Dungeon_Level();
}

int CRuntime_Scene::Is_Workshop_Map(CVirtualMachine* vm)
{
	return (GameNetManager::Get()->Get_Current_Map_Workshop_ID() != 0) ? 1 : 0;
}

void CRuntime_Scene::Vibrate(CVirtualMachine* vm, CVMObjectHandle center, float duration, float intensity)
{
	return Game::Get()->Vibrate(center.Get()->To_Vec3(), duration, intensity);
}

void CRuntime_Scene::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Active_Game_Mode", &Get_Active_Game_Mode);
	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle>("Scene", "Set_Active_Game_Mode", &Set_Active_Game_Mode);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Map_Tint", &Get_Map_Tint);
	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle>("Scene", "Set_Map_Tint", &Set_Map_Tint);

	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Ambient_Lighting", &Get_Ambient_Lighting);
	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle>("Scene", "Set_Ambient_Lighting", &Set_Ambient_Lighting);

	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Clear_Color", &Get_Clear_Color);
	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle>("Scene", "Set_Clear_Color", &Set_Clear_Color);

	vm->Get_Bindings()->Bind_Function<int>("Scene", "Get_Map_Dungeon_Level", &Get_Map_Dungeon_Level);
	vm->Get_Bindings()->Bind_Function<int>("Scene", "Is_Workshop_Map", &Is_Workshop_Map);

	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle>("Scene", "Find_Actors", &Find_Actors);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,int,int>("Scene", "Find_Actor_By_Field", &Find_Actor_By_Field);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle>("Scene", "Find_Enabled_Actors", &Find_Enabled_Actors);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,float,CVMObjectHandle>("Scene", "Find_Actors_In_Radius", &Find_Actors_In_Radius);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle>("Scene", "Find_Actors_In_Area", &Find_Actors_In_Area);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle>("Scene", "Find_Actors_With_Collision_In_Area", &Find_Actors_With_Collision_In_Area);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,float,float>("Scene", "Find_Actors_Distance_From_Cameras", &Find_Actors_Distance_From_Cameras);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,float,CVMObjectHandle,float,CVMObjectHandle>("Scene", "Find_Actors_In_FOV", &Find_Actors_In_FOV);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString>("Scene", "Find_Actors_By_Tag", &Find_Actors_By_Tag);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle>("Scene", "Find_Closest_Actor", &Find_Closest_Actor);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle, CVMObjectHandle, CVMObjectHandle, CVMObjectHandle>("Scene", "Find_Closest_Actor_Filtered", &Find_Closest_Actor_Filtered);

	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle,CVMObjectHandle>("Scene", "Spawn", &Spawn);
	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle>("Scene", "Is_Spawned",	&Is_Spawned);
	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle>("Scene", "Despawn", &Despawn);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Bounds", &Get_Bounds);
//	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Active_Camera", &Get_Active_Camera);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,int>("Scene", "Get_Net_ID_Actor", &Get_Net_ID_Actor);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle, int>("Scene", "Get_Unique_ID_Actor", &Get_Unique_ID_Actor);
	vm->Get_Bindings()->Bind_Function<float,CVMObjectHandle,CVMObjectHandle>("Scene", "Distance_To_Closest_Actor_Of_Type", &Distance_To_Closest_Actor_Of_Type);
	vm->Get_Bindings()->Bind_Function<float,CVMObjectHandle,CVMObjectHandle>("Scene", "Distance_To_Furthest_Actor_Of_Type", &Distance_To_Furthest_Actor_Of_Type);
	vm->Get_Bindings()->Bind_Function<float,CVMObjectHandle,int>("Scene", "Distance_To_Enemy_Actors", &Distance_To_Enemy_Actors);
	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle>("Scene", "Count_Actors_By_Type", &Count_Actors_By_Type);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,int>("Scene", "Get_Camera_Screen_Viewport_Index", &Get_Camera_Screen_Viewport_Index);
	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle>("Scene", "Get_Camera_Index", &Get_Camera_Index);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle>("Scene", "Get_Camera_Screen_Viewport", &Get_Camera_Screen_Viewport);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMObjectHandle>("Scene", "Get_Camera_Screen_Resolution", &Get_Camera_Screen_Resolution);
	vm->Get_Bindings()->Bind_Function<int>("Scene", "Get_Camera_Count", &Get_Camera_Count);

	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle,CVMObjectHandle>("Scene", "Are_Actors_Colliding", &Are_Actors_Colliding);
	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle>("Scene", "Are_Actors_Visible", &Are_Actors_Visible);

	vm->Get_Bindings()->Bind_Function<void,CVMObjectHandle,float,float>("Scene", "Vibrate", &Vibrate);

	vm->Get_Bindings()->Bind_Function<float>("Time", "Get_Map_Time", &Get_Map_Time);

//	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Possessed_Actors", &Get_Possessed_Actors);
//	vm->Get_Bindings()->Bind_Function<CVMObjectHandle>("Scene", "Get_Local_Possessed_Actors", &Get_Local_Possessed_Actors);
}

