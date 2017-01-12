// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Cameras/GameCamera.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Generic/Math/Math.h"
#include "Engine/Scene/Map/Map.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Game/Runner/GameOptions.h"

#include "Engine/Renderer/RenderPipeline.h"

#include <algorithm>

GameCamera::GameCamera(float fov, Rect2D viewport)
{
	m_fov			= fov;
	m_viewport		= viewport;
	m_near_clip		= 0.0f;
	m_far_clip		= (MAP_LAYER_DEPTH + 1) * (DEFAULT_MAP_DEPTH + 1);
}

Matrix4 GameCamera::Get_Projection_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Orthographic(0, viewport.Width, viewport.Height, 0, near_clip, far_clip);
}

Matrix4 GameCamera::Get_View_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	Vector3 pos = m_position;
	
	// With HQ2X we have to translate to absolute pixel precision. We will do shifting
	// by sub-units afterwards.
	if (*GameOptions::hq2x_enabled)
	{
		return Matrix4::Translate(-pos.Floor());
	}
	else
	{
		return Matrix4::Translate(-pos);
	}
}

Rect2D GameCamera::Get_Viewport()
{
	return Rect2D
	(
		m_viewport.X,
		m_viewport.Y,
		m_viewport.Width,
		m_viewport.Height
	);
}

Rect2D GameCamera::Get_Bounding_Viewport()
{
	return Rect2D
	(
		m_position.X, 
		m_position.Y, 
		m_viewport.Width, 
		m_viewport.Height 
	);
}

Rect2D GameCamera::Get_Screen_Viewport()
{
	Rect2D screen_viewport = Camera::Get_Screen_Viewport();
	Rect2D bounding_viewport = Get_Bounding_Viewport();

	// With HQ2X we render a 2-2 pixel ratio. Which we then scale to appropriate screen size afterwards.
	if (*GameOptions::hq2x_enabled)
	{
		return Rect2D
		(
			m_viewport.X,
			m_viewport.Y,
			bounding_viewport.Width * 2,
			bounding_viewport.Height * 2
		);
	}
	else
	{
		return screen_viewport;
	}
}

void GameCamera::Tick(const FrameTime& time)
{
	// Done in game-mode script.
}