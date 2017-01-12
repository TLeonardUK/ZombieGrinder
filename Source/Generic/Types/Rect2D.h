// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef __GENERIC_RECTANGLE__
#define __GENERIC_RECTANGLE__

#include <string>

#include "Generic/Types/Vector2.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector4.h"
#include "Generic/Types/Oval2D.h"
#include "Generic/Types/Line2D.h"

class Rect2D
{
//	MEMORY_ALLOCATOR(Rectangle, "Data Types");

public:
	float X, Y, Width, Height;

	Rect2D();
	Rect2D(float x, float y, float w, float h);
	Rect2D(int x, int y, int w, int h);
	Rect2D(Vector2 x, Vector2 y);
	Rect2D(Vector4 x);

	// Operators.
	bool operator==(const Rect2D& rhs) const;
	bool operator!=(const Rect2D& rhs) const;
	Rect2D operator* (const Vector2 a) const;
	Rect2D operator+ (const Rect2D a) const;
	Rect2D operator+ (const Vector3 a) const;

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Rect2D& result);

	Rect2D Inflate(float x, float y);
	bool Intersects(const Vector2& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Rect2D& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Oval2D& p, Vector3* intersection_point = NULL) const;
	bool Intersects(const Line2D& p, Vector3* intersection_point = NULL) const;

	float DistanceToSquared(Vector2 p);

	Rect2D Union(Vector2 p);
	Rect2D Union(Rect2D p);

	Rect2D ClampInside(Rect2D bounds);
	Rect2D ConstrainInside(Rect2D bounds);

	Vector2 Center();

};

#endif