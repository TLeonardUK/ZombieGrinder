// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef __GENERIC_OVAL__
#define __GENERIC_OVAL__

#include <string>

//#include "Generic/Types/Rect2D.h"
//#include "Generic/Types/Line2D.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Types/Vector3.h"

class Rect2D;
class Line2D;

class Oval2D
{
//	MEMORY_ALLOCATOR(Rectangle, "Data Types");

public:
	float X, Y, Width, Height;

	Oval2D();
	Oval2D(float x, float y, float w, float h);
	Oval2D(int x, int y, int w, int h);
	Oval2D(const Rect2D& bounds);

	Vector2 Center() const;
	Vector2 Point_On_Edge(float angle) const;

	bool Intersects(const Oval2D& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Line2D& p, Vector3* intersection_point = NULL) const;


};

#endif