// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Light.h"
#include "Engine/Scene/Tickable.h"

#include "Generic/Stats/Stats.h"

#include <algorithm>

DEFINE_FRAME_STATISTIC("Scene/Total Tickables", int, g_scene_total_tickable, true);
DEFINE_FRAME_STATISTIC("Scene/Total Ticked", int, g_scene_total_ticked, true);

Rect2D Scene::Get_Boundries()
{
	return m_boundries;
}

void Scene::Set_Boundries(Rect2D rect)
{
	m_boundries = rect;
}

void Scene::Unload()
{
	std::vector<Actor*> actors = m_actors;
	for (std::vector<Actor*>::iterator iter = actors.begin(); iter != actors.end(); iter++)
	{
		Actor* actor = *iter;
		SAFE_DELETE(actor);
	}
	m_actors.clear();

//	DBG_ASSERT(m_tickables.size() == 6);
//	DBG_ASSERT(m_drawables.size() == 5);
}

std::vector<Actor*>& Scene::Get_Actors()
{
	return m_actors;
}

void Scene::Add_Actor(Actor* camera)
{
	if (std::find(m_actors.begin(), m_actors.end(), camera) == m_actors.end())
	{
		m_actors.push_back(camera);
	}
}

void Scene::Remove_Actor(Actor* camera)
{
	std::vector<Actor*>::iterator iter = std::find(m_actors.begin(), m_actors.end(), camera);
	if (iter != m_actors.end())
	{
		m_actors.erase(iter);
	}
}

std::vector<Camera*>& Scene::Get_Cameras()
{
	return m_cameras;
}

void Scene::Add_Camera(Camera* camera)
{
	if (std::find(m_cameras.begin(), m_cameras.end(), camera) == m_cameras.end())
	{
		m_cameras.push_back(camera);
	}
}

void Scene::Remove_Camera(Camera* camera)
{
	std::vector<Camera*>::iterator iter = std::find(m_cameras.begin(), m_cameras.end(), camera);
	if (iter != m_cameras.end())
	{
		m_cameras.erase(iter);
	}
}

std::vector<Light*>& Scene::Get_Lights()
{
	return m_lights;
}

void Scene::Add_Light(Light* camera)
{
	if (std::find(m_lights.begin(), m_lights.end(), camera) == m_lights.end())
	{
		m_lights.push_back(camera);
	}
}

void Scene::Remove_Light(Light* camera)
{
	std::vector<Light*>::iterator iter = std::find(m_lights.begin(), m_lights.end(), camera);
	if (iter != m_lights.end())
	{
		m_lights.erase(iter);
	}
}

std::vector<IDrawable*>& Scene::Get_Drawables()
{
	return m_drawables;
}

void Scene::Add_Drawable(IDrawable* camera)
{
	if (std::find(m_drawables.begin(), m_drawables.end(), camera) == m_drawables.end())
	{
		m_drawables.push_back(camera);
	}
}

void Scene::Remove_Drawable(IDrawable* camera)
{
	std::vector<IDrawable*>::iterator iter = std::find(m_drawables.begin(), m_drawables.end(), camera);
	if (iter != m_drawables.end())
	{
		m_drawables.erase(iter);
	}
}

std::vector<Tickable*>& Scene::Get_Tickables()
{
	return m_tickables;
}

void Scene::Add_Tickable(Tickable* camera)
{
	if (std::find(m_tickables.begin(), m_tickables.end(), camera) == m_tickables.end())
	{
		m_tickables.push_back(camera);
	}

	m_last_tickable_modify_index++;
}

void Scene::Remove_Tickable(Tickable* camera)
{
	std::vector<Tickable*>::iterator iter = std::find(m_tickables.begin(), m_tickables.end(), camera);
	if (iter != m_tickables.end())
	{
		m_tickables.erase(iter);
	}

	m_last_tickable_modify_index++;
}

