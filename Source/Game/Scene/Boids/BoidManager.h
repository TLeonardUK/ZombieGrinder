// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_BOIDMANAGER_
#define _GAME_SCENE_BOIDMANAGER_

#include "Engine/Scene/Scene.h"
#include "Engine/Engine/FrameTime.h"

#include "Generic/Patterns/Singleton.h"

class BoidComponent;
class ScriptedActor;

class BoidManager
	: public Singleton<BoidManager>
{
	MEMORY_ALLOCATOR(BoidManager, "BoidManager");

private:
	struct Boid
	{
		BoidComponent*	boid;
		ScriptedActor*	parent;
		Vector3			last_position;
		Vector3			target_position;
		Vector3			velocity;
		Vector3			center_offset;
		float			neighbour_distance;
	};

	std::vector<BoidComponent*> m_boids;

	Vector3 Calculate_Cohesion(Boid* component, std::vector<Boid*>& neighbours);
	Vector3 Calculate_Seperation(Boid* component, std::vector<Boid*>& neighbours);
	Vector3 Calculate_Alignment(Boid* component, std::vector<Boid*>& neighbours);

public:
	void Register_Boid(BoidComponent* component);
	void Unregister_Boid(BoidComponent* component);

	void Tick(const FrameTime& time);

};

#endif

