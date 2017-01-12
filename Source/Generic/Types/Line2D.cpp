// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Line2D.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Circle2D.h"

#include "Generic/Math/Math.h"

#include <vector>

Line2D::Line2D()
{
}

Line2D::Line2D(Vector2 start, Vector2 end)
	: Start(start)
	, End(end)
{
}

Line2D::Line2D(float s_x, float s_y, float e_x, float e_y)
	: Start(s_x, s_y)
	, End(e_x, e_y)
{
}

Line2D::Line2D(const Rect2D& bounds)
	: Start(bounds.X, bounds.Y)
	, End(bounds.Width, bounds.Height)
{
}

bool Line2D::Intersects(const Line2D& p, Vector3* intersection_pointer) const
{
	float p0_x = Start.X;
	float p0_y = Start.Y;
	float p1_x = End.X;
	float p1_y = End.Y;
	float p2_x = p.Start.X;
	float p2_y = p.Start.Y;
	float p3_x = p.End.X;
	float p3_y = p.End.Y;
	/*
	float p0_x = Min(Start.X, End.X);
	float p0_y = Min(Start.Y, End.Y);
	float p1_x = Max(Start.X, End.X);
	float p1_y = Max(Start.Y, End.Y);
	float p2_x = Min(p.Start.X, p.End.X);
	float p2_y = Min(p.Start.Y, p.End.Y);
	float p3_x = Max(p.Start.X, p.End.X);
	float p3_y = Max(p.Start.Y, p.End.Y);

	// Diagonal bottom-left to top-right seems fucked, cba working out the maths for
	// this atm, this fixes it.
	if (p0_x < p1_x && p0_y > p1_y)
	{
		p0_x = Min(Start.X, End.X);
		p0_y = Max(Start.Y, End.Y);
		p1_x = Max(Start.X, End.X);
		p1_y = Min(Start.Y, End.Y);
		p2_x = Min(p.Start.X, p.End.X);
		p2_y = Max(p.Start.Y, p.End.Y);
		p3_x = Max(p.Start.X, p.End.X);
		p3_y = Min(p.Start.Y, p.End.Y);

		DBG_LOG("From:%.1f,%.1f To:%.1f,%.1f -- From:%.1f,%.1f To:%.1f,%.1f", p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y);
	}
	*/

	float s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;  s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;  s2_y = p3_y - p2_y;

	float s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
	{
		if (intersection_pointer != NULL)
		{
			*intersection_pointer = Vector3(
				p0_x + (t * s1_x),
				p0_y + (t * s1_y),
				0.0f
			);
		}

		return true;
	}

	return false;
}

bool Line2D::Intersects(const Rect2D& p, Vector3* intersection_pointer) const
{
	Vector2 start = this->Start;
	Vector2 end = this->End;

	// Line inside rect.
	if (p.Intersects(start) && p.Intersects(end))
	{
		if (intersection_pointer != NULL)
		{
			// Eeeeerrrrm, start of the line for intersection I guess?
			*intersection_pointer = Vector3(start.X, start.Y, 0.0f);
		}
		return true;
	}

	// Check each line segment.
	std::vector<Vector3> points;

	Vector3 point;

	// Top edge
	if (Intersects(Line2D(p.X, p.Y, p.X + p.Width, p.Y), &point))						
	{
		points.push_back(point);
	}

	// Bottom edge
	if (Intersects(Line2D(p.X, p.Y + p.Height, p.X + p.Width, p.Y + p.Height), &point))
	{
		points.push_back(point);
	}
	
	// Left edge
	if (Intersects(Line2D(p.X, p.Y, p.X, p.Y + p.Height), &point))						
	{
		points.push_back(point);
	}
	
	// Right edge
	if (Intersects(Line2D(p.X + p.Width, p.Y, p.X + p.Width, p.Y + p.Height), &point))
	{
		points.push_back(point);
	}

	// Return closes to start position.
	float best_dist = 0.0f;
	for (int i = 0; i < (int)points.size(); i++)
	{
		Vector3& pos = points[i];
		float dist = (Vector3(start.X, start.Y, 0.0f) - pos).Length();
		if (i == 0 || dist < best_dist)
		{
			*intersection_pointer = pos;
			best_dist = dist;
		}
	}

	return (points.size() > 0);
}

bool Line2D::Intersects(const Circle2D& p, Vector3* intersection_pointer) const
{     
	float x1 = Start.X;
	float y1 = Start.Y;
	float x2 = End.X;
	float y2 = End.Y;

	float cx = p.X;
	float cy = p.Y;
	float cr = p.Radius;

	float dx = x2 - x1;
	float dy = y2 - y1;
	float a = dx * dx + dy * dy;
	float b = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
	float c = cx * cx + cy * cy;

	c += x1 * x1 + y1 * y1;
	c -= 2 * (cx * x1 + cy * y1);
	c -= cr * cr;

	float bb4ac = b * b - 4 * a * c;

	if (bb4ac >= 0)
	{
		if (intersection_pointer != NULL)
		{
			float t = -b / (2.0f * a);
			*intersection_pointer = Vector3(cx + t * dx, cy + t * dy, 0.0f);

		}
		return true;
	}

	return false;
}