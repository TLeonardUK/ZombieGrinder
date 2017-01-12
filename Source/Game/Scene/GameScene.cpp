// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/GameScene.h"
#include "Game/Runner/GameMode.h"
#include "Game/Runner/Game.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/DemoProxyActor.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Network/ActorReplicator.h"
#include "Game/Network/GameNetUser.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/IO/BinaryStream.h"

#include "Engine/Engine/GameEngine.h"

void GameScene::Unload()
{
	Scene::Unload();

	m_scripted_actors.clear();
	m_despawn_actors.clear();
	m_scripted_actor_types.Clear();
}

void GameScene::Add_Actor(Actor* camera)
{
	if (std::find(m_actors.begin(), m_actors.end(), camera) == m_actors.end())
	{
		m_actors.push_back(camera);
	}

	ScriptedActor* scripted = dynamic_cast<ScriptedActor*>(camera);
	if (scripted != NULL)
	{
		Add_Scripted_Actor(scripted);
	}
}

void GameScene::Remove_Actor(Actor* camera)
{
	std::vector<Actor*>::iterator iter = std::find(m_actors.begin(), m_actors.end(), camera);
	if (iter != m_actors.end())
	{
		m_actors.erase(iter);
	}

	ScriptedActor* scripted = dynamic_cast<ScriptedActor*>(camera);
	if (scripted != NULL)
	{
		Remove_Scripted_Actor(scripted);
	}
}

void GameScene::Calculate_Full_Camera_Bounding_Box()
{
	Rect2D area;
	int index = 0;

	GameMode* mode = Game::Get()->Get_Game_Mode();
	for (int i = 0; i < mode->Get_Camera_Count(); i++)
	{
		Camera* cam = Game::Get()->Get_Camera((CameraID::Type)(CameraID::Game1 + i));
		Rect2D view = cam->Get_Bounding_Viewport();

		if (i == 0)
		{
			area = view;
		}
		else
		{
			area = area.Union(view);
		}
	}

	m_full_camera_bounding_box = area;
}

Rect2D GameScene::Get_Full_Camera_Bounding_Box()
{
	return m_full_camera_bounding_box;
}

void GameScene::Add_Scripted_Actor(ScriptedActor* scripted)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	
	// Actors are bucketed when added to the scene so we can easily do things like Find(typeof(Enemy)) 
	// without iterating all objects and doing a buck of Is_Derived() checks.
	if (std::find(m_scripted_actors.begin(), m_scripted_actors.end(), scripted) == m_scripted_actors.end())
	{
		m_scripted_actors.push_back(scripted);

		// Go through all of our classes and add us to relevant buckets.
		CVMLinkedSymbol* symbol = scripted->Get_Script_Symbol();
		while (symbol != NULL && symbol->symbol->type != SymbolType::Root)
		{
			for (int i = 0; i < symbol->symbol->class_data->interface_count; i++)
			{
				CVMLinkedSymbol* inter = vm->Get_Symbol_Table_Entry(symbol->symbol->class_data->interfaces[i].interface_symbol_index);
				Add_To_Scripted_Bucket(inter, scripted);
			}

			Add_To_Scripted_Bucket(symbol, scripted);
			symbol = vm->Get_Symbol_Table_Entry(symbol->symbol->class_data->super_class_index);
		}
	}
}

void GameScene::Remove_Scripted_Actor(ScriptedActor* scripted)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	std::vector<ScriptedActor*>::iterator iter = std::find(m_scripted_actors.begin(), m_scripted_actors.end(), scripted);
	if (iter != m_scripted_actors.end())
	{
		m_scripted_actors.erase(iter);
	}

	// Go through all of our classes and add us to relevant buckets.
	CVMLinkedSymbol* symbol = scripted->Get_Script_Symbol();
	while (symbol != NULL && symbol->symbol->type != SymbolType::Root)
	{
		for (int i = 0; i < symbol->symbol->class_data->interface_count; i++)
		{
			CVMLinkedSymbol* inter = vm->Get_Symbol_Table_Entry(symbol->symbol->class_data->interfaces[i].interface_symbol_index);
			Remove_From_Scripted_Bucket(inter, scripted);
		}

		Remove_From_Scripted_Bucket(symbol, scripted);
		symbol = vm->Get_Symbol_Table_Entry(symbol->symbol->class_data->super_class_index);
	}
}

