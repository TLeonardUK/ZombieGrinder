// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Rect2D.h"
#include "Generic/Helper/StringHelper.h"

#include "Generic/Math/Math.h"

#include <cstdlib>

Rect2D::Rect2D()
	: X(0)
	, Y(0)
	, Width(0)
	, Height(0)
{
}

Rect2D::Rect2D(float x, float y, float w, float h)
{
	X = x;
	Y = y;
	Width = w;
	Height = h;
}

Rect2D::Rect2D(int x, int y, int w, int h)
{
	X = (float)x;
	Y = (float)y;
	Width = (float)w;
	Height = (float)h;
}

Rect2D::Rect2D(Vector2 x, Vector2 y)
{
	X = x.X;
	Y = x.Y;
	Width = y.X;
	Height = y.Y;
}

Rect2D::Rect2D(Vector4 x)
{
	X = x.X;
	Y = x.Y;
	Width = x.Z;
	Height = x.W;
}

bool Rect2D::operator==(const Rect2D& rhs) const
{
	return (X == rhs.X && Y == rhs.Y && Width == rhs.Width && Height == rhs.Height);
}

bool Rect2D::operator!=(const Rect2D& rhs) const
{
	return !operator==(rhs);
}

bool Rect2D::Intersects(const Vector2& p, Vector3* intersection_pointer) const
{
	if (p.X >= X && p.Y >= Y && p.X < X + Width && p.Y < Y + Height)
	{
		if (intersection_pointer != NULL)
		{
			*intersection_pointer = Vector3(p.X, p.Y, 0.0f);
		}

		return true;
	}

	return false;
}

bool Rect2D::Intersects(const Rect2D& p, Vector3* intersection_pointer) const
{
	if (p.X + p.Width < X	||	
		p.Y + p.Height < Y	||
		p.X > X + Width		||
		p.Y > Y + Height)
	{
		return false;
	}

	// Fuck it return center for now.
	if (intersection_pointer != NULL)
	{
		*intersection_pointer = Vector3(p.X + (p.Width * 0.5f), p.Y + (p.Height * 0.5f), 0.0f);
	}

	return true;
}

bool Rect2D::Intersects(const Oval2D& p, Vector3* intersection_pointer) const
{
	DBG_ASSERT(false);
	return false;
}

bool Rect2D::Intersects(const Line2D& p, Vector3* intersection_pointer) const
{
	DBG_ASSERT(false);
	return false;
}

float Rect2D::DistanceToSquared(Vector2 p)
{
	float cx = X + (Width * 0.5f);
	float cy = Y + (Height * 0.5f);
	float dx = Max(fabs(p.X - cx) - Width / 2.0f, 0);
	float dy = Max(fabs(p.Y - cy) - Height / 2.0f, 0);
	return dx * dx + dy * dy;
}

Rect2D Rect2D::Union(Vector2 p)
{
	Rect2D result = *this;
	float top		= Min(p.Y, result.Y);
	float bottom	= Max(p.Y, result.Y + result.Height);
	float left	= Min(p.X, result.X);
	float right	= Max(p.X, result.X + result.Width);	
	return Rect2D(left, top, right - left, bottom - top);
}

Rect2D Rect2D::Union(Rect2D p)
{
	Rect2D result = *this;
	float left	= Min(p.X, result.X);
	float top		= Min(p.Y, result.Y);
	float right	= Max(p.X + p.Width, result.X + result.Width);	
	float bottom	= Max(p.Y + p.Height, result.Y + result.Height);
	return Rect2D(left, top, right - left, bottom - top);
}

Rect2D Rect2D::Inflate(float x, float y)
{
	return Rect2D
	(
		X - (x * 0.5f),
		Y - (y * 0.5f),
		Width + x,
		Height + y
	);
}

Rect2D Rect2D::operator*(const Vector2 a) const
{
	return Rect2D
	(
		X * a.X,
		Y * a.Y,
		Width * a.X,
		Height * a.Y
	);
}

Rect2D Rect2D::operator+(const Rect2D a) const
{
	return Rect2D
	(
		X + a.X,
		Y + a.Y,
		Width + a.Width,
		Height + a.Height
	);
}

Rect2D Rect2D::operator+(const Vector3 a) const
{
	return Rect2D
	(
		X + a.X,
		Y + a.Y,
		Width,
		Height
	);
}
std::string Rect2D::To_String()
{
	return StringHelper::To_String(X) + "," + StringHelper::To_String(Y) + "," + StringHelper::To_String(Width)+ "," + StringHelper::To_String(Height);	
}

bool Rect2D::Parse(std::string value, Rect2D& result)
{
	std::vector<std::string> segments;
	StringHelper::Split(value.c_str(), ',', segments);
	
	if (segments.size() != 4)
		return false;

	result = Rect2D(
				(float)atof(segments.at(0).c_str()),
				(float)atof(segments.at(1).c_str()),
				(float)atof(segments.at(2).c_str()),
				(float)atof(segments.at(3).c_str())
			);

	return true;
}

Rect2D Rect2D::ClampInside(Rect2D bounds)
{
	Rect2D output = *this;
	output.X = Max(output.X, bounds.X);
	output.Y = Max(output.Y, bounds.Y);
	if (output.X + output.Width > bounds.X + bounds.Width)
	{
		output.Width -= (output.X + output.Width) - (bounds.X + bounds.Width);
	}
	if (output.Y + output.Height > bounds.Y + bounds.Height)
	{
		output.Height -= (output.Y + output.Height) - (bounds.Y + bounds.Height);
	}
	return output;
}

Rect2D Rect2D::ConstrainInside(Rect2D bounds)
{
	Rect2D output = *this;
	
	if (output.X < bounds.X)
	{
		float diff = (bounds.X - output.X);
		output.X += diff;
	}
	if (output.Y < bounds.Y)
	{
		float diff = (bounds.Y - output.Y);
		output.Y += diff;
	}
	if (output.X + output.Width > bounds.X + bounds.Width)
	{
		float diff = (output.X + output.Width) - (bounds.X + bounds.Width);
		output.X -= diff;
	}
	if (output.Y + output.Height > bounds.Y + bounds.Height)
	{
		float diff = (output.Y + output.Height) - (bounds.Y + bounds.Height);
		output.Y -= diff;
	}

	return output;
}

Vector2 Rect2D::Center()
{
	return Vector2(X + (Width * 0.5f), Y + (Height * 0.5f));
}