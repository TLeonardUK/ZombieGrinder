// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Cameras/OrthoCamera.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Generic/Math/Math.h"
#include "Engine/Scene/Map/Map.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIManager.h"

OrthoCamera::OrthoCamera(float fov, Rect2D viewport)
{
	m_fov = fov;
	m_viewport = viewport;
	//m_far_clip = MAP_LAYER_DEPTH * (DEFAULT_MAP_DEPTH + 1);
	//m_far_clip = 0.0f;

	m_near_clip = 0.0f;
	m_far_clip = 1.0f;
}

Matrix4 OrthoCamera::Get_Projection_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Orthographic(0, viewport.Width, viewport.Height, 0, near_clip, far_clip);
}

Matrix4 OrthoCamera::Get_View_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Translate(-m_position.Floor());
}

void OrthoCamera::Tick(const FrameTime& time)
{
}