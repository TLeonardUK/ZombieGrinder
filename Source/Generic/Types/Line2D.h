// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef __GENERIC_LINE__
#define __GENERIC_LINE__

#include <string>

#include "Generic/Types/Vector2.h"
#include "Generic/Types/Vector3.h"
//#include "Generic/Types/Rect2D.h"

class Rect2D;
class Circle2D;

class Line2D
{
//	MEMORY_ALLOCATOR(Rectangle, "Data Types");

public:
	Vector2 Start;
	Vector2 End;

	Line2D();
	Line2D(Vector2 start, Vector2 end);
	Line2D(const Rect2D& bounds);
	Line2D(float s_x, float s_y, float e_x, float e_y);

	bool Intersects(const Line2D& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Rect2D& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Circle2D& p, Vector3* intersection_point = NULL) const;


};

#endif