// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Actors/Components/Collision/CollisionComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Particles/ParticleManager.h"

#include "Engine/Scene/Pathing/PathManager.h"

#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetClient.h"

#include "Game/Network/GameNetUser.h"

#include "Game/Scene/GameScene.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#ifndef MASTER_BUILD
//#define DAMAGE_DEBUG
#endif

CollisionComponent::CollisionComponent()
	: m_type(CollisionType::Solid)
	, m_shape(CollisionShape::Rectangle)
	, m_area(0, 0, 0, 0)
	, m_handle(NULL)
	, m_group(CollisionGroup::Environment)
	, m_collides_with(CollisionGroup::All)
	, m_colliding(false)
	, m_blocks_path(false)
	, m_blocks_path_spawns(false)
	, m_velocity(0.0f, 0.0f)
	, m_smoothed(true)
	, m_demo_instigator_team(-1)
{
}

CollisionComponent::~CollisionComponent()
{
	Scene* scene = GameEngine::Get()->Get_Scene();
	if (scene)
	{
		scene->Remove_Tickable(this);
	}

	m_script_object = NULL;
}

int CollisionComponent::Get_ID()
{
	static int hash = StringHelper::Hash("CollisionComponent");
	return hash;
}

bool CollisionComponent::Is_Used_In_Demo()
{
	return true;
}

void CollisionComponent::Serialize_Demo(BinaryStream* stream, DemoVersion::Type version, bool bSaving, float frameDelta)
{
	Component::Serialize_Demo(stream, version, bSaving, frameDelta);

	if (bSaving)
	{
		ScriptedActor* actor = dynamic_cast<ScriptedActor*>(m_parent);
		if (actor && actor->Get_Owner())
		{
			m_demo_instigator_team = actor->Get_Owner()->Get_State()->Team_Index;
		}
	}

	stream->Serialize<int>(bSaving, reinterpret_cast<int&>(m_type));
	stream->Serialize<int>(bSaving, reinterpret_cast<int&>(m_shape));
	stream->Serialize<int>(bSaving, reinterpret_cast<int&>(m_group));
	stream->Serialize<int>(bSaving, reinterpret_cast<int&>(m_collides_with));
	stream->Serialize<float>(bSaving, m_velocity.X);
	stream->Serialize<float>(bSaving, m_velocity.Y);
	stream->Serialize<float>(bSaving, m_area.X);
	stream->Serialize<float>(bSaving, m_area.Y);
	stream->Serialize<float>(bSaving, m_area.Width);
	stream->Serialize<float>(bSaving, m_area.Height);
	stream->Serialize<int>(bSaving, m_demo_instigator_team);
}

void CollisionComponent::Set_Type(CollisionType::Type value)
{
	m_type = value;
}

CollisionType::Type	CollisionComponent::Get_Type()
{
	return m_type;
}

void CollisionComponent::Set_Shape(CollisionShape::Type value)
{
	m_shape = value;
}

CollisionShape::Type CollisionComponent::Get_Shape()
{
	return m_shape;
}

void CollisionComponent::Set_Area(Rect2D value)
{
	m_area = value;
}

Rect2D CollisionComponent::Get_Area()
{
	return m_area;
}

void CollisionComponent::Set_Velocity(Vector2 value)
{
	m_velocity = value;
}

Vector2 CollisionComponent::Get_Velocity()
{
	return m_velocity;
}

void CollisionComponent::Set_Group(CollisionGroup::Type value)
{
	m_group = value;
}

CollisionGroup::Type CollisionComponent::Get_Group()
{
	return m_group;
}

void CollisionComponent::Set_Collides_With(CollisionGroup::Type value)
{
	m_collides_with = value;
}

CollisionGroup::Type CollisionComponent::Get_Collides_With()
{
	return m_collides_with;
}

bool CollisionComponent::Is_Colliding()
{
	return m_colliding;
}

void CollisionComponent::Set_Blocks_Path(bool value)
{
	m_blocks_path = value;
}

bool CollisionComponent::Get_Blocks_Path()
{
	return m_blocks_path;
}

void CollisionComponent::Set_Smoothed(bool value)
{
	m_smoothed = value;
}

bool CollisionComponent::Get_Smoothed()
{
	return m_smoothed;
}

void CollisionComponent::Set_Blocks_Path_Spawns(bool value)
{
	m_blocks_path_spawns = value;
}

bool CollisionComponent::Get_Blocks_Path_Spawns()
{
	return m_blocks_path_spawns;
}

