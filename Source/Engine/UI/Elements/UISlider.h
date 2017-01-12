// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UISLIDER_
#define _ENGINE_UI_ELEMENTS_UISLIDER_

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

struct UISliderDirection
{
	enum Type
	{
		Horizontal,
		Vertical
	};
};

class UISlider : public UIElement 
{
	MEMORY_ALLOCATOR(UISlider, "UI");

protected:	
	friend class UILayout;

	float				m_progress;
	float				m_draw_progress;

	Color				m_color;
	
	AtlasHandle*		m_atlas;
	AtlasRenderer		m_atlas_renderer;

	FontHandle*			m_font;
	FontRenderer		m_font_renderer;

	PrimitiveRenderer	m_primitive_renderer;
		
	UIFrame				m_background_frame;
	UIFrame				m_active_background_frame;
	UIFrame				m_knob_frame;

	Rect2D				m_bar_box;
	Rect2D				m_active_bar_box;
	Rect2D				m_knob_box;

	Vector2				m_mouse_offset;
	bool				m_moving_mouse;

	bool				m_focusable;
	
	bool					m_game_style;

	UISliderDirection::Type	m_direction;

public:
	UISlider();
	~UISlider();

	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	void Set_Game_Style(bool value) { m_game_style = value; }

	bool Is_Moving() { return m_moving_mouse; }

	void Cancel_Movement();

	void Set_Focusable(bool focusable);

	bool Focus_Left(UIManager* manager);
	bool Focus_Right(UIManager* manager);

	void Refresh_Boxes(UIManager* manager);
	
	void Set_Direction(UISliderDirection::Type direction);

	void Set_Progress(float progress);
	float Get_Progress();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

