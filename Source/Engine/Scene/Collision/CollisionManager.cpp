// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Collision/CollisionManager.h"
#include "Engine/Engine/GameEngine.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Threads/Atomic.h"

#include "Generic/Helper/CollisionHelper.h"

#include "Generic/Stats/Stats.h"

/*

	Ok, so collision manager works like this;

	The collision manager maintains a double buffered array of "collision handles", each 
	handle represents a different collidable object.

	Each frame every object that wants collision response allocates one of these handles
	using Create_Handle_*. The next frame the object reads back the new position from this handle
	and assigns it to its object. Handles only have a lifetime of 2 frames, so they can be deferred 
	to the time between frames (during rendering).

	Collision handles are sorted into a spare quad-tree. Collision is performed between objects and
	the resultant positions and events stored in the collision handle. 

*/

CollisionManager::CollisionManager()
	: m_write_buffer(0)
	, m_handle_count(0)
	, m_write_handle_count(0)
	, m_persistent_handle_count(0)
	, m_write_active_handle_count(0)
	, m_active_handle_count(0)
	, m_persistent_change_count(0)
	, m_persistent_changed(false)
	, m_show_collision(false)
{
	DBG_LOG("sizeof(CollisionManager) = %i kb.", sizeof(CollisionManager) / 1024);
	DBG_LOG("sizeof(CollisionHandle) = %i bytes.", sizeof(CollisionHandle));
	
	m_task = new CollisionSimulationTask();
	m_tree.Resize(grid_resolution, grid_range, max_grid_entries);
}

CollisionManager::~CollisionManager()
{
	SAFE_DELETE(m_task);
}

void CollisionManager::Reset()
{
	m_persistent_handle_count = 0;
	m_handle_count = 0;
	m_write_active_handle_count = 0;
	m_active_handle_count = 0;
	m_write_handle_count = 0;
	m_write_buffer = 0;
	m_persistent_change_count = 0;
	m_persistent_changed = false;
	m_tree.Clear();
}
	
bool CollisionManager::Get_Show_Collision()
{
	return m_show_collision;
}

void CollisionManager::Set_Show_Collision(bool toggle)
{
	m_show_collision = toggle;
}
	
CollisionHandle* CollisionManager::Create_Handle(
	CollisionType::Type type,				// Type of collision.
	CollisionShape::Type shape,				// Shape of collision bounds.
	CollisionGroup::Type group,				// Used to match up collision with other handles.
	CollisionGroup::Type collides_with,		// Groups of collision we collide with.
	Rect2D area,							// Shape area.
	Vector3 new_position,					// New target position.
	Vector3 old_position,					// Last good position.
	bool persistent,						// Persists for the entire level.
	void* meta_data,						// Metadata stored in the handle, useful for linking up handles to objects.	
	void* instigator,						// Mainly used for damage events.
	int instigator_incarnation,				// What cause this handle to be created - mainly for damage collision.
	Vector2 velocity,
	bool smoothed)
{
	CollisionHandle* handle = NULL;
	if (persistent == true)
	{
		DBG_ASSERT(m_persistent_handle_count < max_persistent_handles);
		handle = &m_persistent_handles[m_persistent_handle_count++];

		if (m_persistent_changed == false)
		{
			if (m_persistent_change_count > 0)
			{
				DBG_LOG("[WARNING] Persistent collision handles changed during play! These should only be modified on map start. Collision may be non-deterministic from here on.");
			}
			m_persistent_changed = true;
			m_persistent_change_count = 0;
		}
	}
	else
	{
		DBG_ASSERT(m_handle_count < max_collision_handles);
		handle = &m_handle_buffers[m_write_buffer][m_handle_count++];
	}

	handle->m_type = type;
	handle->m_shape = shape;
	handle->m_group = group;
	handle->m_collides_with = collides_with;
	handle->m_area = area;
	handle->m_start_position = new_position;
	handle->m_new_position = new_position;
	handle->m_old_position = old_position;
	handle->m_has_moved = (new_position != old_position);
	handle->m_event_count = 0;
	handle->m_first_event = NULL;
	handle->m_meta_data = meta_data;
	handle->m_damage_accepted = false;
	handle->m_was_touched = false;
	handle->m_was_considered = false;
	handle->m_instigator = instigator;
	handle->m_instigator_incarnation = instigator_incarnation;
	handle->m_velocity = Vector3(velocity.X, velocity.Y, 0.0f);
	handle->m_velocity_increment = Vector3(0.0f, 0.0f, 0.0f);
	handle->m_has_been_velocity_affected = false;
	handle->m_smoothed = smoothed;

	// TODO: fix.
	//if ((handle->m_has_moved && handle->m_collides_with != CollisionGroup::NONE) ||
	//	handle->m_type == CollisionType::Non_Solid)
	{
		DBG_ASSERT(m_active_handle_count < max_active_handles);
		m_active_handles[m_write_buffer][m_active_handle_count++] = handle;
	}

	return handle;
}

