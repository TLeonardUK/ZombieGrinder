// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIPROGRESSBAR_
#define _ENGINE_UI_ELEMENTS_UIPROGRESSBAR_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

class UIProgressBar : public UIElement 
{
	MEMORY_ALLOCATOR(UIProgressBar, "UI");

protected:	
	friend class UILayout;

	float				m_progress;
	float				m_pending_progress;
	Color				m_color;
	Color				m_fore_color;
	Color				m_bg_color;

	AtlasHandle*		m_atlas;
	AtlasRenderer		m_atlas_renderer;

	FontHandle*			m_font;
	FontRenderer		m_font_renderer;

	PrimitiveRenderer	m_primitive_renderer;
		
	UIFrame				m_background_frame;

	float				m_draw_progress;

	bool				m_show_progress;

public:
	UIProgressBar();
	~UIProgressBar();

	void Refresh();
	void After_Refresh();

	void Set_Progress(float progress);
	void Set_Pending_Progress(float progress);

	void Set_Color(Color color);
	Color Get_Color();

	void Set_Foreground_Color(Color color);
	Color Get_Foreground_Color();

	void Set_Background_Color(Color color);
	Color Get_Background_Color();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