void GameScene::Add_To_Scripted_Bucket(CVMLinkedSymbol* sym, ScriptedActor* actor)
{
	ActorBucket* bucket = NULL;
	if (!m_scripted_actor_types.Get_Ptr(sym->name_hash, bucket))
	{
		ActorBucket new_bucket;
		new_bucket.actors.push_back(actor);

		m_scripted_actor_types.Set(sym->name_hash, new_bucket);
	}
	else
	{
		bucket->actors.push_back(actor);
	}
}

void GameScene::Remove_From_Scripted_Bucket(CVMLinkedSymbol* sym, ScriptedActor* actor)
{
	ActorBucket* bucket = NULL;
	if (m_scripted_actor_types.Get_Ptr(sym->name_hash, bucket))
	{
		std::vector<ScriptedActor*>::iterator iter = std::find(bucket->actors.begin(), bucket->actors.end(), actor);
		if (iter != bucket->actors.end())
		{
			bucket->actors.erase(iter);
		}
	}
}

std::vector<ScriptedActor*> GameScene::Get_Scripted_Actors()
{
	return m_scripted_actors;
}

std::vector<ScriptedActor*> GameScene::Find_Derived_Actors(CVMLinkedSymbol* symbol)
{
	ActorBucket* bucket;

	if (m_scripted_actor_types.Get_Ptr(symbol->name_hash, bucket))
	{
		return bucket->actors;
	}

	return std::vector<ScriptedActor*>();

	/*
	std::vector<ScriptedActor*> actors;

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	// TODO: Bucket all actors to speed this the fuck up.

	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		if (vm->Is_Class_Derived_From(actor->Get_Script_Symbol(), symbol))
		{
			actors.push_back(actor);
		}
	}

	return actors;
	*/
}

ScriptedActor* GameScene::Find_Actor_By_Unique_ID(u32 id)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		if (actor->Get_Script_Symbol()->symbol->class_data->is_replicated &&
			actor->Get_Replication_Info().unique_id == id)
		{
			return actor;
		}
	}

	return NULL;
}

ScriptedActor* GameScene::Get_Actor_By_Map_ID(u32 id)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);

		if (actor->Get_Script_Symbol()->symbol->class_data->is_replicated == false &&
			actor->Get_Map_ID() == id)
		{
			return actor;
		}		
	}

	return NULL;
}

ScriptedActor* GameScene::Spawn(CVMLinkedSymbol* class_symbol, GameNetUser* user)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	// Create the object!
	ScriptedActor* actor = new ScriptedActor(vm, class_symbol);
	actor->Set_Owner(user);
	actor->Setup_Script();

	// Deserialize objects properties.
	CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());

	// Change to default state.
	vm->Set_Default_State(actor->Get_Script_Object());
	actor->Setup_Event_Hooks();
	
	// Add to scene.	
	Scene* scene = GameEngine::Get()->Get_Scene();
	scene->Add_Actor(actor);
	scene->Add_Tickable(actor);

	return actor;
}

void GameScene::Despawn(ScriptedActor* obj)
{
	PRINTF_PROFILE_SCOPE("GameScene::Despawn");

	CVMLinkedSymbol* sym = obj->Get_Script_Symbol();

	if (!!sym->symbol->class_data->is_replicated)
	{
		PRINTF_PROFILE_SCOPE("Actor Replicator Despawn");
// Actually we should allow this on clients so we can predict despawns.
//		if (GameNetManager::Get()->Game_Server() == NULL)
//		{
//			DBG_LOG("[WARNING] Ignored despawn request for actor '%i' - networked objects can only be despawned by server!", obj->Get_Replication_Info().unique_id);
//			return;
//		}
		ActorReplicator::Get()->Despawn_Actor(obj);
	}
	else
	{
		PRINTF_PROFILE_SCOPE("Register_Despawn");
		Register_Despawn(obj);
	}

	Despawn_Children(obj);
}

