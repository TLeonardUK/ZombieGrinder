// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LIGHT_
#define _ENGINE_LIGHT_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Color.h"
#include "Generic/Types/Matrix4.h"

#include "Engine/Engine/FrameTime.h"
#include "Engine/Scene/Actor.h"

struct LightType
{
	enum Type
	{
		Vector2		= 0,
		Ambient		= 1,
		Directional = 2,
		Spotlight	= 3
	};
};

class Light : public Actor
{
	MEMORY_ALLOCATOR(Light, "Scene");

protected:
	LightType::Type m_type;
	float			m_radius;
	float			m_outer_radius;
	Color			m_color;
	bool			m_shadow_caster;

public:

	// Constructors.
	Light(LightType::Type type);
	Light(LightType::Type type, float radius);
	Light(LightType::Type type, float radius, Color color);
	Light(LightType::Type type, float radius, float outer_radius, Color color);

	// Get modifiers.
	virtual LightType::Type Get_Type();
	virtual void Set_Type(LightType::Type type);

	virtual float Get_Radius();
	virtual void Set_Radius(float radius);
	
	virtual float Get_Outer_Radius();
	virtual void Set_Outer_Radius(float radius);
	
	virtual bool Get_Shadow_Caster();
	virtual void Set_Shadow_Caster(bool caster);

	virtual Color Get_Color();
	virtual void Set_Color(Color color);
	
	virtual Matrix4 Get_Projection_Matrix();
	virtual Matrix4 Get_View_Matrix();

};

#endif

