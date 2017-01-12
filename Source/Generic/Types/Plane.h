// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_PLANE_
#define _GENERIC_PLANE_

#include "Generic/Types/Vector3.h"

class Plane
{
	//MEMORY_ALLOCATOR(Plane, "Data Types");

private:
	Vector3 m_normal;
	Vector3 m_point;
	float   m_d;

public:
	Plane()
	{
	}

	Plane(Vector3 v1, 
		  Vector3 v2, 
		  Vector3 v3)
	{
		Vector3 aux1 = v1 - v2;
		Vector3 aux2 = v3 - v2;

		m_normal = aux2.Cross(aux1);

		m_normal = m_normal.Normalize();
		m_point = v2;

		m_d = -(m_normal.Dot(m_point));
	}

	Vector3 Get_Normal()
	{
		return m_normal;
	}

	float Get_Distance()
	{
		return m_d;
	}

	float Get_Distance_To_Point(Vector3 point)
	{
		float distance = m_d + m_normal.Dot(point);
		return distance;
	}

};

#endif