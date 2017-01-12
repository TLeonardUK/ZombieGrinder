// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UITEXTPANEL_
#define _ENGINE_UI_ELEMENTS_UITEXTPANEL_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/Elements/UISlider.h"
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

class UITextPanel : public UIElement 
{
	MEMORY_ALLOCATOR(UITextPanel, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*					m_atlas;
	AtlasRenderer					m_atlas_renderer;
		
	FontHandle*						m_font;
	FontRenderer					m_font_renderer;
	MarkupFontRenderer				m_markup_font_renderer;

	UISlider*						m_slider;

	Vector2							m_text_size;
	
	Rect2D							m_padded_box;
	Rect2D							m_slider_box;
	Rect2D							m_item_box;
	float							m_scroll_range;

	std::string						m_draw_value;

	std::vector<std::string>		m_lines;
	float							m_line_height;

	bool							m_draw_text_only;
	bool							m_override_global_alpha;

	bool							m_shadowed_text;

	std::string						m_start_line_prefix;

protected:
	void Text_Changed();

public:
	UITextPanel();
	~UITextPanel();

	void Set_Start_Line_Prefix(std::string value)
	{
		m_start_line_prefix = value;
	}
	
	void RefreshBoxes(UIManager* manager);
	void Refresh();
	void After_Refresh();
	bool Is_Focusable();
	
	void Append_Text(std::string text);
	void Clear_Text();
	void Set_Text(std::string text);	

	void Set_Scroll_Offset(float offset);
	float Get_Scroll_Offset();

	void Override_Global_Alpha(bool value);

	void Set_Draw_Text_Only(bool value);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

