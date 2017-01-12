// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Game/Scene/Actors/Components/Collision/CollisionComponent.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Renderer/Drawable.h"
#include "Engine/Scene/Tickable.h"

#include "Game/Runner/Game.h"
#include "Game/Scene/GameScene.h"

#include "Game/Network/ActorReplicator.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Engine/Particles/ParticleManager.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CSymbol.h"

ScriptedActor::ScriptedActor(CVirtualMachine* vm, CVMLinkedSymbol* class_symbol)
	: m_object_class(class_symbol)
	, m_virtual_machine(vm)
	, m_script_context(NULL)
	, m_script_object(NULL)
	, m_hibernating(false)
	, m_last_state_change_counter(-1)
	, m_owner(NULL)
	, m_event_listener(NULL)
	, m_target_position(0, 0, 0)
	, m_pending_movement(false)
	, m_limit_movement(false)
	, m_parent(NULL)
	, m_tag("")
	, m_link("")
	, m_particle_instigator_handle(NULL)
	, m_deconstructed(false)	
	, m_last_frame_movement(0.0f, 0.0f, 0.0f)
	, m_last_frame_position(0.0f, 0.0f, 0.0f)
	, m_ignore_pending_movement_this_frame(false)
	, m_is_pawn(false)
	, m_is_relevant_to_demo(false)
	, m_is_relevant_to_demo_calculated(false)
{
	if (m_object_class->symbol->class_data->is_replicated)
	{
		ActorReplicator::Get()->Spawn_Actor(this);
	}

	CVMLinkedSymbol* pawn_class = vm->Find_Class("Pawn");
	m_is_pawn = vm->Is_Class_Derived_From(m_object_class, pawn_class);
}

ScriptedActor::~ScriptedActor()
{
	//DBG_LOG("Destructor invoked for object %i.", m_replication_info.unique_id);

	if (m_deconstructed == false)
	{
		Deconstruct();
	}
	// We have to acquire a lock so other threads do not attempt to acesss the vm while we 
	// instantiate new contexts.
	{
		CVMContextLock lock = m_virtual_machine->Set_Context(m_virtual_machine->Get_Static_Context());
		m_virtual_machine->Free_Context(m_script_context);
		m_script_context = NULL;
	}

	SAFE_DELETE(m_event_listener);
}

OnlineUser* ScriptedActor::Get_Demo_Owner()
{
	if (m_owner != NULL && m_is_pawn)
	{		
		return m_owner->Get_Online_User();
	}
	return NULL;
}

bool ScriptedActor::Is_Relevant_To_Demo()
{
	if (!m_is_relevant_to_demo_calculated)
	{
		m_is_relevant_to_demo = false;

		// We must be replicated.
		ScriptedActor* actor = this;
		while (actor)
		{
			if (!!actor->m_object_class->symbol->class_data->is_replicated)
			{
				m_is_relevant_to_demo = true;
				break;
			}
			actor = actor->Get_Parent();
		}

		// Or if its tagged (we assume tagged actors are going to be used in events etc, to show/hide decals.)
		if (m_tag != "")
		{
			m_is_relevant_to_demo = true;
		}

		// And we must have a visible component. Demo dosen't give a dam about logic components :)
		if (m_is_relevant_to_demo)
		{
			bool bHasVisibleComponents = false;

			for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
			{
				if (m_components[i] != NULL && m_components[i]->Is_Used_In_Demo())
				{
					bHasVisibleComponents = true;
					break;
				}
			}

			if (!bHasVisibleComponents)
			{
				m_is_relevant_to_demo = false;
			}
		}

		if (m_is_relevant_to_demo)
		{
		//	DBG_LOG("Actor '%s' is demo relevant.", m_object_class->symbol->name);
		}

		m_is_relevant_to_demo_calculated = true;
	}

	return m_is_relevant_to_demo;
}

ParticleInstigatorHandle* ScriptedActor::Get_Particle_Instigator_Handle()
{
	return m_particle_instigator_handle;
}

void ScriptedActor::Alloc_Particle_Instigator_Handle()
{
	if (m_particle_instigator_handle == NULL)
	{
		m_particle_instigator_handle = ParticleManager::Get()->Alloc_Instigator_Handle(this);
	}
}

