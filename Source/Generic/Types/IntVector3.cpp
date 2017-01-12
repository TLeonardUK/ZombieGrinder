// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/IntVector3.h"
#include "Generic/Helper/StringHelper.h"

#include <math.h>
#include <cstdlib>

IntVector3::IntVector3()
	: X(0)
	, Y(0)
	, Z(0)
{
}

IntVector3::IntVector3(int x, int y, int z)
	: X(x)
	, Y(y)
	, Z(z)
{
}

std::string IntVector3::To_String()
{
	return StringHelper::To_String(X) + "," + StringHelper::To_String(Y) + "," + StringHelper::To_String(Z);	
}

bool IntVector3::Parse(std::string value, IntVector3& result)
{
	std::vector<std::string> segments;
	StringHelper::Split(value.c_str(), ',', segments);
	
	if (segments.size() != 3)
		return false;

	result = IntVector3(
				atoi(segments.at(0).c_str()),
				atoi(segments.at(1).c_str()),
				atoi(segments.at(2).c_str())
			);

	return true;
}

IntVector3 IntVector3::operator* (const IntVector3 a) const
{
	return IntVector3(X * a.X, Y * a.Y, Z * a.Z);
}

IntVector3 IntVector3::operator+ (const IntVector3 a) const
{
	return IntVector3(X + a.X, Y + a.Y, Z + a.Z);
}

bool IntVector3::operator==(const IntVector3 &other) const
{
	return X == other.X && Y == other.Y && Z == other.Z;
}

bool IntVector3::operator!=(const IntVector3 &other) const
{
	return !operator==(other);
}

IntVector3 IntVector3::operator-(const IntVector3 a) const
{
	return IntVector3(X - a.X, Y - a.Y, Z - a.Z);
}
	
float IntVector3::Length() const
{
	return sqrtf((float)(X * X + Y * Y + Z * Z));
}

float IntVector3::Length_Squared() const
{
	return (float)(X * X + Y * Y + Z * Z);
}