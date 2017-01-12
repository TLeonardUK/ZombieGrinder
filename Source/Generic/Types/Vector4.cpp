// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/Vector4.h"
#include "Generic/Helper/StringHelper.h"

#include <math.h>
#include <cstdlib>

Vector4::Vector4()
	: X(0)
	, Y(0)
	, Z(0)
	, W(0)
{
}

Vector4::Vector4(float x, float y, float z, float w)
	: X(x)
	, Y(y)
	, Z(z)
	, W(w)
{
}

Vector4::Vector4(Vector3 v, float w)
	: X(v.X)
	, Y(v.Y)
	, Z(v.Z)
	, W(w)
{
}

bool Vector4::operator==(const Vector4 &other) const
{
	return (X == other.X && Y == other.Y && Z == other.Z && W == other.W);
}

bool Vector4::operator!=(const Vector4 &other) const
{
	return !operator==(other);
}

std::string Vector4::To_String()
{
	return StringHelper::To_String(X) + "," + StringHelper::To_String(Y) + "," + StringHelper::To_String(Z)+ "," + StringHelper::To_String(W);	
}

bool Vector4::Parse(std::string value, Vector4& result)
{	
	std::vector<std::string> segments;
	StringHelper::Split(value.c_str(), ',', segments);
	
	if (segments.size() != 4)
		return false;

	result = Vector4(
				(float)atof(segments.at(0).c_str()),
				(float)atof(segments.at(1).c_str()),
				(float)atof(segments.at(2).c_str()),
				(float)atof(segments.at(3).c_str())
			);

	return true;
}