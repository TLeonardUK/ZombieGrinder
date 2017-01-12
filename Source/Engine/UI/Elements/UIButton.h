// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIBUTTON_
#define _ENGINE_UI_ELEMENTS_UIBUTTON_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Input/OutputBindings.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;
class OnlineUser;

class UIButton : public UIElement 
{
	MEMORY_ALLOCATOR(UIButton, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*		m_atlas;
	AtlasRenderer		m_atlas_renderer;

	FontHandle*			m_font;
	FontRenderer		m_font_renderer;
	FontRenderer		m_font_renderer_no_shadow;
	MarkupFontRenderer	m_markup_font_renderer;

	UIFrame				m_background_frame;
	UIFrame				m_background_active_frame;
	UIFrame				m_background_press_frame;
	
	UIFrame				m_game_background_frame;
	UIFrame				m_game_background_active_frame;
	UIFrame				m_game_background_press_frame;

	bool				m_use_markup;

	bool				m_selecting;
	float				m_select_timer;

	Color				m_frame_color;

	Rect2D					m_padding;

	OutputBindings::Type	m_hotkey;

	TextAlignment::Type		m_halign;
	TextAlignment::Type		m_valign;

	bool					m_no_frame;

	OnlineUser*				m_select_user;
	JoystickState*			m_select_joystick;

	bool					m_game_style;

	bool					m_fast_anim;

	enum
	{
		SELECT_FLASH_DURATION = 300,
		SELECT_FLASH_INTERVAL = 50,
		SELECT_FLASH_FAST_DURATION = 50
	};

public:
	UIButton();
	~UIButton();

	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

