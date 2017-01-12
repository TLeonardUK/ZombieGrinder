// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UILABEL_
#define _ENGINE_UI_ELEMENTS_UILABEL_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Generic/Types/Color.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;

class UILabel : public UIElement 
{
	MEMORY_ALLOCATOR(UILabel, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*			m_atlas;
	AtlasRenderer			m_atlas_renderer;
	
	FontHandle*				m_font;
	FontRenderer			m_font_renderer;
	MarkupFontRenderer		m_markup_font_renderer;

	UIManager*				m_manager;
	AtlasFrame*				m_background_frame;

	TextAlignment::Type		m_halign;
	TextAlignment::Type		m_valign;

	Rect2D					m_padding;

	Color					m_color;

	Color					m_frame_color;

	std::string				m_frame_name;
	UIFrame					m_frame;

	bool					m_use_markup;
	bool					m_word_wrap;
	bool					m_word_wrap_in_date;
	std::string				m_wrapped_text;

	bool					m_game_style;

public:
	UILabel();
	~UILabel();

	Color Get_Color()
	{
		return m_color;
	}
	void Set_Color(Color color)
	{
		m_color = color;
	}

	void Set_Use_Markup(bool markup)
	{
		m_use_markup = markup;
	}

	void Set_Value(std::string value)
	{
		UIElement::Set_Value(value);

		m_word_wrap_in_date = false;
	}
		
	void Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

