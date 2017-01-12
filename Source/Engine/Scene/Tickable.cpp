// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Tickable.h"

Tickable::Tickable()
	: m_enabled(true)
	, m_tick_priority(TickPriority::Normal)
	, m_tick_offscreen(true)
	, m_tick_area(0.0f, 0.0f, 0.0f, 0.0f)
	, m_last_collect_deferred_frame(0)
	, m_last_create_deferred_frame(0)
{
}

void Tickable::Set_Enabled(bool value)
{
	m_enabled = value;
}

bool Tickable::Get_Enabled() const
{
	return m_enabled;
}

void Tickable::Set_Tick_Priority(TickPriority::Type priority)
{
	m_tick_priority = priority;
}

TickPriority::Type Tickable::Get_Tick_Priority()
{
	return m_tick_priority;
}

void Tickable::Set_Tick_Offscreen(bool priority)
{
	m_tick_offscreen = priority;
}

bool Tickable::Get_Tick_Offscreen()
{
	return m_tick_offscreen;
}

void Tickable::Set_Tick_Area(Rect2D val)
{
	m_tick_area = val;
}

Rect2D Tickable::Get_Tick_Area()
{
	return m_tick_area;
}

Rect2D Tickable::Get_Tick_Offscreen_Bounding_Box()
{
	// Should be overriden!
	return Rect2D(0.0f, 0.0f, 0.0f, 0.0f);
}

void Tickable::Collect_Deferred(const FrameTime& time)
{
	// Collect deferred results if applicable.
}

void Tickable::Create_Deferred(const FrameTime& time)
{
	// Create deferred processes if applicable.
}