void CollisionManager::Tick(const FrameTime& time)
{
	GameEngine* engine = GameEngine::Get();
	TaskManager* manager = TaskManager::Get();

	// Swap buffers.
	m_write_buffer = 1 - m_write_buffer;
	m_write_handle_count = m_handle_count;
	m_handle_count = 0;
	m_write_active_handle_count = m_active_handle_count;
	m_active_handle_count = 0;
	m_simulate_delta = time.Get_Delta();

	// Clear stack.
	m_event_stacks[m_write_buffer].Free();

	// Add task for the next frame.
	m_task_id = manager->Add_Task(m_task, engine->Get_Frame_Deferred_TaskID());	
	manager->Queue_Task(m_task_id);
}

void CollisionManager::Draw(const FrameTime& time)
{
	if (!m_show_collision)
		return;

	// Ensure we have finished simulation before drawing debug info!
	TaskManager::Get()->Wait_For(m_task_id);

	PrimitiveRenderer pr;

	for (int i = 0; i < m_persistent_handle_count; i++)
	{
		CollisionHandle& handle = m_persistent_handles[i];
		Rect2D area
		(
			handle.m_old_position.X + handle.m_area.X,
			handle.m_old_position.Y + handle.m_area.Y,
			handle.m_area.Width,
			handle.m_area.Height
			);

		Color color;
		if (handle.m_was_touched == true)
			color = Color::Blue;
		else
			color = Color::Red;

		if (handle.m_shape == CollisionShape::Oval)
		{
			pr.Draw_Solid_Oval(area, color);
		}
		else if (handle.m_shape == CollisionShape::Circle)
		{
			pr.Draw_Solid_Oval(area, color);
		}
		else if (handle.m_shape == CollisionShape::Line)
		{
			DBG_ASSERT(false);
		}
		else if (handle.m_shape == CollisionShape::Rectangle)
		{
			pr.Draw_Wireframe_Quad(area, 10000.0f, color, 2.0f);
		}

		handle.m_was_touched  = false;
	}

	int read_buffer = 1 - m_write_buffer;

	for (int i = 0; i < m_write_handle_count; i++)
	{
		CollisionHandle& handle = m_handle_buffers[read_buffer][i];
		Rect2D area
		(
			handle.m_old_position.X + handle.m_area.X,
			handle.m_old_position.Y + handle.m_area.Y,
			handle.m_area.Width,
			handle.m_area.Height
		);

		Color color;
	//	if (handle.m_has_moved)
	//	{
	//		color = Color::Green;
	//	}
	//	else
		{
			if (handle.m_was_touched == true)
				color = Color::Blue;
			else if (handle.m_was_considered)
				color = Color::Green;
			else
			{
				if (handle.m_type == CollisionType::Solid)
				{
					color = Color::Gray;
				}
				else
				{
					color = Color::Red;
				}
			}
		}

		if (handle.m_shape == CollisionShape::Oval)
		{
			pr.Draw_Solid_Oval(area, color);
		}
		else if (handle.m_shape == CollisionShape::Circle)
		{
			pr.Draw_Solid_Oval(area, color);
		}
		else if (handle.m_shape == CollisionShape::Line)
		{
			pr.Draw_Line(Vector3(area.X, area.Y, 0.0f), Vector3(area.Width, area.Height, 0.0f), 2.0f, color);
		}
		else if (handle.m_shape == CollisionShape::Rectangle)
		{
			pr.Draw_Wireframe_Quad(area, 10000.0f, color, 2.0f);
		}
			
		handle.m_was_touched  = false;
		handle.m_was_considered = false;
	}

	// Draw debug lines/points
	for (int i = 0; i < (int)m_debug_lines.size(); i++)
	{
		Line2D& l = m_debug_lines[i];
		pr.Draw_Line(Vector3(l.Start.X, l.Start.Y, 0.0f), Vector3(l.End.X, l.End.Y, 0.0f), 2.0f, Color::Magenta);
	}
	for (int i = 0; i < (int)m_debug_points.size(); i++)
	{
		Vector3& l = m_debug_points[i];
		pr.Draw_Solid_Oval(Rect2D(l.X - 2.0f, l.Y - 2.0f, 5.0f, 5.0f), Color::Magenta);
	}

	m_debug_lines.clear();
	m_debug_points.clear();

	/*for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			pr.Draw_Wireframe_Quad(Rect2D(x * grid_resolution, y * grid_resolution, grid_resolution, grid_resolution), 10000.0f, Color::Yellow, 2.0f);
		}
	}*/
}

