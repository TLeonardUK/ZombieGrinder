// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_
#define _GAME_SCENE_

#include "Engine/Scene/Scene.h"

#include "Generic/Types/HashTable.h"

class ScriptedActor;
struct CVMLinkedSymbol;
class GameNetUser;

class GameScene : public Scene
{
	MEMORY_ALLOCATOR(GameScene, "GameScene");

private:
	struct DespawnRequest
	{
		ScriptedActor* actor;
		int			   defer_frames;
	};

	std::vector<DespawnRequest> m_despawn_actors;
	std::vector<ScriptedActor*> m_scripted_actors;

	struct ActorBucket
	{
		std::vector<ScriptedActor*> actors;
	};

	HashTable<ActorBucket, unsigned int> m_scripted_actor_types;

	Rect2D m_full_camera_bounding_box;

	bool m_editor_force_visible;

	enum 
	{
		despawn_defer_frames = 2 // we defer despawns for a couple of frames so any references to the actors should
								 // be cleared out of any defered actions.
	};

public:
	GameScene()
		: m_editor_force_visible(false)
	{
	}

	virtual void Unload();

	bool Get_Editor_Force_Hide()
	{
		return m_editor_force_visible;
	}

	void Set_Editor_Force_Hide(bool val)
	{
		m_editor_force_visible = val;
	}

	virtual Actor* Create_Demo_Proxy_Actor(int demo_id);

	std::vector<ScriptedActor*> Get_Scripted_Actors();
	std::vector<ScriptedActor*> Find_Derived_Actors(CVMLinkedSymbol* symbol);
	ScriptedActor* Find_Actor_By_Unique_ID(u32 id);
	ScriptedActor* Get_Actor_By_Map_ID(u32 id);

	void Add_To_Scripted_Bucket(CVMLinkedSymbol* bucket, ScriptedActor* actor);
	void Remove_From_Scripted_Bucket(CVMLinkedSymbol* bucket, ScriptedActor* actor);

	virtual void Add_Actor(Actor* camera);
	virtual void Remove_Actor(Actor* camera);	
	virtual void Remove_Scripted_Actor(ScriptedActor* camera);
	virtual void Add_Scripted_Actor(ScriptedActor* scripted);

	ScriptedActor* Spawn(CVMLinkedSymbol* symbol, GameNetUser* owner);
	void Despawn(ScriptedActor* obj);
	void Despawn_Children(ScriptedActor* obj);

	void Register_Despawn(ScriptedActor* obj);

	std::vector<Actor*> Get_Actors_Of_Class(CVMLinkedSymbol* sym);
	
	void Calculate_Full_Camera_Bounding_Box();
	Rect2D Get_Full_Camera_Bounding_Box();

	void Get_Scripted_Actors_In_Radius(Vector3 position, float radius, std::vector<ScriptedActor*>& result);
	void Get_Scripted_Actors_In_Radius(Vector3 position, float radius, CVMLinkedSymbol* type_of, std::vector<ScriptedActor*>& result);
	void Get_Scripted_Actors_Of_Type(CVMLinkedSymbol* type_of, std::vector<ScriptedActor*>& result);

	void Run_Despawns();

	int Get_Particle_Instigator_Team_Index(void* instigator);

};

#endif

