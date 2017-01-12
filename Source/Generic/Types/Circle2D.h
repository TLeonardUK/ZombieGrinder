// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef __GENERIC_CIRCLE__
#define __GENERIC_CIRCLE__

#include <string>

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector3.h"

class Circle2D
{
	//	MEMORY_ALLOCATOR(Rectangle, "Data Types");

public:
	float X, Y, Radius;

	Circle2D();
	Circle2D(float x, float y, float radius);
	Circle2D(Rect2D r);

	bool Intersects(const Circle2D& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Rect2D& p, Vector3* intersection_point = NULL) const;

};

#endif