int CollisionManager::Get_Collisions(CollisionHandle* primary, PotentialCollision* results, int results_size)
{
	Rect2D area = primary->m_area;
	Rect2D world_area = area + primary->m_new_position;
	
	static CollisionHandle* possible[4096];
	static int possible_count = 4096;

	int result_count = 0;

	int count = m_tree.Find(primary, possible, possible_count);
	for (int j = 0; j < count; j++)
	{
		CollisionHandle* secondary = possible[j];
		Rect2D secondary_world_area = secondary->m_area + secondary->m_new_position;

		bool intersects = false;
		Vector3 intersection_point;

		switch (primary->m_shape)
		{
		case CollisionShape::Rectangle:
			{
				switch (secondary->m_shape)
				{
					case CollisionShape::Rectangle:	intersects = CollisionHelper<Rect2D, Rect2D>::Intersects(world_area, secondary_world_area, &intersection_point); break;
					case CollisionShape::Circle:	intersects = CollisionHelper<Circle2D, Rect2D>::Intersects(secondary_world_area, world_area, &intersection_point); break;
					case CollisionShape::Oval:		DBG_ASSERT(false);//intersects = CollisionHelper<Rect2D, Oval2D>::Intersects(world_area, secondary_world_area); break;
					case CollisionShape::Line:		intersects = CollisionHelper<Line2D, Rect2D>::Intersects(secondary_world_area, world_area, &intersection_point); break;
				}
				break;
			}
		case CollisionShape::Circle:
			{
				switch (secondary->m_shape)
				{
				case CollisionShape::Rectangle:	intersects = CollisionHelper<Circle2D, Rect2D>::Intersects(world_area, secondary_world_area, &intersection_point); break;
				case CollisionShape::Circle:	intersects = CollisionHelper<Circle2D, Circle2D>::Intersects(world_area, secondary_world_area, &intersection_point); break;
				case CollisionShape::Oval:		DBG_ASSERT(false);//intersects = CollisionHelper<Rect2D, Oval2D>::Intersects(world_area, secondary_world_area); break;
				case CollisionShape::Line:		intersects = CollisionHelper<Line2D, Circle2D>::Intersects(secondary_world_area, world_area, &intersection_point); break;
				}
				break;
			}
		case CollisionShape::Oval:
			{
				switch (secondary->m_shape)
				{
					case CollisionShape::Rectangle: DBG_ASSERT(false);//intersects = CollisionHelper<Rect2D, Oval2D>::Intersects(secondary_world_area, world_area); break;
					case CollisionShape::Circle:	DBG_ASSERT(false);//intersects = CollisionHelper<Oval2D, Circle2D>::Intersects(secondary_world_area, world_area); break;
					case CollisionShape::Oval:		DBG_ASSERT(false);//intersects = CollisionHelper<Oval2D, Oval2D>::Intersects(world_area, secondary_world_area); break;
					case CollisionShape::Line:		DBG_ASSERT(false);//intersects = CollisionHelper<Oval2D, Line2D>::Intersects(world_area, secondary_world_area); break;
				}
				break;
			}
		case CollisionShape::Line:
			{
				switch (secondary->m_shape)
				{
					case CollisionShape::Rectangle: intersects = CollisionHelper<Line2D, Rect2D>::Intersects(world_area, secondary_world_area, &intersection_point); break;
					case CollisionShape::Circle:	intersects = CollisionHelper<Line2D, Circle2D>::Intersects(world_area, secondary_world_area, &intersection_point); break;
					case CollisionShape::Oval:		DBG_ASSERT(false);//intersects = CollisionHelper<Oval2D, Line2D>::Intersects(secondary_world_area, world_area); break;
					case CollisionShape::Line:		intersects = CollisionHelper<Line2D, Line2D>::Intersects(secondary_world_area, world_area, &intersection_point); break;
				}
				break;
			}
		}

		if (intersects)
		{
			if (primary->m_shape == CollisionShape::Line)
			{
				m_debug_lines.push_back(Line2D(world_area));
				m_debug_points.push_back(Vector3(intersection_point));
			}

			if (secondary->Get_Type() == CollisionType::Solid ||
				primary->Get_Type() == CollisionType::Solid)
			{
				secondary->m_was_touched = true;
				primary->m_was_touched = true;
			}

			results[result_count].Handle = secondary;
			results[result_count].Intersection_Point = intersection_point;
			result_count++;
		
			if (result_count >= results_size)
			{
				return result_count;
			}
		}
	}

	return result_count;
}