void Scene::Tick(const FrameTime& time, std::vector<Rect2D> screen_viewports)
{
	g_scene_total_tickable.Set(m_tickables.size());

	// Work out which tickables can be ticked this frame.
	std::vector<Tickable*> potential_ticks;
	for (std::vector<Tickable*>::iterator iter = m_tickables.begin(); iter != m_tickables.end(); iter++)
	{
		Tickable* tickable = *iter;

		if (tickable->Get_Enabled())
		{
			if (tickable->Get_Tick_Offscreen())
			{
				Rect2D tick_area = tickable->Get_Tick_Area();
				if (tick_area.Width >= 1.0f || tick_area.Height >= 1.0f)
				{
					for (std::vector<Rect2D>::iterator iter1 = screen_viewports.begin(); iter1 != screen_viewports.end(); iter1++)
					{
						Rect2D& viewport = *iter1;
						if (viewport.Intersects(tick_area))
						{
							potential_ticks.push_back(tickable);
							break;
						}
					}
				}					 
				else
				{
					potential_ticks.push_back(tickable);
				}
			}
			else
			{
				Rect2D bounding_box = tickable->Get_Tick_Offscreen_Bounding_Box();

				for (std::vector<Rect2D>::iterator iter2 = screen_viewports.begin(); iter2 != screen_viewports.end(); iter2++)
				{
					Rect2D& viewport = *iter2;
					if (viewport.Intersects(bounding_box))
					{
						potential_ticks.push_back(tickable);
						break;
					}
				}
			}
		}
	}

	// Sort by tick priority.
	std::sort(potential_ticks.begin(), potential_ticks.end(), &Tickable::Sort_By_Tick_Priority_Predicate);

	// Tick!
	for (std::vector<Tickable*>::iterator iter = potential_ticks.begin(); iter != potential_ticks.end(); iter++)
	{
		(*iter)->Tick(time);
	}

	g_scene_total_ticked.Set(potential_ticks.size());
}

void Scene::Collect_Deferred(const FrameTime& time)
{
	int start_modify_index = m_last_tickable_modify_index;

	// Tick all tickables.
	for (int i = 0; i < (int)m_tickables.size(); i++) // Done this way in case we remove tickables during collection.
	{
		Tickable* tickable = m_tickables[i];

		if (tickable->Get_Enabled() && tickable->m_last_collect_deferred_frame != time.Get_Frame())
		{
			tickable->m_last_collect_deferred_frame = time.Get_Frame();
			tickable->Collect_Deferred(time);

			if (m_last_tickable_modify_index == start_modify_index)
			{
				start_modify_index = m_last_tickable_modify_index;
			}
		}
	}
}

void Scene::Create_Deferred(const FrameTime& time)
{
	int start_modify_index = m_last_tickable_modify_index;

	// Tick all tickables.
	for (int i = 0; i < (int)m_tickables.size(); i++) // Done this way in case we remove tickables during collection.
	{
		Tickable* tickable = m_tickables[i];

		if (tickable->Get_Enabled() && tickable->m_last_create_deferred_frame != time.Get_Frame())
		{		
			tickable->m_last_create_deferred_frame = time.Get_Frame();
			tickable->Create_Deferred(time);

			if (m_last_tickable_modify_index == start_modify_index)
			{
				start_modify_index = m_last_tickable_modify_index;
			}
		}
	}
}

void Scene::Build_Demo_ID_Lookup()
{
	m_demo_id_lookup.Clear();

	for (std::vector<Actor*>::iterator iter = m_actors.begin(); iter != m_actors.end(); iter++)
	{
		Actor* actor = *iter;
		if (actor->Is_Relevant_To_Demo())
		{
			m_demo_id_lookup.Set(actor->Get_Demo_ID(), actor);
		}
	}
}

Actor* Scene::Get_Actor_By_Demo_ID(int demo_id)
{
	Actor* result = NULL;
	if (m_demo_id_lookup.Get(demo_id, result))
	{
		return result;
	}
	return NULL;
}
