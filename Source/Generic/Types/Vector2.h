// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VECTOR2_
#define _GENERIC_VECTOR2_

#include <string>

class Vector2
{
//	MEMORY_ALLOCATOR(Point, "Data Types");

public:
	float X, Y;

	Vector2();
	Vector2(int x, int y);
	Vector2(float x, float y);
	
	// Operators.
	bool operator==(Vector2& rhs) const;
	bool operator!=(Vector2& rhs) const;
	Vector2 operator+ (const Vector2 a) const;
	Vector2 operator- (const Vector2 a) const;
	Vector2 operator* (const Vector2 a) const;
	Vector2 operator* (const float a) const;
	Vector2 operator/ (const Vector2 a) const;

	Vector2 Round() const;
	Vector2 MaxValue(const Vector2 a) const;
	Vector2 MinValue(const Vector2 a) const;
	float	Length() const;
	float	Length_Squared() const;

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Vector2& result);

};

#endif