void CollisionManager::Simulate()
{		
	PROFILE_SCOPE("Collision Simulation");

	double time = Platform::Get()->Get_Ticks();
	int find_counts = 0;

	int read_buffer = 1 - m_write_buffer;

	// Clear quad tree of non-persistent handles.
	m_tree.Clear_Non_Persistent();
	m_tree.Clear_Flags();

	// Add persistent handles if updated.
	if (m_persistent_changed == true)
	{
		// Will be non-deterministic if people try adding persistent handles
		// while we are in the middle of simulation!

		for (int i = 0; i < m_persistent_handle_count; i++)
		{
			CollisionHandle* primary = &m_persistent_handles[i];
			m_tree.Add(primary, true);
		}

		m_persistent_changed = false;
	}

	// Add non-persistant handles to quad tree. Do in two passes, particles second. This 
	// way if particles overwhelm collision buffers it shouldn't have any effect on "main" collisions.
	for (int i = 0; i < m_write_handle_count; i++)
	{
		CollisionHandle* primary = &m_handle_buffers[read_buffer][i];
		if (primary->Get_Group() != CollisionGroup::Particle)
		{
			m_tree.Add(primary, false);
		}
	}
	for (int i = 0; i < m_write_handle_count; i++)
	{
		CollisionHandle* primary = &m_handle_buffers[read_buffer][i];
		if (primary->Get_Group() == CollisionGroup::Particle)
		{
			m_tree.Add(primary, false);
		}
	}

	// Perform collision only for moving collisions.
	static PotentialCollision primary_collisions[4096];

	// TODO: Blah this multi-pass system is ugly, fux.
	// Pass 0 = Velocity affecting collision areas.
	// Pass 1 = Non-velocity affecting collision areas.
	for (int pass = 0; pass < 2; pass++)
	{
		for (int i = 0; i < m_write_active_handle_count; i++)
		{
			CollisionHandle* primary = m_active_handles[read_buffer][i];

			if (primary->m_velocity.X != 0.0f || primary->m_velocity.Y != 0.0f)
			{
				if (pass != 0)
				{
					continue;
				}
			}
			else
			{
				if (pass == 0)
				{
					continue;
				}
			}

			Vector3 old_position = primary->m_old_position;
			Vector3 new_position = primary->m_new_position + primary->m_velocity_increment;
			Vector3 result_position = new_position;

			if (primary->m_velocity_increment.X != 0.0f || primary->m_velocity_increment.Y != 0.0f)
			{
				primary->m_has_been_velocity_affected = true;
			}

			// Find all things we could possibly collide with.
			find_counts++;
			int collision_count = Get_Collisions(primary, primary_collisions, 4096);
			if (collision_count > 0)
			{				
				primary->m_was_considered = true;

				// Add touch events.
				for (int j = 0; j < collision_count; j++)
				{
					CollisionEvent* evt = m_event_stacks[m_write_buffer].Alloc();
					if (evt != NULL)
					{
						evt->Type	= CollisionEventType::Touch;
						evt->From	= primary;
						evt->To		= primary_collisions[j].Handle;
						evt->Point	= primary_collisions[j].Intersection_Point;

						evt->Next = primary->m_first_event;
						primary->m_first_event = evt;
						primary->m_event_count++;
					}
					else
					{
						DBG_LOG("[WARNING] Used up all collision events, ignoring collision! Might want to increase stack allocator size?");
					}
					/*
					if (primary_collisions[j].Handle->m_velocity_increment.X != 0.0f || primary_collisions[j].Handle->m_velocity_increment.Y != 0.0f)
					{
						DBG_LOG("CREATED VELOCITY INC EVENT: %i", collision_count);
						for (int k = 0; k < collision_count; k++)
						{
							DBG_LOG("\t[%i] 0x%08x", k, primary_collisions[k].Handle);
						}
					}
					*/
					primary_collisions[j].Handle->m_velocity_increment = primary_collisions[j].Handle->m_velocity_increment + (primary->m_velocity * m_simulate_delta);
				}

				const float SMOOTH_CORNER_DISTANCE = 8.0f;
				const float SMOOTH_PROBE_SIZE = 1.0f;

				switch (primary->m_type)
				{
				case CollisionType::Solid:
					{	
						if (old_position.X != new_position.X || old_position.Y != new_position.Y)
						{
							// Try moving along each axis individually, to allow user to slide.
							primary->m_new_position.X = new_position.X;
							primary->m_new_position.Y = old_position.Y;

							collision_count = Get_Collisions(primary, primary_collisions, 1);
							if (collision_count > 0)
							{
								primary->m_new_position.X = old_position.X;
								primary->m_new_position.Y = new_position.Y;

								collision_count = Get_Collisions(primary, primary_collisions, 1);
								if (collision_count > 0)
								{
									result_position = old_position;
								}
								else
								{
									result_position = primary->m_new_position;

									// Vertical axis is valid movement, horizontal is not.
									// If smoothed, then attempt to move along vertical axis with the velocity lost on the horizonal axis, to the nearest free collision area.
									if (primary->m_smoothed && fabs(old_position.Y - new_position.Y) < fabs(old_position.X - new_position.X) * 0.25f)
									{
										float lost_velocity = fabs(new_position.X - old_position.X) * 0.75f;

										bool bWasSmoothed = false;

										Rect2D original_area = primary->m_area;
										Rect2D single_px_area = Rect2D(original_area.X, original_area.Center().Y, original_area.Width, SMOOTH_PROBE_SIZE);

										// Look upwards to see if we are on a corner.
										primary->m_area = single_px_area;
										primary->m_new_position.X = new_position.X;
										primary->m_new_position.Y = new_position.Y - SMOOTH_CORNER_DISTANCE;
										if (Get_Collisions(primary, primary_collisions, 1) <= 0)
										{
											primary->m_area = original_area;
											primary->m_new_position.X = old_position.X;
											primary->m_new_position.Y = new_position.Y - lost_velocity;

											// Check new up position is valid.
											if (Get_Collisions(primary, primary_collisions, 1) <= 0)
											{
												result_position = primary->m_new_position;
												bWasSmoothed = true;
											}
										}
										else 
										{
											// Look downwards to see if we are on corner.
											primary->m_area = single_px_area;
											primary->m_new_position.X = new_position.X;
											primary->m_new_position.Y = new_position.Y + SMOOTH_CORNER_DISTANCE;
											if (Get_Collisions(primary, primary_collisions, 1) <= 0)
											{
												primary->m_area = original_area;
												primary->m_new_position.X = old_position.X;
												primary->m_new_position.Y = new_position.Y + lost_velocity;

												// Check new down position is valid.
												if (Get_Collisions(primary, primary_collisions, 1) <= 0)
												{
													result_position = primary->m_new_position;
													bWasSmoothed = true;
												}
											}
										}

										// Couldn't smooth corner, just set the result.
										if (!bWasSmoothed)
										{
											primary->m_area = original_area;
											primary->m_new_position.X = old_position.X;
											primary->m_new_position.Y = new_position.Y;
											result_position = primary->m_new_position;
										}
									}
								}
							}
							else
							{
								result_position = primary->m_new_position;

								// Horizontal axis is valid movement, vertical is not.
								// If smoothed, then attempt to move along horizontal axis with the velocity lost on the vertical axis, to the nearest free collision area.
								if (primary->m_smoothed && fabs(old_position.X - new_position.X) < fabs(old_position.Y - new_position.Y) * 0.25f)
								{
									float lost_velocity = fabs(new_position.Y - old_position.Y) * 0.75f;

									bool bWasSmoothed = false;

									Rect2D original_area = primary->m_area;
									Rect2D single_px_area = Rect2D(original_area.Center().X, original_area.Y, SMOOTH_PROBE_SIZE, original_area.Height);

									// Look left to see if we are on a corner.
									primary->m_area = single_px_area;
									primary->m_new_position.X = new_position.X - SMOOTH_CORNER_DISTANCE;
									primary->m_new_position.Y = new_position.Y;
									if (Get_Collisions(primary, primary_collisions, 1) <= 0)
									{
										primary->m_area = original_area;
										primary->m_new_position.X = new_position.X - lost_velocity;
										primary->m_new_position.Y = old_position.Y;

										// Check new left position is valid.
										if (Get_Collisions(primary, primary_collisions, 1) <= 0)
										{
											result_position = primary->m_new_position;
											bWasSmoothed = true;
										}
									}
									else
									{
										// Look right to see if we are on corner.
										primary->m_area = single_px_area;
										primary->m_new_position.X = new_position.X + SMOOTH_CORNER_DISTANCE;
										primary->m_new_position.Y = new_position.Y;
										if (Get_Collisions(primary, primary_collisions, 1) <= 0)
										{
											primary->m_area = original_area;
											primary->m_new_position.X = new_position.X + lost_velocity;
											primary->m_new_position.Y = old_position.Y;

											// Check new right position is valid.
											if (Get_Collisions(primary, primary_collisions, 1) <= 0)
											{
												result_position = primary->m_new_position;
												bWasSmoothed = true;
											}
										}
									}

									// Couldn't smooth corner, just set the result.
									if (!bWasSmoothed)
									{
										primary->m_area = original_area;
										primary->m_new_position.X = new_position.X;
										primary->m_new_position.Y = old_position.Y;
										result_position = primary->m_new_position;
									}
								}
							}
						}
						else
						{
							result_position = primary->m_old_position;
						}

						break;
					}
				case CollisionType::Non_Solid:
					{
						// Nothing to do here?
						break;
					}
				}
			}

			primary->m_new_position = result_position;
		}
	}

	double elapsed = Platform::Get()->Get_Ticks() - time;
//	DBG_LOG("COLLISION:%.2f ms, %i finds", elapsed, find_counts);
}

void CollisionSimulationTask::Run()
{
	CollisionManager* manager = CollisionManager::Get();
	manager->Simulate();
}