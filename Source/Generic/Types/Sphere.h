// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_SPHERE_
#define _GENERIC_SPHERE_

#include "Generic/Types/Vector3.h"

class Sphere
{
//	MEMORY_ALLOCATOR(Sphere, "Data Types");

public:
	Vector3 Center;
	float   Radius;

	Sphere(Vector3 center, float radius)
		: Center(center)
		, Radius(radius)
	{
	}

};

#endif