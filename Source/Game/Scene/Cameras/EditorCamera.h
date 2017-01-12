// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_EDITOR_CAMERA_
#define _ENGINE_CAMERA_EDITOR_CAMERA_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/Scene/Cameras/EditorCameraType.h"

#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Tickable.h"

#define EDITOR_CAMERA_RAYCAST_LENGTH 32.0f

class EditorGrid;

class EditorCamera : public Camera
{
	MEMORY_ALLOCATOR(EditorCamera, "Scene");

private:
	float m_zoom;
	Vector2 m_old_mouse_position;
	Rect2D m_input_viewport;

public:
	EditorCamera(float fov, Rect2D rect);
	
	// Projection matricies.
	Matrix4 Get_Projection_Matrix();
	Matrix4 Get_View_Matrix();
	
	Rect2D	Get_Bounding_Viewport();
	Rect2D	Get_Viewport();

	float	Get_Zoom();
	void	Set_Zoom(float zoom);

	void	Set_Input_Viewport(Rect2D rect);
	Rect2D	Get_Input_Viewport();
	
	// Base functions.
	void Tick(const FrameTime& time);

};

#endif

