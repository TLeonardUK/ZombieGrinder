// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VECTOR3_
#define _GENERIC_VECTOR3_

#include "Generic/Types/IntVector3.h"

#include <string>

struct Vector3
{
//	MEMORY_ALLOCATOR(Vector3, "Data Types");

public:
	float X, Y, Z;

	// Constructors.
	Vector3();
	Vector3(float x, float y, float z);

	// Operator overloads.
	Vector3 operator* (const float a) const;
	Vector3 operator* (const Vector3 a) const;
	Vector3 operator* (const IntVector3 a) const;
	Vector3 operator+ (const Vector3 a) const;
	Vector3 operator+ (const float a) const;
	Vector3 operator/ (const Vector3 a) const;
	Vector3 operator/ (const float a) const;
	Vector3 operator- (const Vector3 a) const;
	Vector3 operator- (const IntVector3 a) const;
	Vector3 operator- () const;
	bool operator==(const Vector3 &other) const;
	bool operator!=(const Vector3 &other) const;
	
	// General.
	float   Dot(Vector3 v) const;
	Vector3	Cross(Vector3 a) const;
	Vector3 Normalize() const;
	float	Length() const;
	float	Length_Squared() const;

	Vector3	Limit(float magnitude) const;

	Vector3 Direction() const;
	Vector3 Right() const;

	float Get_Min_Element() const;
	float Get_Max_Element() const;
	Vector3 Get_Abs() const;
	Vector3 Floor() const;

	Vector3 Lerp(Vector3 other, float delta) const;

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Vector3& result);

};

#endif