void GameScene::Register_Despawn(ScriptedActor* obj)
{
	// TODO: This whole thing is an ungodly mess - Fix object handling in scripts so this isnt needed!!

#ifndef MASTER_BUILD
	for (std::vector<DespawnRequest>::iterator iter = m_despawn_actors.begin(); iter != m_despawn_actors.end(); iter++)
	{
		DespawnRequest& actor = *iter;
		if (actor.actor == obj && 
			actor.defer_frames != 0) // We dont care if despawn is requested on same frame - this can be caused by things like - muzzle flash anim finishing and parent weapon destruction both causing despawns for the flash. 
		{
  			DBG_LOG(
				"[WARNING] A duplicate despawn was registered for actor (%i) of type '%s' (%i frames after first despawn request). This may cause potential issues, possible stale references!", 
				actor.actor->Get_Replication_Info().unique_id, 
				actor.actor->Get_Script_Symbol()->symbol->name,
				actor.defer_frames
			);
		}
	}
#endif

	DespawnRequest r;
	r.actor = obj;
	r.defer_frames = 0;

	{
		PRINTF_PROFILE_SCOPE("Object Deconstruction");
		obj->Deconstruct();
	}
	{
		PRINTF_PROFILE_SCOPE("Remove Actor");
		Remove_Actor(obj);
	}

//	DBG_LOG("Despawn request registered for object '%i'.", obj->Get_Replication_Info().unique_id);
	m_despawn_actors.push_back(r);
}

void GameScene::Despawn_Children(ScriptedActor* parent)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		if (actor->Get_Parent() == parent)
		{
			Despawn(actor);
			iter = m_scripted_actors.begin();
		}
	}
}

void GameScene::Run_Despawns()
{
	for (std::vector<DespawnRequest>::iterator iter = m_despawn_actors.begin(); iter != m_despawn_actors.end(); )
	{
		DespawnRequest& actor = *iter;
		actor.defer_frames++;

		if (actor.defer_frames >= despawn_defer_frames)
		{
			SAFE_DELETE(actor.actor);
			iter = m_despawn_actors.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

std::vector<Actor*> GameScene::Get_Actors_Of_Class(CVMLinkedSymbol* sym)
{
	std::vector<Actor*> result;

	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		if (actor->Get_Script_Symbol() == sym)
		{
			result.push_back(actor);
		}
	}

	return result;
}

void GameScene::Get_Scripted_Actors_In_Radius(Vector3 position, float radius, std::vector<ScriptedActor*>& result)
{
	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		float distance = (actor->Get_World_Center() - position).Length();
		if (distance <= radius)
		{
			result.push_back(actor);
		}
	}
}

void GameScene::Get_Scripted_Actors_In_Radius(Vector3 position, float radius, CVMLinkedSymbol* type_of, std::vector<ScriptedActor*>& result)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		if (vm->Is_Class_Derived_From(actor->Get_Script_Symbol(), type_of))
		{
			float distance = (actor->Get_World_Center() - position).Length();
			if (distance <= radius)
			{
				result.push_back(actor);
			}
		}
	}
}

void GameScene::Get_Scripted_Actors_Of_Type(CVMLinkedSymbol* type_of, std::vector<ScriptedActor*>& result)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	for (std::vector<ScriptedActor*>::iterator iter = m_scripted_actors.begin(); iter != m_scripted_actors.end(); iter++)
	{
		ScriptedActor* actor = (*iter);
		if (vm->Is_Class_Derived_From(actor->Get_Script_Symbol(), type_of))
		{
			result.push_back(actor);
		}
	}
}

Actor* GameScene::Create_Demo_Proxy_Actor(int demo_id) 
{
	DemoProxyActor* actor = new DemoProxyActor();
	actor->Set_Demo_ID(demo_id);
	Add_Actor(actor);

	return actor;
}

int GameScene::Get_Particle_Instigator_Team_Index(void* val)
{
	ScriptedActor* instigator = reinterpret_cast<ScriptedActor*>(val);
	if (instigator && instigator->Get_Owner())
	{
		return instigator->Get_Owner()->Get_State()->Team_Index;
	}
	return -1;
}
