// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_EDITOR_EDITORFOREGROUND_
#define _ENGINE_SCENE_EDITOR_EDITORFOREGROUND_

#include "Engine/Renderer/Drawable.h"

struct AtlasFrame;
class UIScene_Editor;

class EditorForeground : public IDrawable
{
	MEMORY_ALLOCATOR(EditorForeground, "Scene");

private:
	float m_depth;

	bool m_draw_randgen;
	bool m_draw_grid;

	UIScene_Editor* m_editor;

public:
	EditorForeground(UIScene_Editor* editor);

	virtual void  Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance);
	virtual float Get_Draw_Depth();
	virtual bool  Is_Visible_From(Rect2D viewport);
	
	void Set_Draw_RandGen_Template(bool val)
	{
		m_draw_randgen = val;
	}

	bool Get_Draw_RandGen_Template()
	{
		return m_draw_randgen;
	}

	void Set_Draw_Grid_Template(bool val)
	{
		m_draw_grid = val;
	}

	bool Get_Draw_Grid_Template()
	{
		return m_draw_grid;
	}

};

#endif

