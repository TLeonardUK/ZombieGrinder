// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Camera.h"
#include "Engine/Platform/Platform.h"

#include "Engine/Display/GfxDisplay.h"

#include "Generic/Types/Vector4.h"

#include "Generic/Math/Math.h"

Camera::Camera() 
	: m_fov(60)
	, m_viewport(0, 0, 800, 600)
	, m_screen_viewport(0, 0, 800, 600)
	, m_near_clip(0.1f)
	, m_far_clip(40.0f)
	, m_post_process_index(0)
{
}

int Camera::Get_PostProcess_Index()
{
	return m_post_process_index;
}

void Camera::Set_PostProcess_Index(int index)
{
	m_post_process_index = index;
}

Rect2D Camera::Get_Viewport()
{
	return m_viewport;
}

void Camera::Set_Viewport(Rect2D rect)
{
	m_viewport = rect;
}

Rect2D Camera::Get_Screen_Viewport()
{
	return m_screen_viewport;
}

void Camera::Set_Screen_Viewport(Rect2D rect)
{
	m_screen_viewport = rect;
}

float Camera::Get_FOV()
{
	return m_fov;
}

void Camera::Set_FOV(float fov)
{
	m_fov = fov;
}

float Camera::Get_Near_Clip()
{
	return m_near_clip;
}

void Camera::Set_Near_Clip(float clip)
{
	m_near_clip = clip;
}

float Camera::Get_Far_Clip()
{
	return m_far_clip;
}

void Camera::Set_Far_Clip(float clip)
{
	m_far_clip = clip;
}

Rect2D Camera::Get_Bounding_Viewport()
{
	return Rect2D(m_position.X, m_position.Y, m_viewport.Width, m_viewport.Height);
}

Frustum Camera::Get_Frustum()
{
	Rect2D	  viewport = Get_Viewport();
	Vector3   rotation = Get_Rotation();
	Vector3   position = Get_Position();

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
	
	return Frustum(m_fov,
				   viewport.Width / (float) viewport.Height,
				   m_near_clip,
				   m_far_clip,
				   position,
				   center,
				   up);
}

Matrix4 Camera::Get_Projection_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Perspective(Get_FOV(), viewport.Width / viewport.Height, near_clip, far_clip);
}

Matrix4 Camera::Get_View_Matrix()
{
	Vector3 rotation = Get_Rotation();
	Vector3 position = Get_Position();

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

Vector3 Camera::Unproject(Vector3 position)
{
	GfxDisplay* display = GfxDisplay::Get();

	// Calculate viewport.
	Rect2D viewport	= m_viewport; 
	viewport.X		= viewport.X;
	viewport.Y		= viewport.Y;
	viewport.Width	= Max(viewport.Width, 0);
	viewport.Height	= Max(viewport.Height, 0);

	// Get inverse transform of projection*view matrix.
	Matrix4 inverseTransform = (Get_Projection_Matrix() * Get_View_Matrix()).Inverse();

	// Transformation to normalised coordinates.
	Vector4 inVector = Vector4
	(
		((position.X - viewport.X) / viewport.Width) * 2.0f - 1.0f,
		-((position.Y - viewport.Y) / viewport.Height) * 2.0f + 1.0f,
		2.0f * position.Z - 1.0f,
		1.0f
	);

	// Transform to world coordinates.
	Vector4 outVector = inverseTransform * inVector;
	if (outVector.W == 0.0f)
	{
		return Vector3(0, 0, 0);
	}

	return Vector3 
	(
		outVector.X / outVector.W,
		outVector.Y / outVector.W,
		outVector.Z / outVector.W
	);
}

Vector2 Camera::Unproject(Vector2 pos)
{
	Rect2D viewport = Get_Viewport();
	pos.X -= viewport.X;
	pos.Y -= viewport.Y;

	float width = viewport.Width;
	float height = viewport.Height;

	Vector4 screen_space_position = Vector4
	(
		((pos.X / (float)width) * 2.0f) - 1.0f, 
		((1.0f - (pos.Y / (float)height)) * 2.0f) - 1.0f, 
		1.0f, 
		1.0f
	);

	Matrix4 screen_to_map_matrix = (Get_Projection_Matrix() * Get_View_Matrix()).Inverse();
	Vector4 map_position = screen_to_map_matrix * screen_space_position;

	return Vector2(map_position.X, map_position.Y);
}

Vector2 Camera::Project(Vector2 pos)
{
	Rect2D viewport = Get_Viewport();
	float width = viewport.Width;
	float height = viewport.Height;

	Matrix4 world_to_view = (Get_Projection_Matrix() * Get_View_Matrix());
	Vector4 map_position = world_to_view * Vector4(pos.X, pos.Y, 1.0f, 1.0f);

	return Vector2
	(
		width * ((map_position.X + 1.0f) / 2.0f), 
		height * (1.0f - ((map_position.Y + 1.0f) / 2.0f)) // OpenGL specific, fix.
	);
}

Vector2 Camera::Project(Vector2 pos, Rect2D viewport)
{
	Matrix4 world_to_view = (Get_Projection_Matrix() * Get_View_Matrix());
	Vector4 map_position = world_to_view * Vector4(pos.X, pos.Y, 1.0f, 1.0f);

	return Vector2
	(
		viewport.X + (viewport.Width * ((map_position.X + 1.0f) / 2.0f)), 
		viewport.Y + (viewport.Height * (1.0f - ((map_position.Y + 1.0f) / 2.0f))) // OpenGL specific, fix.
	);
}