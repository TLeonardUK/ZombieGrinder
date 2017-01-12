// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_GAME_CAMERA_
#define _ENGINE_CAMERA_GAME_CAMERA_

#include "Generic/Types/Rect2D.h"

#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Tickable.h"

class GameCamera : public Camera
{
	MEMORY_ALLOCATOR(GameCamera, "Scene");

private:
/*
	std::vector<Actor*>	m_targets;
	float				m_zoom;
	float				m_zoom_padding;
	Vector3				m_target_position;
*/

public:
	GameCamera(float fov, Rect2D rect);
	
	// Base functions.
	void Tick(const FrameTime& time);
	
	// Projection matricies.
	Matrix4 Get_Projection_Matrix();
	Matrix4 Get_View_Matrix();
	
	Rect2D	Get_Bounding_Viewport();
	Rect2D	Get_Viewport();

	Rect2D Get_Screen_Viewport();

/*
	// Target settings.
	void					Add_Target		(Actor* actor);
	void					Remove_Target	(Actor* actor);
	std::vector<Actor*>&	Get_Targets		();

	// Effects
*/

};

#endif