void CollisionComponent::Collect_Deferred(const FrameTime& time)
{
	if (!m_active)
	{
		return;
	}

	GameNetClient* client = GameNetManager::Get()->Game_Client();
	bool in_editor_mode = client != NULL && client->Get_Server_State().In_Editor_Mode;

	if (in_editor_mode)
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	ScriptedActor* actor = dynamic_cast<ScriptedActor*>(m_parent);
	
	// When replaying a demo we use very simplified collision. Which means the replay
	// is not accurate, but close enough for most uses.
	if (actor == NULL)
	{
		if (m_handle != NULL)
		{
			CollisionEvent* first_event;
			int count = m_handle->Get_Events(first_event);

			for (int i = 0; i < count; i++)
			{
				if (first_event == NULL)
				{
					DBG_LOG("Number of events in collision handle linked list was lowe than event count! Somethings gone wrong somewhere!");
					break;
				}

				CollisionEvent& evt = *first_event;
				first_event = first_event->Next;

				// Ignore self-collisions.
				CollisionComponent* other_component = reinterpret_cast<CollisionComponent*>(evt.To->Get_Meta_Data());
				if (other_component != NULL && m_parent == other_component->m_parent)
				{
					continue;
				}

				bool is_damage = (evt.To->Get_Group() == CollisionGroup::Damage);
				if (is_damage)
				{
					ParticleInstance* particle = reinterpret_cast<ParticleInstance*>(evt.To->Get_Instigator());

					// If particle has died, ignore this collision or half the data we get
					// back may be bad.
					if (particle->incarnation != evt.To->Get_Instigator_Incarnation())
					{
						continue;
					}

					// Check if instigator is on different team than our instigator. If so,
					// then assume damage is accepted. Or if we have no team assign, assume accepted (crates etc).
					if (particle->demo_instigator_team != this->m_demo_instigator_team ||
						this->m_demo_instigator_team == -1)
					{
						evt.To->Set_Damage_Accepted_Flag();
						evt.From->Set_Damage_Accepted_Flag();
					}
				}
			}
		}
		
		return;
	}

	// Flag all current collisions as being untouched.
	for (std::vector<StoredCollision>::iterator iter = m_stored_collision.begin(); iter != m_stored_collision.end(); iter++)
	{
		(*iter).Touched = false;
	}

	m_colliding = false;

	if (m_handle != NULL)
	{
		// Throw collision events to the parents script.
		CollisionEvent* first_event;
		int count = m_handle->Get_Events(first_event);
	
		if (actor != NULL && count > 0 && m_script_object.Get().Get() != NULL)
		{		
			CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());
			ScriptEventListener* listener = actor->Get_Event_Listener();

			for (int i = 0; i < count; i++)
			{
				// No idea how this can occur event-count and list should match, but it seems to occassionally not,
				// dump out logs till we figure this one out.
				if (first_event == NULL)
				{
					DBG_LOG("Number of events in collision handle linked list was lowe than event count! Somethings gone wrong somewhere!");
					break;
				}

				CollisionEvent& evt = *first_event;
				first_event = first_event->Next;

				// Ignore self-collisions.
				CollisionComponent* other_component = reinterpret_cast<CollisionComponent*>(evt.To->Get_Meta_Data());
				if (other_component != NULL && m_parent == other_component->m_parent)
				{
					continue;
				}

				if (evt.To->Get_Type() == CollisionType::Solid)
				{
					m_colliding = true;
				}

				// Damage events have to be processed differently from most.
				bool is_damage = (evt.To->Get_Group() == CollisionGroup::Damage);				
				if (is_damage)
				{
					ParticleInstance* particle = reinterpret_cast<ParticleInstance*>(evt.To->Get_Instigator());

					// If particle has died, ignore this collision or half the data we get
					// back may be bad.
					if (particle->incarnation != evt.To->Get_Instigator_Incarnation())
					{
						continue;
					}

					// If we have a particle as an instigator, grab the info from it.
					if (particle != NULL)
					{					
						ParticleFX_EmitterConfig* emitter = particle->emitter_type;

						bool new_collision = true;
						bool apply_damage = false;
						float damage = 0.0f;

						// If damage is constant always apply.
						if (emitter->constant_damage == true)
						{
							apply_damage = true;
							damage = particle->damage * time.Get_Delta();

						//	DBG_ASSERT(damage != 0.0f);
						}

						// Otherwise only apply on first touch.
						else
						{
							// Is this a new touch event?
							for (std::vector<StoredCollision>::iterator iter = m_stored_collision.begin(); iter != m_stored_collision.end(); iter++)
							{
								StoredCollision& col = *iter;
								if (col.Particle == particle->incarnation)
								{
									col.Touched = true;
									new_collision = false;
									break;
								}
							}

							if (new_collision)
							{
								StoredCollision col;
								col.Touched = true;
								col.Particle = particle->incarnation;
								col.Other = NULL;
								col.Persist_Frames = 5; // We persist particle collision for a few frames so you don't end up walking in/out of the collision area and getting multi-damage.

								m_stored_collision.push_back(col);

								apply_damage = true;
								damage = particle->damage;

							//	DBG_ASSERT(damage != 0.0f);
							}
						}

						// TODO: Check if instigator still valid at this point? Use IDs instead?
						ScriptedActor* instigator = NULL;
						if (particle->instigator != NULL && particle->instigator_incarnation == particle->instigator->Get_Incarnation())
						{
							instigator = reinterpret_cast<ScriptedActor*>(particle->instigator->Get_Instigator()); /// fffffffffff
						}

						// Actually apply damage.
						if (apply_damage)
						{
							//if (instigator != NULL)
							//{
								Vector3 to_center = evt.To->Get_Center();
								Vector3 from_center = evt.To->Get_Old_Center();
								
								// We aren't moving, so work out direction based on to/from center.
								if (to_center == from_center)
								{
									to_center = evt.To->Get_Center();
									from_center = evt.From->Get_Center();
								}

								// Beam based weapons always set the position to the center of the destination. We
								// should do it based on intersection point but eh.
								if (evt.To->Get_Shape() == CollisionShape::Line)
								{
									to_center = from_center;
									from_center = Vector3(evt.To->Get_Area().X, evt.To->Get_Area().Y, 0.0f);
								//	from_center = evt.To->Get_Intersection_Point();
								//	DBG_LOG("POS=%f,%f INTER=%f,%f", evt.To->Get_Center().X, evt.To->Get_Center().Y, from_center.X, from_center.Y)
								}

								float dir = atan2(from_center.Y - to_center.Y, from_center.X - to_center.X) - HALFPI;

								// Construct position object.
								CVMObjectHandle pos = vm->Create_Vec3(to_center.X, to_center.Y, to_center.Z);
								
								// Scale explosive damage by distance (TODO: Do in script).
								if (particle->emitter_type->damage_type == ParticleFX_DamageType::Explosive)
								{
									static const float kill_radius_fraction = 0.3f;

									float double_radius = particle->emitter_type->damage_radius;// * 2;

									float distance = (from_center - to_center).Length();
									float kill_radius = double_radius * kill_radius_fraction; // Inner 30% of explosion radius is kill-zone.
									float fade_radius = double_radius - kill_radius;

									float damage_delta = 0.0f;
									if (distance < kill_radius)
									{
										damage_delta = 1.0f;
									}
									if (distance >= kill_radius && distance <= double_radius)
									{
										damage_delta = 1.0f - ((distance - kill_radius) / fade_radius);
									}

									damage_delta = Clamp(damage_delta, 0.0f, 1.0f);

									damage *= damage_delta;

									// Ignore no-damage.
									if (fabs(damage) < 0.01f)
									{
										continue;
									}
								}

								// Fire event!
								listener->On_Damage(
									m_script_object,
									instigator == NULL ? NULL : instigator->Get_Script_Object(),
									pos,
									dir,
									damage,
									(int)ParticleFX_DamageType::Get_Collision_Damage_Type(particle->emitter_type->damage_type),
									particle->ricochet_count,
									particle->penetration_count,
									particle->subtype,
									particle->weapon_type // STAT_TODO
								);		

								// Did actor accept damage?
								CVMValue retval;
								vm->Get_Return_Value(retval);
								if (retval.int_value == 1)
								{
#ifdef DAMAGE_DEBUG
									DBG_LOG("'%s' accepted %.2f damage from '%s'.", 
										m_script_object.Get().Get()->Get_Symbol()->symbol->name, 
										damage, 
										instigator == NULL ? "Environment" : instigator->Get_Script_Object().Get()->Get_Symbol()->symbol->name);
#endif

									evt.To->Set_Damage_Accepted_Flag();
									evt.From->Set_Damage_Accepted_Flag();
								}
								else
								{
#ifdef DAMAGE_DEBUG
									DBG_LOG("'%s' rejected %.2f damage from '%s'.", 
										m_script_object.Get().Get()->Get_Symbol()->symbol->name, 
										damage, 
										instigator == NULL ? "Environment" : instigator->Get_Script_Object().Get()->Get_Symbol()->symbol->name);
#endif
								}
							//}
						}
						else
						{
#ifdef DAMAGE_DEBUG
							DBG_LOG("'%s' not applying %.2f damage from '%s'.", 
								m_script_object.Get().Get()->Get_Symbol()->symbol->name, 
								damage, 
								instigator == NULL ? "Environment" : instigator->Get_Script_Object().Get()->Get_Symbol()->symbol->name);
#endif
						}
					}
					else 
					{
						// TODO: Actor based damage, not just particle based. Damage zones etc?
					}
				}
				else
				{
					CollisionComponent* other_component2 = reinterpret_cast<CollisionComponent*>(evt.To->Get_Meta_Data());
					if (other_component2 != NULL)
					{
						ScriptedActor* other_actor = dynamic_cast<ScriptedActor*>(other_component2->Get_Parent());
						if (other_actor != NULL)
						{
							switch (evt.Type)
							{
							case CollisionEventType::Touch:
								{
									if (m_type == CollisionType::Solid)
									{
										listener->On_Touch(m_script_object, other_actor->Get_Script_Object());
									}
									else
									{
										bool new_collision = true;

										// Is this a new touch event?
										for (std::vector<StoredCollision>::iterator iter = m_stored_collision.begin(); iter != m_stored_collision.end(); iter++)
										{
											StoredCollision& col = *iter;
											if (col.Other == other_actor)
											{
												col.Touched = true;
												new_collision = false;
												break;
											}
										}

										if (new_collision == true)
										{
											listener->On_Touch(m_script_object, other_actor->Get_Script_Object());
											listener->On_Enter(m_script_object, other_actor->Get_Script_Object());

											StoredCollision col;
											col.Touched = true;
											col.Other = other_actor;
											col.Persist_Frames = 0;
											col.Particle = -1;

											m_stored_collision.push_back(col);
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}

		// Apply position change if not non-solid.
		if (m_type != CollisionType::Non_Solid)
		{
			Vector3 result = m_handle->Get_Position();
			Get_Parent()->Set_Position(result);
		}
	}

	// All untouched collision have now left, so fire leave event and remove them.
	for (std::vector<StoredCollision>::iterator iter = m_stored_collision.begin(); iter != m_stored_collision.end(); )
	{
		StoredCollision& col = *iter;

		if (col.Touched == false && col.Persist_Frames <= 0)
		{
			ScriptedActor* other_actor = dynamic_cast<ScriptedActor*>(col.Other);

			if (other_actor != NULL)
			{
				CVMContextLock lock = vm->Set_Context(actor->Get_Script_Context());
				ScriptEventListener* listener = actor->Get_Event_Listener();

				listener->On_Leave(m_script_object, other_actor->Get_Script_Object());
			}

			iter = m_stored_collision.erase(iter);
		}
		else
		{
			col.Persist_Frames--;
			iter++;
		}
	}
}

void CollisionComponent::Tick(const FrameTime& time)
{
	if (!m_active)
	{
		return;
	}
}

void CollisionComponent::Create_Deferred(const FrameTime& time)
{
	if (!m_active)
	{
		return;
	}

	CollisionManager* manager = CollisionManager::Get();
	Actor* actor = dynamic_cast<Actor*>(Get_Parent());
	ScriptedActor* scripted_actor = dynamic_cast<ScriptedActor*>(Get_Parent());

	// Send new movement/current position to the collision manager.
	Vector3 position = actor->Get_Position();
	Vector3 last_position = position;

	if (scripted_actor != NULL)
	{
		if (scripted_actor->Is_Movement_Pending())
		{
			position = scripted_actor->Get_Target_Position();
		}
		scripted_actor->Clear_Pending_Movement();
	}

	// Push some work to the collision manager.
	m_handle = manager->Create_Handle(
		m_type, 
		m_shape,
		m_group,
		m_collides_with,
		m_area, 
		(m_shape == CollisionShape::Line) ? Vector3(0.0f, 0.0f, 0.0f) : position, 
		(m_shape == CollisionShape::Line) ? Vector3(0.0f, 0.0f, 0.0f) : last_position,
		false,
		this,
		NULL,
		0,
		m_velocity,
		m_smoothed);

	// Block paths if required.
	if (m_blocks_path == true)
	{
		PathManager* path_manager = PathManager::Get();
		
		Rect2D block_area = m_area;
		block_area.X += position.X;
		block_area.Y += position.Y;

		path_manager->Create_Blocker(block_area);
	}

	// Block path spawns if required.
	if (m_blocks_path_spawns == true)
	{
		PathManager* path_manager = PathManager::Get();

		Rect2D block_area = m_area;
		block_area.X += position.X;
		block_area.Y += position.Y;

		path_manager->Create_Blocker(block_area, true);
	}
}
