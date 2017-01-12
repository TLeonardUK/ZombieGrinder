// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Vector2.h"
#include "Generic/Helper/StringHelper.h"
#include "Generic/Math/Math.h"

#include <cstdlib>

Vector2::Vector2()
	: X(0)
	, Y(0)
{
}

Vector2::Vector2(int x, int y)
{
	X = (float)x;
	Y = (float)y;
}

Vector2::Vector2(float x, float y)
{
	X = x;
	Y = y;
}

Vector2 Vector2::Round() const
{
	return Vector2
	(
		ceilf(X),
		ceilf(Y)
	);
}	

Vector2 Vector2::MaxValue(const Vector2 a) const
{
	return Vector2
	(
		Max(X, a.X),
		Max(Y, a.Y)
	);
}

Vector2 Vector2::MinValue(const Vector2 a) const
{
	return Vector2
	(
		Min(X, a.X),
		Min(Y, a.Y)
	);
}

float Vector2::Length() const
{
	return sqrt(X * X + Y * Y);
}

float Vector2::Length_Squared() const
{
	return X * X + Y * Y;
}

bool Vector2::operator==(Vector2& rhs) const
{
	return (X == rhs.X && Y == rhs.Y);
}

bool Vector2::operator!=(Vector2& rhs) const
{
	return !operator==(rhs);
}

Vector2 Vector2::operator+(const Vector2 a) const
{
	return Vector2(X + a.X, Y + a.Y);
}

Vector2 Vector2::operator-(const Vector2 a) const
{
	return Vector2(X - a.X, Y - a.Y);
}

Vector2 Vector2::operator*(const Vector2 a) const
{
	return Vector2(X * a.X, Y * a.Y);
}

Vector2 Vector2::operator*(const float a) const
{
	return Vector2(X * a, Y * a);
}

Vector2 Vector2::operator/(const Vector2 a) const
{
	return Vector2(X / a.X, Y / a.Y);
}

std::string Vector2::To_String()
{
	return StringHelper::To_String(X) + "," + StringHelper::To_String(Y);
}

bool Vector2::Parse(std::string value, Vector2& result)
{
	std::vector<std::string> segments;
	StringHelper::Split(value.c_str(), ',', segments);

	if (segments.size() != 2)
		return false;

	result = Vector2(
				(float)atof(segments.at(0).c_str()),
				(float)atof(segments.at(1).c_str())
			 );

	return true;
}
