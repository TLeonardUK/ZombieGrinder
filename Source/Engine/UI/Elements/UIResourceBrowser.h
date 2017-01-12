// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIRESOURCEBROWSER_
#define _ENGINE_UI_ELEMENTS_UIRESOURCEBROWSER_

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
class UIToolbar;
class UISlider;
class UITextBox;
class UIToolbarItem;
class SoundChannel;

struct UIResourceBrowserMode
{
	enum Type
	{
		Image,
		Audio,
		Animation
	};
};

class UIResourceBrowser : public UIElement 
{
	MEMORY_ALLOCATOR(UIResourceBrowser, "UI");

protected:	
	friend class UILayout;

	FontHandle*					m_font;
	FontRenderer				m_font_renderer;

	UIFrame						m_background_frame;
	UIFrame						m_border_frame;
	UIFrame						m_select_background_frame;

	AtlasFrame*					m_folder_icon;
	AtlasFrame*					m_file_icon;

	UIToolbar*					m_tool_bar;

	UISlider*					m_slider;
	float						m_slider_width;
	float						m_scroll_range;

	float						m_tool_bar_height;
	UITextBox*					m_filter_text_box;

	std::vector<std::string>	m_items;
	std::string					m_filter;
	UIResourceBrowserMode::Type	m_mode;

	std::vector<std::string>	m_resources;

	float						m_item_height;
	float						m_item_padding;

	PrimitiveRenderer			m_primitive_renderer;
	AtlasRenderer				m_atlas_renderer;

	std::string					m_selected_resource;

	float						m_anim_timer;

	SoundChannel*				m_sfx_channel;

public:
	UIResourceBrowser();
	~UIResourceBrowser();

	void Refresh();
	void After_Refresh();
	
	std::string Get_Selected_Item();

	void Set_Filter(std::string filter);
	void Set_Mode(UIResourceBrowserMode::Type mode);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

	void Draw_Item(const FrameTime& time, UIManager* manager, UIScene* scene, Rect2D item_box, std::string text);

};

#endif

