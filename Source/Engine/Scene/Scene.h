// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_
#define _ENGINE_SCENE_

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/Rect2D.h"

#include "Generic/Types/HashTable.h"

#include <vector>

class Camera;
class IDrawable;
class Tickable;
class Light;
class Actor;
struct CVMLinkedSymbol;

// There are 2 passes for deferred collection. We use it to prioritise different 
#define CREATE_DEFERRED_PASSES 2

class Scene
{
	MEMORY_ALLOCATOR(Scene, "Scene");

protected:
	std::vector<Camera*>	m_cameras;
	std::vector<IDrawable*>	m_drawables;
	std::vector<IDrawable*>	m_predraw_registered_drawables;
	std::vector<Tickable*>	m_tickables;
	std::vector<Light*>		m_lights;
	std::vector<Actor*>		m_actors;

	Rect2D					m_boundries;

	HashTable<Actor*, int>	m_demo_id_lookup;

	int	m_last_tickable_modify_index;

public:

	// Base functions.
	void Tick(const FrameTime& time, std::vector<Rect2D> screen_viewports);
	void Collect_Deferred(const FrameTime& time);
	void Create_Deferred(const FrameTime& time);

	// Unloads all actors on current map.
	virtual void Unload();

	// Specialized type lists.
	// ---------------------------------------------------------

	// Camera accessors.
	std::vector<Camera*>& Get_Cameras();
	void Add_Camera(Camera* camera);
	void Remove_Camera(Camera* camera);	
	
	// Light accessors.
	std::vector<Light*>& Get_Lights();
	void Add_Light(Light* light);
	void Remove_Light(Light* light);	

	// Generic scene type lists.
	// ---------------------------------------------------------

	// Drawable accessors.
	std::vector<IDrawable*>& Get_Drawables();
	void Add_Drawable(IDrawable* camera);
	void Remove_Drawable(IDrawable* camera);	

	// Tickable accessors.
	std::vector<Tickable*>& Get_Tickables();
	void Add_Tickable(Tickable* camera);
	void Remove_Tickable(Tickable* camera);	

	// Camera accessors.
	std::vector<Actor*>& Get_Actors();
	virtual void Add_Actor(Actor* camera);
	virtual void Remove_Actor(Actor* camera);	
	virtual std::vector<Actor*> Get_Actors_Of_Class(CVMLinkedSymbol* sym) = 0; // This is pretty hacky, anything that needs script access should be in game project.
	virtual Rect2D Get_Full_Camera_Bounding_Box() = 0;

	// Scene properties.
	Rect2D Get_Boundries();
	void Set_Boundries(Rect2D rect);

	// Demo stuff.
	void Build_Demo_ID_Lookup();
	Actor* Get_Actor_By_Demo_ID(int demo_id);
	virtual Actor* Create_Demo_Proxy_Actor(int demo_id) = 0;
	virtual int Get_Particle_Instigator_Team_Index(void* instigator) = 0;

};

#endif

