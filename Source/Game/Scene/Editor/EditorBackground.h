// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_EDITOR_EDITORBACKGROUND_
#define _ENGINE_SCENE_EDITOR_EDITORBACKGROUND_

#include "Engine/Renderer/Drawable.h"

struct AtlasFrame;
class UIScene_Editor;

class EditorBackground : public IDrawable
{
	MEMORY_ALLOCATOR(EditorBackground, "Scene");

private:
	AtlasFrame* m_background_frame;
	float m_depth;

	UIScene_Editor* m_editor;

public:
	EditorBackground(UIScene_Editor* editor);

	virtual void  Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance);
	virtual float Get_Draw_Depth();
	virtual bool  Is_Visible_From(Rect2D viewport);

};

#endif

