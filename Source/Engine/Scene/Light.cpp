// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Light.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

Light::Light(LightType::Type type)
	: m_type(type)
	, m_radius(1.0f)
	, m_color(Color(255, 255, 255, 255))
	, m_shadow_caster(false)
{
}

Light::Light(LightType::Type type, float radius)
	: m_type(type)
	, m_radius(radius)
	, m_outer_radius(radius)
	, m_color(Color(255, 255, 255, 255))
	, m_shadow_caster(false)
{
}

Light::Light(LightType::Type type, float radius, Color color)
	: m_type(type)
	, m_radius(radius)
	, m_color(color)
	, m_outer_radius(radius)
	, m_shadow_caster(false)
{
}

Light::Light(LightType::Type type, float radius, float outer_radius, Color color)
	: m_type(type)
	, m_radius(radius)
	, m_outer_radius(outer_radius)
	, m_color(color)
	, m_shadow_caster(false)
{
}

// Get modifiers.
LightType::Type Light::Get_Type()
{
	return m_type;
}

void Light::Set_Type(LightType::Type type)
{	
	DBG_ASSERT(m_type != LightType::Ambient || m_shadow_caster == false);
	m_type = type;
}

float Light::Get_Radius()
{
	return m_radius;
}

void Light::Set_Radius(float radius)
{
	m_radius = radius;
}

float Light::Get_Outer_Radius()
{
	return m_outer_radius;
}

void Light::Set_Outer_Radius(float radius)
{
	m_outer_radius = radius;
}

bool Light::Get_Shadow_Caster()
{
	return m_shadow_caster;
}

void Light::Set_Shadow_Caster(bool caster)
{
	DBG_ASSERT(m_type != LightType::Ambient);
	m_shadow_caster = caster;
}

Color Light::Get_Color()
{
	return m_color;
}

void Light::Set_Color(Color color)
{
	m_color = color;
}

Matrix4 Light::Get_Projection_Matrix()
{
	Camera* camera = RenderPipeline::Get()->Get_Active_Camera();
	Frustum camera_frustum = camera->Get_Frustum();
	Rect2D viewport = camera->Get_Viewport();

	Vector3 position = camera->Get_Position();

	Matrix4 view_matrix = Get_View_Matrix();
	const Vector3* corners = camera_frustum.Get_Corners();

	switch (m_type)
	{
		case LightType::Spotlight:
			{
				return Matrix4::Perspective(45.0f, 1.0f, camera->Get_Near_Clip(), camera->Get_Far_Clip()); 
			}
		case LightType::Directional:	
			{	
				// Convert frustum corners into view space and calculate bounds.
				Vector3 mins(0.0f, 0.0f, 0.0f);
				Vector3 maxs(0.0f, 0.0f, 0.0f);

				for (int i = 0; i < 8; i++)
				{
					Vector4 vec = view_matrix * Vector4(corners[i], 1.0f);

					if (mins.X > vec.X) mins.X = vec.X;
					if (maxs.X < vec.X) maxs.X = vec.X;
					if (mins.Y > vec.Y) mins.Y = vec.Y;
					if (maxs.Y < vec.Y) maxs.Y = vec.Y;
					if (mins.Z > vec.Z) mins.Z = vec.Z;
					if (maxs.Z < vec.Z) maxs.Z = vec.Z;
				}

			//	float worldUnitsPerTexel = (maxs.X - mins.X) / viewport.Width;
			//	mins.X = ceil(mins.X / worldUnitsPerTexel) * worldUnitsPerTexel;
			//	maxs.X = ceil(maxs.X / worldUnitsPerTexel) * worldUnitsPerTexel;
			//	mins.Y = ceil(mins.Y / worldUnitsPerTexel) * worldUnitsPerTexel;
			//	maxs.Y = ceil(maxs.Y / worldUnitsPerTexel) * worldUnitsPerTexel;

				return Matrix4::Orthographic(mins.X, maxs.X, maxs.Y, mins.Y, -mins.Z, -maxs.Z);
			}
		default: DBG_ASSERT(false);
	}

	return Matrix4::Identity(); // Just shuts up C4715: Not all control paths return a value.
}

Matrix4 Light::Get_View_Matrix()
{
	Camera* camera			= RenderPipeline::Get()->Get_Active_Camera();
	Frustum camera_frustum	= camera->Get_Frustum();
	
	Vector3 position = Get_Position();
	Vector3 rotation = Get_Rotation();
	
	switch (m_type)
	{
	case LightType::Spotlight:
		{
			// Calculate view matrix.
			float horizontal = rotation.Y;
			float vertical   = rotation.Z;
			Vector3 direction
			(
				cos(vertical) * sin(horizontal),
				sin(vertical),
				cos(vertical) * cos(horizontal)
			);
			Vector3	right
			(
				sin(horizontal - 3.14f / 2.0f),
				0,
				cos(horizontal - 3.14f / 2.0f)
			);
			Vector3 center = position + direction;
			Vector3 up = right.Cross(direction);
	
			return Matrix4::LookAt(position, center, up);
		}
	case LightType::Directional:
		{
			Vector3 centroid				= camera_frustum.Get_Centroid();
			float	distance_from_centroid  = 50.0f;//camera->Get_Far_Clip() * 0.01f;

			// Calculate view matrix.
			float horizontal = rotation.Y;
			float vertical   = rotation.Z;
			Vector3 direction
			(
				cos(vertical) * sin(horizontal),
				sin(vertical),
				cos(vertical) * cos(horizontal)
			);
			Vector3	right
			(
				sin(horizontal - 3.14f / 2.0f),
				0,
				cos(horizontal - 3.14f / 2.0f)
			);
			Vector3 lposition	= centroid - (direction * distance_from_centroid);
			Vector3 lcenter		= position + direction;
			Vector3 lup			= right.Cross(direction);
	
			return Matrix4::LookAt(lposition, lcenter, lup);
		}
	default:
		DBG_ASSERT(false);
	}

	return Matrix4::Identity(); // Just shuts up C4715: Not all control paths return a value.
}
