// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_COLLISIONHELPER_
#define _GENERIC_HELPER_COLLISIONHELPER_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Oval2D.h"
#include "Generic/Types/Line2D.h"
#include "Generic/Types/Circle2D.h"

template <typename TypeA, typename TypeB>
class CollisionHelper
{
public:

	// WARNING:
	// Intersection point can only be safely used on line->something collision, others may return
	// different results depending on the intersection type (rect fully contained in rect etc).
	INLINE static bool Intersects(TypeA a, TypeB b, Vector3* intersection_point);
};

// ===================================================================
// Rect -> Rect
// ===================================================================
template <>
class CollisionHelper<Rect2D, Rect2D>
{
public:
	INLINE static bool Intersects(const Rect2D& a, const Rect2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Rect -> Oval
// ===================================================================
template <>
class CollisionHelper<Rect2D, Oval2D>
{
public:
	INLINE static bool Intersects(const Rect2D& a, const Oval2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Rect -> Circle
// ===================================================================
template <>
class CollisionHelper<Circle2D, Rect2D>
{
public:
	INLINE static bool Intersects(const Circle2D& a, const Rect2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Rect -> Line
// ===================================================================
template <>
class CollisionHelper<Rect2D, Line2D>
{
public:
	INLINE static bool Intersects(const Rect2D& a, const Line2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Circle -> Circle
// ===================================================================
template <>
class CollisionHelper<Circle2D, Circle2D>
{
public:
	INLINE static bool Intersects(const Circle2D& a, const Circle2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Oval -> Oval
// ===================================================================
template <>
class CollisionHelper<Oval2D, Oval2D>
{
public:
	INLINE static bool Intersects(const Oval2D& a, const Oval2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Line -> Rect
// ===================================================================
template <>
class CollisionHelper<Line2D, Rect2D>
{
public:
	INLINE static bool Intersects(const Line2D& a, const Rect2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Line -> Circle
// ===================================================================
template <>
class CollisionHelper<Line2D, Circle2D>
{
public:
	INLINE static bool Intersects(const Line2D& a, const Circle2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

// ===================================================================
// Line -> Line
// ===================================================================
template <>
class CollisionHelper<Line2D, Line2D>
{
public:
	INLINE static bool Intersects(const Line2D& a, const Line2D& b, Vector3* intersection_point)
	{
		return a.Intersects(b, intersection_point);
	}		
};

#endif
