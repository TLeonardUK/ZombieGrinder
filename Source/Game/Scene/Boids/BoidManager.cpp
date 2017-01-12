// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Boids/BoidManager.h"
#include "Game/Scene/Actors/Components/Tickable/BoidComponent.h"
#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Network/GameNetUser.h"

#include "Engine/Network/NetManager.h"

#include <algorithm>

void BoidManager::Register_Boid(BoidComponent* component)
{
	m_boids.push_back(component);
}

void BoidManager::Unregister_Boid(BoidComponent* component)
{
	std::vector<BoidComponent*>::iterator iter = std::find(m_boids.begin(), m_boids.end(), component);
	if (iter != m_boids.end())
	{
		m_boids.erase(iter);
	}
}

void BoidManager::Tick(const FrameTime& time)
{
	// Boids are only simulated on the server or we end up with miss-matched movement.
	if (!NetManager::Get()->Is_Server())
	{
		return;
	}

	std::vector<Boid*> neighbours;
	neighbours.reserve(m_boids.size());

	// Precompute some reused data.
	std::vector<Boid> boids;
	for (unsigned int i = 0; i < m_boids.size(); i++)
	{
		BoidComponent* boid = m_boids[i];
		ScriptedActor* parent = dynamic_cast<ScriptedActor*>(boid->Get_Parent());

		// Calculate current velocity.
		Vector2 center_offset	= boid->Get_Center();
		Vector3 last_position	= parent->Get_Position() + Vector3(center_offset.X, center_offset.Y, 0.0f);
		Vector3 target_position = last_position;

		if (parent->Is_Movement_Pending())
		{
			target_position = parent->Get_Target_Position() + Vector3(center_offset.X, center_offset.Y, 0.0f);
		}

		Vector3 velocity		= target_position - last_position;

		Boid n;
		n.boid					= boid;
		n.parent				= parent;
		n.last_position			= last_position;
		n.target_position		= target_position;
		n.velocity				= velocity;
		n.neighbour_distance	= boid->Get_Neighbour_Distance();
		n.center_offset			= Vector3(center_offset.X, center_offset.Y, 0.0f);

		boids.push_back(n);
	}

	for (unsigned int i = 0; i < boids.size(); i++)
	{
		Boid* boid = &boids[i];

		// Find our neighbours.
		neighbours.clear();
		for (unsigned int j = 0; j < m_boids.size(); j++)
		{
			if (j == i)
			{
				continue;
			}

			Boid* other	= &boids[j];
			float distance = (other->target_position - boid->target_position).Length();

			if (distance < boid->neighbour_distance)
			{
				neighbours.push_back(other);
			}
		}

		// Calculate boid velocity.
		Vector3 cohesion		= Calculate_Cohesion(boid, neighbours) * boid->boid->Get_Cohesion_Weight();
		Vector3 seperation		= Calculate_Seperation(boid, neighbours) * boid->boid->Get_Seperation_Weight();
		Vector3 alignment		= Calculate_Alignment(boid, neighbours) * boid->boid->Get_Avoidance_Weight();
		Vector3 acceleration	= (seperation + alignment + cohesion).Limit(10.0f); 

		// TODO: Dumb, not cummulative.
		Vector3 result_velocity = boid->velocity + (acceleration * time.Get_Delta());
		Vector3 new_position	= boid->last_position + result_velocity;
		Vector3 result			= new_position - boid->center_offset;

		// Only apply if we own this object.
		GameNetUser* owner = boid->parent->Get_Owner();
		bool is_server = NetManager::Get()->Is_Server();
		if ((owner == NULL && is_server) || (owner != NULL && owner->Get_Online_User()->Is_Local()))
		{
			boid->parent->MoveTo(result);
		}
	}
}

Vector3 BoidManager::Calculate_Cohesion(Boid* boid, std::vector<Boid*>& neighbours)
{
	// TODO: Patttth, we don't care about any of this atm.
	return Vector3(0.0f, 0.0f, 0.0f);
}

Vector3 BoidManager::Calculate_Seperation(Boid* boid, std::vector<Boid*>& neighbours)
{	
	Vector3 mean = Vector3(0.0f, 0.0f, 0.0f);
	int count = 0;

	float sep_distance = boid->boid->Get_Seperation_Distance();

	for (unsigned int i = 0; i < neighbours.size(); i++)
	{
		Boid* other	= neighbours[i];
		Vector3 vec = (boid->target_position - other->target_position);

		float distance = vec.Length();
		if (distance <= 0.0f)
		{
			distance = 1.0f;
		}

		if (distance > 0 && distance <= sep_distance)
		{
			mean = mean + (vec.Normalize() / distance);
			count++;
		}
	}

	if (count > 0)
	{
		mean = mean / (float)count;
	}

	return mean.Limit(boid->boid->Get_Maximum_Force());
}

Vector3 BoidManager::Calculate_Alignment(Boid* boid, std::vector<Boid*>& neighbours)
{
	// TODO: Patttth, we don't care about any of this atm.
	return Vector3(0.0f, 0.0f, 0.0f);

	/*
	Vector3 mean = Vector3(0.0f, 0.0f, 0.0f);
	int count = 0;

	for (unsigned int i = 0; i < neighbours.size(); i++)
	{
		Boid* other	= neighbours[i];
		float distance = (other->target_position - boid->target_position).Length();

		mean = mean + distance;
		count++;
	}

	if (count > 0)
	{
		mean = mean / (float)count;
	}

	return mean.Limit(boid->boid->Get_Maximum_Force());
	*/
}