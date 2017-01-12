// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Oval2D.h"
#include "Generic/Types/Rect2D.h"

#include "Generic/Math/Math.h"

Oval2D::Oval2D()
{
}

Oval2D::Oval2D(float x, float y, float w, float h)
	: X(x)
	, Y(y)
	, Width(w)
	, Height(h)
{
}

Oval2D::Oval2D(int x, int y, int w, int h)
	: X((float)x)
	, Y((float)y)
	, Width((float)w)
	, Height((float)h)
{
}

Oval2D::Oval2D(const Rect2D& bounds)
	: X(bounds.X)
	, Y(bounds.Y)
	, Width(bounds.Width)
	, Height(bounds.Height)
{
}

Vector2 Oval2D::Center() const
{
	return Vector2
	(
		X + (Width * 0.5f),
		Y + (Height * 0.5f)
	);
}

Vector2 Oval2D::Point_On_Edge(float angle) const
{
	Vector2 center = Center();
	float radius_x = Width * 0.5f;
	float radius_y = Height * 0.5f;
	float px = center.X + (cos(angle) * radius_x);
	float py = center.Y + (sin(angle) * radius_y);
	return Vector2(px, py);
}

// http://www.java-gaming.org/index.php/topic,26966
bool Oval2D::Intersects(const Oval2D& b, Vector3* intersection_pointer) const
{
	DBG_ASSERT(false);
	return false;

	/*
	Vector2 a_center = this->Center();
	Vector2 b_center = b.Center();
	
	// Calculate distance between centers.
	float dist_x = b_center.X - a_center.X;
	float dist_y = b_center.Y - a_center.Y;
	float center_distance = sqrt(dist_x * dist_x + dist_y * dist_y);

	// Calculate angle between centers and determine the point that lies on the edge
	// in said direction. Get the distance between center and this point.
	float angle = atan2(b_center.Y - a_center.Y, b_center.X - a_center.X);
	float a_dist_to_edge = (a_center - Point_On_Edge(angle)).Length();
	float b_dist_to_edge = (b_center - b.Point_On_Edge(-angle)).Length();

	// Add edge distances together.
	float edge_dist_sum = a_dist_to_edge + b_dist_to_edge;

	// If straight distance is less than edge distance sum the we are touching!
	return center_distance < edge_dist_sum;
	*/

	/*
	Vector2 a_center = this->Center();
	Vector2 b_center = b.Center();

	float a_radius_x = this->Width * 0.5f;
	float a_radius_y = this->Height * 0.5f;

	float b_radius_x = b.Width * 0.5f;
	float b_radius_y = b.Height * 0.5f;

	float a_smallest_radius = Min(a_radius_x, a_radius_y);
	float b_smallest_radius = Min(b_radius_x, b_radius_y);

	float a_largest_radius = Max(a_radius_x, a_radius_y);
	float b_largest_radius = Max(b_radius_x, b_radius_y);

	float smallest_radius_sum = a_smallest_radius + b_smallest_radius;
	float largest_radius_sum = a_largest_radius + b_largest_radius;

	float dist_x = b_center.X - a_center.X;
	float dist_y = b_center.Y - a_center.Y;

	//float W = largest_radius_sum / smallest_radius_sum;
	//float overral_distance = sqrt((dist_x / W) * (dist_x / W) + dist_y * dist_y);

	float center_distance = sqrt(dist_x * dist_x + dist_y * dist_y);

	return center_distance < (smallest_radius_sum * 2);
	*/
}

bool Oval2D::Intersects(const Line2D& p, Vector3* intersection_pointer) const
{
	DBG_ASSERT(false);
	return false;
}