void ScriptedActor::Deconstruct()
{
	Scene* scene = GameEngine::Get()->Get_Scene();

	if (m_deconstructed)
	{
		return;
	}

	{
		PRINTF_PROFILE_SCOPE("Dispose Components");

		if (m_event_listener != NULL)
		{
			m_event_listener->Set_Enabled(false);
		}

		scene->Remove_Tickable(this);

		for (int i = 0; i < MAX_COMPOSITE_ACTOR_COMPONENTS; i++)
		{
			if (m_components[i] != NULL)
			{
				IDrawable* drawable = dynamic_cast<IDrawable*>(m_components[i]);
				if (drawable != NULL)
				{
					scene->Remove_Drawable(drawable);
				}
				Tickable* tickable = dynamic_cast<Tickable*>(m_components[i]);
				if (tickable != NULL)
				{
					scene->Remove_Tickable(tickable);
				}
			}
		}

		// Remove controller from any pawns possessing us.
		std::vector<NetUser*>& users = GameNetManager::Get()->Get_Net_Users();

		for (std::vector<NetUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
		{
			GameNetUser* user = static_cast<GameNetUser*>(*iter);
			if (user->Get_Controller() == this)
			{
				user->Set_Controller_ID(-1);
			}
		}
	}

	// Remove instigator flag from all particles created by us.
	if (m_particle_instigator_handle != NULL)
	{
		PRINTF_PROFILE_SCOPE("Free Particles");
		ParticleManager::Get()->Free_Instigator_Handle(m_particle_instigator_handle);
		m_particle_instigator_handle = NULL;
	}

	// Remove actor from scene.
	{
		PRINTF_PROFILE_SCOPE("Remove_Scripted_Actor");
		Game::Get()->Get_Game_Scene()->Remove_Scripted_Actor(this);
	}

	// Remove replicated information, 99% of the time this will already have been
	// done and will be what is invoking this destructor. But just to be safe.
	if (m_object_class->symbol->class_data->is_replicated)
	{ 
		PRINTF_PROFILE_SCOPE("Purge Actor");
		ActorReplicator::Get()->Purge_Actor(this, false);
	}

	// Set object handle on object to null. Will end up
	// with null-access violations if scripts try to access this
	// after this point. Scene.Is_Spawned can check for this.
	{
		PRINTF_PROFILE_SCOPE("Set Meta Data");

		m_script_object.Get().Get()->Set_Meta_Data(NULL);
		m_script_object = NULL;

		m_owner = NULL;

		m_deconstructed = true;
	}
}
/*
Rect2D ScriptedActor::Get_Screen_Bounding_Box()
{
	Rect2D world_bb = Get_World_Bounding_Box();
	Camera* active_cam = Game::Get()->Get_Camera(CameraID::Game1); // SPLITSCREEN_TODO
	Camera* screen_cam = Game::Get()->Get_Camera(CameraID::UI);
	Rect2D screen_bb = screen_cam->Get_Viewport();

	if (active_cam == NULL || screen_cam == NULL)
	{
		return world_bb;
	}

	Vector2 top_left = active_cam->Project(Vector2(world_bb.X, world_bb.Y), screen_bb);
	Vector2 bottom_right = active_cam->Project(Vector2(world_bb.X + world_bb.Width, world_bb.Y + world_bb.Height), screen_bb);

	Rect2D screen_bbox = Rect2D
	(
		top_left.X,
		top_left.Y,
		bottom_right.X - top_left.X,
		bottom_right.Y - top_left.Y
	);

	return screen_bbox;
}
*/

Vector3 ScriptedActor::Get_Collision_Center()
{
	CollisionComponent* component = Get_Component<CollisionComponent*>();
	if (component != NULL)
	{
		Rect2D area = component->Get_Area();
		Vector3 pos = Get_Position() + Vector3(area.X + (area.Width * 0.5f), area.Y + (area.Height * 0.5f), 0.0f);
		return pos;
	}
	else
	{
		return Get_World_Center();
	}
}

Rect2D ScriptedActor::Get_World_Collision_Box()
{
	CollisionComponent* component = Get_Component<CollisionComponent*>();
	if (component != NULL)
	{
		Rect2D area = component->Get_Area();
		Vector3 pos = Get_Position();
		return Rect2D(pos.X + area.X, pos.Y + area.Y, area.Width, area.Height);
	}
	else
	{
		return Get_World_Bounding_Box();
	}
}

GameNetUser* ScriptedActor::Get_Owner()
{
	return m_owner;
}

void ScriptedActor::Set_Owner(GameNetUser* user)
{
	m_owner = user;
}

ActorReplicationInfo& ScriptedActor::Get_Replication_Info()
{
	return m_replication_info;
}

CVMObjectHandle ScriptedActor::Get_Script_Object()
{
	return m_script_object;
}

CVMContext* ScriptedActor::Get_Script_Context()
{
	return m_script_context;
}

Rect2D ScriptedActor::Get_Tick_Offscreen_Bounding_Box()
{
	return Get_World_Bounding_Box();
}

void ScriptedActor::Hibernate()
{
	//DBG_ASSERT(!m_hibernating); /// Eeeeh, should this be an assert?
	m_hibernating = true;
	Set_Enabled(false);
}

void ScriptedActor::WakeUp()
{
	//DBG_ASSERT(m_hibernating); /// Eeeeh, should this be an assert?
	m_hibernating = false;
	Set_Enabled(true);
}

bool ScriptedActor::Is_Hibernating()
{
	return m_hibernating;
}

void ScriptedActor::Setup_Script()
{
	// We have to acquire a lock so other threads do not attempt to acesss the vm while we 
	// instantiate new contexts.
	{
		CVMContextLock lock = m_virtual_machine->Set_Context(m_virtual_machine->Get_Static_Context());

		m_script_context = m_virtual_machine->New_Context(this);
		DBG_ASSERT(m_script_context != NULL);
	}

	// Instantiate the new object with objects context so component constructors
	// etc know what they are working on.
	{
		CVMContextLock lock = m_virtual_machine->Set_Context(m_script_context);

		// New script object, oooooohhhh yeeeeh.
		m_script_object = m_virtual_machine->New_Object(m_object_class, true, this);
		DBG_ASSERT(m_script_object.Get().Get() != NULL);
	}
}

void ScriptedActor::Setup_Event_Hooks()
{
	m_event_listener = new ScriptEventListener(m_script_context, m_script_object, this);
}

ScriptEventListener* ScriptedActor::Get_Event_Listener()
{
	return m_event_listener;
}

Vector3 ScriptedActor::Get_Last_Frame_Movement()
{
	return m_last_frame_movement;
}

void ScriptedActor::Tick(const FrameTime& time)
{
	// Send the script it's On_Tick event!
	m_event_listener->On_Tick();

	m_last_frame_movement = m_position - m_last_frame_position;
	m_last_frame_position = m_position;
}

void ScriptedActor::MoveTo(Vector3 pos)
{
	// If we have collision, defer the movement, our collision component will take care of it.
	CollisionComponent* component = Get_Component<CollisionComponent*>();
	if (component != NULL)
	{
		m_target_position = pos;
		m_pending_movement = true;
	}
	else
	{
		m_position = pos;
	}
}

void ScriptedActor::Teleport(Vector3 pos)
{
	m_position = pos;
	m_ignore_pending_movement_this_frame = true;
}

void ScriptedActor::LimitMovement(Rect2D movement)
{
	m_limit_movement = true;
	m_movement_limits = movement;
}

Vector3 ScriptedActor::Get_Target_Position()
{
	Vector3 offset = Vector3(0, 0, 0);

	if (m_limit_movement == true)
	{
		Rect2D bb = Rect2D(m_target_position.X + m_bounding_box.X, m_target_position.Y + m_bounding_box.Y, m_bounding_box.Width, m_bounding_box.Height);
		if (bb.X < m_movement_limits.X)
		{
			offset.X = -(bb.X - m_movement_limits.X);
		}
		if (bb.Y < m_movement_limits.Y)
		{
			offset.Y = -(bb.Y - m_movement_limits.Y);
		}
		if (bb.X + bb.Width > m_movement_limits.X + m_movement_limits.Width)
		{
			offset.X = (m_movement_limits.X + m_movement_limits.Width) - (bb.X + bb.Width);
		}
		if (bb.Y + bb.Height > m_movement_limits.Y + m_movement_limits.Height)
		{
			offset.Y = (m_movement_limits.Y + m_movement_limits.Height) - (bb.Y + bb.Height);
		}
	}

	return m_target_position + offset;
}

bool ScriptedActor::Is_Movement_Pending()
{
	return m_pending_movement && !m_ignore_pending_movement_this_frame;
}

void ScriptedActor::Clear_Pending_Movement()
{
	m_pending_movement = false;
	m_limit_movement = false;
	m_ignore_pending_movement_this_frame = false;
}

ScriptedActor* ScriptedActor::Get_Parent()
{
	return m_parent;
}

void ScriptedActor::Set_Parent(ScriptedActor* user)
{
	DBG_ASSERT_STR(m_script_object.Get().Get()->Get_Symbol()->symbol->class_data->is_replicated == false, "Only non-replicated objects can have parents.");
	m_parent = user;
	DBG_ASSERT(m_parent != this);
}

void ScriptedActor::Serialize(MapFileObjectBlock* block)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	block->Class_Name = Get_Script_Object().Get()->Get_Symbol()->symbol->name;

	BinaryStream bs;
	int output_version = 0;
	CVMContextLock lock = vm->Set_Context(Get_Script_Context());
	Get_Script_Object().Get()->Serialize(&bs, CVMObjectSerializeFlags::Full, &output_version);		
	block->Serialized_Data_Version = output_version;
	block->Serialized_Data.Set(bs.Data(), (int)bs.Length());				
	bs.Close();
}

ScriptedActor* ScriptedActor::Deserialize(MapFileObjectBlock* block)
{
	Map* map = Game::Get()->Get_Map();
	GameScene* scene = Game::Get()->Get_Game_Scene();

	ScriptedActor* scripted = block->Create_Instance();
	if (scripted != NULL)
	{
		if (!scripted->Get_Script_Object().Get()->Get_Symbol()->symbol->class_data->is_replicated)
			scripted->Set_Map_ID(map->Create_Map_ID());
		scripted->Set_Selected(false);

		scene->Add_Actor(scripted);
		scene->Add_Tickable(scripted);
	}

	return scripted;
}