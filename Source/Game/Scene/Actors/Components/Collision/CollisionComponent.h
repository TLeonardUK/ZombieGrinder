// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_COLLISION_COLLISIONCOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_COLLISION_COLLISIONCOMPONENT_

#include "Engine/Renderer/Drawable.h"

#include "Game/Scene/Actors/Components/Drawable/DrawableComponent.h"

#include "Engine/Scene/Collision/CollisionManager.h"

#include "Engine/Scene/Animation.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

struct AtlasFrame;
struct AtlasAnimation;
struct CollisionHandle;
class ScriptedActor;
struct ParticleInstance;

class CollisionComponent : public Component, public Tickable
{
	MEMORY_ALLOCATOR(CollisionComponent, "Scene");

private:

	struct StoredCollision
	{
		int Particle;
		ScriptedActor* Other;
		bool Touched;
		int Persist_Frames;
	};

	bool					m_blocks_path;
	bool					m_blocks_path_spawns;
	bool					m_smoothed;

	CollisionType::Type		m_type;
	CollisionShape::Type	m_shape;
	Rect2D					m_area;
	CollisionGroup::Type	m_group;
	CollisionGroup::Type	m_collides_with;

	CollisionHandle*		m_handle;

	CVMGCRoot				m_script_object;

	Vector2					m_velocity;

	std::vector<StoredCollision>	m_stored_collision;

	int						m_demo_instigator_team;

	bool m_colliding;

public:
	CollisionComponent();
	virtual ~CollisionComponent();

	void Set_Script_Object(CVMObjectHandle handle)
	{
		m_script_object = handle;
	}
	
	std::string	Get_Script_Class_Name()
	{
		return "CollisionComponent";
	}

	virtual void Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta);

	virtual int Get_ID();
	virtual bool Is_Used_In_Demo();

	void					Set_Type			(CollisionType::Type value);
	CollisionType::Type		Get_Type			();
	void					Set_Shape			(CollisionShape::Type value);
	CollisionShape::Type	Get_Shape			();
	void					Set_Group			(CollisionGroup::Type value);
	CollisionGroup::Type	Get_Group			();
	void					Set_Collides_With	(CollisionGroup::Type value);
	CollisionGroup::Type	Get_Collides_With	();
	void					Set_Area			(Rect2D value);
	Rect2D					Get_Area			();
	void					Set_Blocks_Path		(bool value);
	bool					Get_Blocks_Path		();
	void					Set_Blocks_Path_Spawns		(bool value);
	bool					Get_Blocks_Path_Spawns		();
	void					Set_Smoothed(bool value);
	bool					Get_Smoothed();
	void					Set_Velocity		(Vector2 value);
	Vector2					Get_Velocity		();
	
	bool					Is_Colliding		();
	
	void Tick(const FrameTime& time);
	void Collect_Deferred(const FrameTime& time);
	void Create_Deferred(const FrameTime& time);
	
};

#endif

