// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_SCRIPTED_ACTOR_
#define _ENGINE_SCENE_ACTORS_SCRIPTED_ACTOR_

#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Tickable.h"
#include "Engine/Renderer/Drawable.h"

#include "Game/Scene/Actors/CompositeActor.h"

#include "Game/Scripts/GameVirtualMachine.h"

#include "XScript/VirtualMachine/CVMContext.h"

#include "Game/Network/ActorReplicationInfo.h"

#include "Game/Scripts/ScriptEventListener.h"

#include "Engine/Particles/ParticleManager.h"

#include "Game/Scene/Map/Blocks/MapFileObjectBlock.h"

#include "Generic/Types/CircleBuffer.h"

class GameNetUser;

struct Directions
{
	enum Type
	{
		S,
		SE,
		E,
		NE,
		N,
		NW,
		W,
		SW
	};
};

static const char* Direction_Short_Mnemonics[] = 
{
	"s",
	"se",
	"e",
	"ne",
	"n",
	"nw",
	"w",
	"sw"
};

class ScriptedActor : 
	public CompositeActor,
	public Tickable
{
	MEMORY_ALLOCATOR(ScriptedActor, "Scene");

private:
	CVirtualMachine*		m_virtual_machine;

	CVMLinkedSymbol*		m_object_class;
	CVMContext*				m_script_context;
	CVMGCRoot				m_script_object;

	ScriptedActor*			m_parent;

	bool					m_is_pawn;

	bool					m_hibernating;

	int						m_last_state_change_counter;
	ScriptEventListener*	m_event_listener;

	ActorReplicationInfo	m_replication_info;

	GameNetUser*			m_owner;

	Vector3					m_target_position;
	bool					m_pending_movement;
	
	bool					m_limit_movement;
	Rect2D					m_movement_limits;

	std::vector<std::string>	m_tags;
	std::string					m_tag;
	std::string					m_link;

	u32						m_map_id;
	
	ParticleInstigatorHandle*	m_particle_instigator_handle;

	bool					m_deconstructed;

	Vector3					m_last_frame_movement;
	Vector3					m_last_frame_position;

	bool					m_ignore_pending_movement_this_frame;

	bool					m_is_relevant_to_demo;
	bool					m_is_relevant_to_demo_calculated;

protected:

public:
	ScriptedActor(CVirtualMachine* vm, CVMLinkedSymbol* class_symbol);
	virtual ~ScriptedActor();

	virtual bool Is_Relevant_To_Demo();
	virtual OnlineUser* Get_Demo_Owner();

	u32 Get_Map_ID()
	{
		return m_map_id;
	}

	void Set_Map_ID(u32 id)
	{
		m_map_id = id;
	}

	void Set_Tag(std::string tag)
	{
		m_tag = tag;
		StringHelper::Split(m_tag.c_str(), ',', m_tags);
	}
	std::string Get_Tag()
	{
		return m_tag;
	}
	std::vector<std::string> Get_Tags()
	{
		return m_tags;
	}

	void Set_Link(std::string tag)
	{
		m_link = tag;
	}
	std::string Get_Link()
	{
		return m_link;
	}

	ParticleInstigatorHandle* Get_Particle_Instigator_Handle();
	void Alloc_Particle_Instigator_Handle();

	void Deconstruct();

	Rect2D Get_Tick_Offscreen_Bounding_Box();

	GameNetUser* Get_Owner();
	void Set_Owner(GameNetUser* user);

	ScriptedActor* Get_Parent();
	void Set_Parent(ScriptedActor* user);

	ActorReplicationInfo& Get_Replication_Info();

	void Tick(const FrameTime& time);

	void Hibernate();
	void WakeUp();
	bool Is_Hibernating();

	CVMObjectHandle Get_Script_Object();
	CVMContext* Get_Script_Context();

	Rect2D Get_World_Collision_Box();
	Vector3 Get_Collision_Center();
	
	INLINE CVMLinkedSymbol* Get_Script_Symbol()
	{
		return m_object_class;
	}

	Rect2D Get_Screen_Bounding_Box();

	ScriptEventListener* Get_Event_Listener();

	void Setup_Script();
	void Setup_Event_Hooks();
	
	// Movement position.
	void MoveTo(Vector3 pos);
	void Teleport(Vector3 pos);
	void LimitMovement(Rect2D movement);
	Vector3 Get_Target_Position();
	bool Is_Movement_Pending();
	void Clear_Pending_Movement();
	Vector3 Get_Last_Frame_Movement();

	// Serialization.
	void Serialize(MapFileObjectBlock* block);
	static ScriptedActor* Deserialize(MapFileObjectBlock* block);

};

#endif

