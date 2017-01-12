// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VECTOR4_
#define _GENERIC_VECTOR4_

#include "Generic/Types/Vector3.h"

#include <string>

struct Vector4
{
//	MEMORY_ALLOCATOR(Vector4, "Data Types");

public:
	float X, Y, Z, W;

	// Constructors.
	Vector4();
	Vector4(float x, float y, float z, float w);
	Vector4(Vector3 v, float w);
	bool operator==(const Vector4 &other) const;
	bool operator!=(const Vector4 &other) const;

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Vector4& result);

};

#endif