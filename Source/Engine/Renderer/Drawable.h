// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_DRAWABLE_
#define _ENGINE_DRAWABLE_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Patterns/Singleton.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderPipelineTypes.h"

// Base class for all objects that can be drawn by the renderer.
class IDrawable
{
	MEMORY_ALLOCATOR(IDrawable, "Rendering");

protected:
	Camera* m_draw_camera;
	int m_render_slot_hash;

public:

	IDrawable();

	// Base functions.	
	virtual void	Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance) = 0;
	virtual void	Draw_Debug(const FrameTime& time, RenderPipeline* pipeline) { }

	// Called before drawing starts. Called only once unlock Draw which can be called multiple
	// times in splitscreen.
	virtual void	Pre_Draw();

	// Sets the camera that this object draws for, NULL for all.
	void			Set_Draw_Camera(Camera* camera);
	Camera*			Get_Draw_Camera();

	// Sets the render slot for this object.
	void			Set_Render_Slot(const char* name);
	int				Get_Render_Slot_Hash();

	// Get draw depth, used for sorting back to front or vis-versa by the renderer.
	virtual float	Get_Draw_Depth() = 0;

	// Returns true if this drawable object is visible from the given viewport.
	virtual bool	Is_Visible_From(Rect2D viewport) = 0;

	// Adds all draw instances for this drawable.
	virtual void	Add_Draw_Instances(const FrameTime& time, std::vector<DrawInstance>& instances) 
	{
		DrawInstance instance;
		instance.draw_depth = Get_Draw_Depth();
		instance.draw_index = 0;
		instance.drawable = this;
		instance.transparent = false;

		instances.push_back(instance);
	}

};

#endif

