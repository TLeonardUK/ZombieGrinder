// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_DEMO_PROXY_ACTOR_
#define _ENGINE_SCENE_ACTORS_DEMO_PROXY_ACTOR_

#include "Game/Scene/Actors/CompositeActor.h"

class DemoProxyActor : public CompositeActor
{
	MEMORY_ALLOCATOR(DemoProxyActor, "Scene");

protected:

	virtual ~DemoProxyActor();

	virtual bool Is_Relevant_To_Demo();
	virtual bool Is_Demo_Proxy();

};

#endif

