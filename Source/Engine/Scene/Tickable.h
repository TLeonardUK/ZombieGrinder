// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TICKABLE_
#define _ENGINE_TICKABLE_

#include "Engine/Engine/FrameTime.h"

#include "Generic/Types/Vector3.h"
#include "Generic/Types/Rect2D.h"

struct TickPriority
{
	enum Type
	{
		Early  = -1,
		Normal =  0,
		Late   =  1,
		Final  =  2
	};
};

class Scene;

class Tickable
{
	MEMORY_ALLOCATOR(Tickable, "Scene");

protected:
	bool				m_enabled;
	TickPriority::Type  m_tick_priority;
	bool				m_tick_offscreen;
	Rect2D				m_tick_area;

protected:
	friend class Scene;

	int					m_last_create_deferred_frame;
	int					m_last_collect_deferred_frame;

public:

	Tickable();

	// Base functions.
	virtual void Tick(const FrameTime& time) = 0;
	virtual void Collect_Deferred(const FrameTime& time);
	virtual void Create_Deferred(const FrameTime& time);

	// State changes.
	virtual void Set_Enabled(bool value);
	virtual bool Get_Enabled() const;

	virtual void Set_Tick_Priority(TickPriority::Type priority);
	virtual TickPriority::Type Get_Tick_Priority();

	virtual void Set_Tick_Offscreen(bool val);
	virtual bool Get_Tick_Offscreen();

	virtual void Set_Tick_Area(Rect2D val);
	virtual Rect2D Get_Tick_Area();

	virtual Rect2D Get_Tick_Offscreen_Bounding_Box();

	static bool Sort_By_Tick_Priority_Predicate(const Tickable* a, const Tickable* b)
	{
		return a->m_tick_priority < b->m_tick_priority;
	}

};

#endif

