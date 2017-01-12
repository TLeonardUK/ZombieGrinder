// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_FRUSTUM_
#define _GENERIC_FRUSTUM_

#include "Generic/Types/AABB.h"
#include "Generic/Types/Sphere.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Plane.h"
#include "Generic/Types/Vector2.h"

#include <vector>

class Frustum
{
	//MEMORY_ALLOCATOR(Frustum, "Data Types");

private:
	Plane	m_planes[6];
	Vector3 m_corners[8];

	float m_near_distance;
	float m_far_distance;

	float m_near_width;
	float m_near_height;

	float m_far_width;
	float m_far_height;

	float m_aspect_ratio;
	float m_angle;
	float m_tangent;

public:

	struct IntersectionResult
	{
		enum Type
		{
			Outside,
			Intersects,
			Inside
		};
	};

	struct Planes
	{
		enum Type
		{
			Top,
			Bottom,
			Left,
			Right,
			Near,
			Far
		};
	};

	struct Corners
	{
		enum Type
		{
			NearTopLeft,
			NearTopRight,
			NearBottomLeft,
			NearBottomRight,
			FarTopLeft,
			FarTopRight,
			FarBottomLeft,
			FarBottomRight
		};
	};	

	Frustum();
	Frustum(float angle,
			float aspect_ratio,
			float near,
			float far,
			const Vector3& pos,
			const Vector3& target,
			const Vector3& up);
	Frustum(Vector2 size,
			float near,
			float far,
			const Vector3& pos,
			const Vector3& target,
			const Vector3& up);

	IntersectionResult::Type Intersects(const AABB& aabb);
	IntersectionResult::Type Intersects(const Sphere& sphere);

	Vector3			Get_Centroid();
	const Vector3*	Get_Corners();

};

#endif