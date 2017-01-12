// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_ACTORS_COMPONENTS_DRAWBLE_DRAWABLECOMPONENT_
#define _ENGINE_SCENE_ACTORS_COMPONENTS_DRAWBLE_DRAWABLECOMPONENT_

#include "Engine/Renderer/Drawable.h"

#include "Game/Scene/Actors/CompositeActor.h"

class CompositeActor;

class DrawableComponent : public Component, public IDrawable
{
	MEMORY_ALLOCATOR(DrawableComponent, "Scene");

private:

public:
	DrawableComponent();
	virtual ~DrawableComponent();

	virtual float Get_Draw_Depth();

	bool Is_Visible_From(Rect2D viewport);

	virtual void Pre_Draw();

	void Draw_Debug(const FrameTime& time, RenderPipeline* pipeline);

};

#endif

