// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_TICKABLECOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_TICKABLE_TICKABLECOMPONENT_

#include "Engine/Scene/Tickable.h"

#include "Game/Scene/Actors/CompositeActor.h"

class CompositeActor;

class TickableComponent : public Component, public Tickable
{
	MEMORY_ALLOCATOR(TickableComponent, "Scene");

private:

public:
	TickableComponent();
	virtual ~TickableComponent();

};

#endif

