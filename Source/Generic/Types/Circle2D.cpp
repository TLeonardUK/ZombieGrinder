// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Circle2D.h"
#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"

#include <cstdlib>
#include <math.h>

Circle2D::Circle2D()
	: X(0.0f)
	, Y(0.0f)
	, Radius(0.0f)
{
}

Circle2D::Circle2D(float x, float y, float radius)
	: X(x)
	, Y(y)
	, Radius(radius)
{
}

Circle2D::Circle2D(Rect2D r)
	: X(r.X + (r.Width * 0.5f))
	, Y(r.Y + (r.Height * 0.5f))
	, Radius(r.Width * 0.5f)
{
}

bool Circle2D::Intersects(const Circle2D& p, Vector3* intersection_pointer) const
{
	float dist_sqr = abs((X - p.X) * (X - p.X)) + ((Y - p.Y) * (Y - p.Y));
	float max_dist_sqr = (Radius + p.Radius) * (Radius + p.Radius);

	if (intersection_pointer != NULL)
	{
		// TODO: Don't care for now, just return circle center.
		*intersection_pointer = Vector3(X, Y, 0.0f);
	}

	return (dist_sqr < max_dist_sqr);
}

bool Circle2D::Intersects(const Rect2D& p, Vector3* intersection_pointer) const
{
	// Find closest point on rect to circle.
	float aabb_center_x		= p.X + (p.Width * 0.5f);
	float aabb_center_y		= p.Y + (p.Height * 0.5f);
	float closest_corner_x	= X;
	float closest_corner_y	= Y;

	if (X < p.X)				closest_corner_x = p.X;
	if (X > p.X + p.Width)		closest_corner_x = p.X + p.Width;
	if (Y < p.Y)				closest_corner_y = p.Y;
	if (Y > p.Y + p.Height)		closest_corner_y = p.Y + p.Height;

	// Figure out distance.
	float dx		 = (X - closest_corner_x);
	float dy		 = (Y - closest_corner_y);
	float dist_sqr	 = abs((dx * dx) + (dy * dy));
	float radius_sqr = (Radius * Radius);

	if (dist_sqr < radius_sqr)
	{
		if (intersection_pointer != NULL)
		{
			// TODO: Don't care for now, just return circle center.
			*intersection_pointer = Vector3(X, Y, 0.0f);
		}

		return true;
	}

	return false;
}

