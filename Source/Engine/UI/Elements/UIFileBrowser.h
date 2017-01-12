// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIFILEBROWSER_
#define _ENGINE_UI_ELEMENTS_UIFILEBROWSER_

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

class UIFileBrowser : public UIElement 
{
	MEMORY_ALLOCATOR(UIFileBrowser, "UI");

protected:	
	friend class UILayout;
	
	FontHandle*					m_font;
	FontRenderer				m_font_renderer;

	UIFrame						m_background_frame;
	UIFrame						m_border_frame;
	UIFrame						m_select_background_frame;

	AtlasFrame*					m_folder_icon;
	AtlasFrame*					m_file_icon;

	std::string					m_new_path;
	
	UISlider*					m_slider;
	float						m_slider_width;
	float						m_scroll_range;

	UIToolbar*					m_tool_bar;
	float						m_tool_bar_height;

	UIToolbarItem*				m_create_folder_item;
	UIToolbarItem*				m_level_up_item;
	
	UITextBox*					m_path_text_box;
	UITextBox*					m_text_box;

	float						m_text_box_height;

	std::string					m_directory;
	std::vector<std::string>	m_files;
	std::vector<std::string>	m_directories;
	
	float						m_item_height;
	float						m_item_padding;

	PrimitiveRenderer			m_primitive_renderer;
	AtlasRenderer				m_atlas_renderer;

	bool						m_is_filename_valid;
	std::string					m_extension;

public:
	UIFileBrowser();
	~UIFileBrowser();

	void Set_Extension_Filter(std::string ext);
	void Set_Path(std::string path);
	std::string Get_Path();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	
	void Draw_Item(const FrameTime& time, UIManager* manager, UIScene* scene, Rect2D item_box, std::string text, bool is_folder);

};

#endif

