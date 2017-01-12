// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_
#define _ENGINE_CAMERA_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Frustum.h"
#include "Generic/Types/Matrix4.h"

#include "Engine/Engine/FrameTime.h"
#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Tickable.h"

class Camera : public Actor, public Tickable
{
	MEMORY_ALLOCATOR(Camera, "Scene");

protected:
	Rect2D	m_viewport;
	Rect2D	m_screen_viewport;
	float	m_fov;
	float	m_near_clip;
	float	m_far_clip;
	int		m_post_process_index;

public:

	// Constructors.
	Camera();

	// Get modifiers.
	virtual Frustum Get_Frustum();

	virtual int Get_PostProcess_Index();
	virtual void Set_PostProcess_Index(int index);

	virtual Rect2D Get_Viewport();
	virtual void Set_Viewport(Rect2D rect);

	virtual Rect2D Get_Screen_Viewport();
	virtual void Set_Screen_Viewport(Rect2D rect);

	virtual float Get_FOV();
	virtual void  Set_FOV(float fov);

	virtual float Get_Near_Clip();
	virtual void  Set_Near_Clip(float clip);

	virtual float Get_Far_Clip();
	virtual void  Set_Far_Clip(float clip);

	virtual Matrix4 Get_Projection_Matrix();
	virtual Matrix4 Get_View_Matrix();

	virtual Rect2D Get_Bounding_Viewport();

	// Unproject screen coordinates.
	virtual Vector3 Unproject(Vector3 position);
	virtual Vector2 Unproject(Vector2 position);

	virtual Vector2 Project(Vector2 position, Rect2D viewport);
	virtual Vector2 Project(Vector2 position);

};

#endif

