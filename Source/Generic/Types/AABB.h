// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_AABB_
#define _GENERIC_AABB_

#include "Generic/Types/Vector3.h"

class AABB
{
	//MEMORY_ALLOCATOR(AABB, "Data Types");

public:
	float X, Y, Z;
	float Width, Height, Depth;

	Vector3 Vertices[8];
	Vector3 Center;

	AABB()
	{
	}

	AABB(float x, float y, float z, float w, float h, float d)
	{
		X = x;
		Y = y;
		Z = z;
		Width = w;
		Height = h;
		Depth = d;

		Vertices[0] = Vector3(X, Y, Z);
		Vertices[1] = Vector3(X + Width, Y, Z);
		Vertices[2] = Vector3(X + Width, Y, Z + Depth);
		Vertices[3] = Vector3(X, Y, Z + Depth);
		Vertices[4] = Vector3(X, Y + Height, Z);
		Vertices[5] = Vector3(X + Width, Y + Height, Z);
		Vertices[6] = Vector3(X + Width, Y + Height, Z + Depth);
		Vertices[7] = Vector3(X, Y + Height, Z + Depth);
		
		Center = Vector3(X + (Width  * 0.5f),
					     Y + (Height * 0.5f),
					     Z + (Depth  * 0.5f));
	}

};

#endif