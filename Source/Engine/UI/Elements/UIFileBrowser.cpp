// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIFileBrowser.h"
#include "Engine/UI/Elements/UIToolbar.h"
#include "Engine/UI/Elements/UITextBox.h"
#include "Engine/UI/Elements/UIToolbarItem.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

UIFileBrowser::UIFileBrowser()
	: m_tool_bar(NULL)
	, m_tool_bar_height(30)
	, m_slider(NULL)
	, m_slider_width(10.0f)
	, m_text_box(NULL)
	, m_text_box_height(28)
	, m_path_text_box(NULL)
	, m_scroll_range(0)
	, m_item_height(25.0f)
	, m_item_padding(4.0f)
	, m_new_path("")
	, m_is_filename_valid(false)
	, m_extension("")
{

}

UIFileBrowser::~UIFileBrowser()
{
}

std::string UIFileBrowser::Get_Path()
{
	std::string filename = m_text_box->Get_Value();
	if (m_is_filename_valid == true)
		return Platform::Get()->Normalize_Path(m_directory + "/" + filename);
	else
		return "";
}

void UIFileBrowser::Set_Path(std::string path)
{
	Platform* platform = Platform::Get();
	std::string file = "";
	std::string dir = "";

	path = platform->Get_Absolute_Path(path.c_str());

	if (platform->Is_Directory(path.c_str()))
	{
		dir = path.c_str();
		file = "";
	}
	else
	{
		file = platform->Extract_Filename(path.c_str());
		dir  = platform->Extract_Directory(path.c_str());
	}

	if (dir[dir.size() - 1] != '/' &&
		dir[dir.size() - 1] != '\\')
	{
		dir += "/";
	}

	m_directory = dir;

	m_directories.clear();
	m_files.clear();

	platform->List_Directory(m_directory.c_str(), m_directories, ListDirectoryFlags::DirectoriesOnly);
	platform->List_Directory(m_directory.c_str(), m_files, ListDirectoryFlags::FilesOnly);

	// Strip all files that don't match extension.
	for (std::vector<std::string>::iterator iter = m_files.begin(); iter != m_files.end(); )
	{
		std::string subpath = *iter;
		std::string ext = Platform::Get()->Extract_Extension(subpath);

		if (ext != m_extension)
		{
			iter = m_files.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	m_scroll_range = (m_directories.size() + m_files.size()) * m_item_height;

	m_path_text_box->Set_Value(dir.c_str());
	m_text_box->Set_Value(file.c_str());
	m_slider->Set_Progress(0.0f);
}

void UIFileBrowser::Set_Extension_Filter(std::string ext)
{
	m_extension = ext;
}	

void UIFileBrowser::Refresh()
{
	// Calculate screen-space box.
	UIManager* manager	= GameEngine::Get()->Get_UIManager();
	m_font				= manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, false);

	m_screen_box = Calculate_Screen_Box();

	m_background_frame			= UIFrame("filebrowser_background_#");
	m_border_frame				= UIFrame("filebrowser_border_#");
	m_select_background_frame	= UIFrame("toolbaritem_background_hover_#");

	m_folder_icon				= ResourceFactory::Get()->Get_Atlas_Frame("icon_folder");
	m_file_icon					= ResourceFactory::Get()->Get_Atlas_Frame("icon_file");

	// Add the menu bar.
	if (m_tool_bar == NULL)
	{
		m_tool_bar = new UIToolbar();
		m_tool_bar->Set_Scene(m_scene);
		m_tool_bar->Set_Parent(this);		
		m_tool_bar->Set_Box(Rect2D(0.0f, 0.0f, m_screen_box.Width, m_tool_bar_height));			
		m_tool_bar->Set_Alignment(UIToolbarAlignment::Far);
		Add_Child(m_tool_bar);

		m_create_folder_item = new UIToolbarItem();		
		m_create_folder_item->Set_Name("create_folder_toolbar_item");
		m_create_folder_item->Set_Icon(ResourceFactory::Get()->Get_Atlas_Frame("icon_folder"));
		m_tool_bar->Add_Child(m_create_folder_item);

		m_level_up_item = new UIToolbarItem();
		m_level_up_item->Set_Name("level_up_toolbar_item");
		m_level_up_item->Set_Icon(ResourceFactory::Get()->Get_Atlas_Frame("icon_folder_up"));
		m_tool_bar->Add_Child(m_level_up_item);
	}
	else
	{
		m_tool_bar->Set_Box(Rect2D(0.0f, 0.0f, m_screen_box.Width, m_tool_bar_height));		
	}
		
	// Add the slider.
	if (m_slider == NULL)
	{
		m_slider = new UISlider();
		m_slider->Set_Scene(m_scene);
		m_slider->Set_Parent(this);		
		m_slider->Set_Box(Rect2D(m_screen_box.Width - m_slider_width, m_tool_bar_height, m_slider_width, m_screen_box.Height - m_tool_bar_height - m_text_box_height));		
		m_slider->Set_Direction(UISliderDirection::Vertical);
		m_slider->Set_Focusable(false);
		m_slider->Set_Progress(1.0f);
		m_slider->Set_Game_Style(false);
		Add_Child(m_slider);
	}
	else
	{
		m_slider->Set_Box(Rect2D(m_screen_box.Width - m_slider_width, m_tool_bar_height, m_slider_width, m_screen_box.Height - m_tool_bar_height - m_text_box_height));		
	}
	
	// Add the path text box.
	if (m_path_text_box == NULL)
	{
		m_path_text_box = new UITextBox();
		m_path_text_box->Set_Scene(m_scene);
		m_path_text_box->Set_Parent(this);		
		m_path_text_box->Set_Box(Rect2D(0.0f, 0.0f, m_screen_box.Width * 0.5f, m_tool_bar_height));	
		m_path_text_box->Set_Game_Style(false);		
		m_path_text_box->Set_Draw_Border(false);
		Add_Child(m_path_text_box);
	}
	else
	{
		m_path_text_box->Set_Box(Rect2D(0.0f, 0.0f, m_screen_box.Width * 0.5f, m_tool_bar_height));		
	}

	// Add the text box.
	if (m_text_box == NULL)
	{
		m_text_box = new UITextBox();
		m_text_box->Set_Scene(m_scene);
		m_text_box->Set_Parent(this);		
		m_text_box->Set_Box(Rect2D(0.0f, m_screen_box.Height - m_text_box_height, m_screen_box.Width, m_text_box_height));	
		m_text_box->Set_Game_Style(false);		
		Add_Child(m_text_box);
	}
	else
	{
		m_text_box->Set_Box(Rect2D(0.0f, m_screen_box.Height - m_text_box_height, m_screen_box.Width, m_text_box_height));		
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIFileBrowser::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIFileBrowser::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update toolbars.
	std::string name = m_text_box->Get_Value();
	bool can_create_folder = true;
	
	m_is_filename_valid = true;

	if (name == "" || 
		name.find_first_of("/\\:<>\"|\0\t\b") != std::string::npos) // Invalid path chars.
	{
		can_create_folder = false;
		m_is_filename_valid = false;
	}
	else
	{
		bool is_file = Platform::Get()->Is_File((m_directory + "/" + name).c_str());
		bool is_dir = Platform::Get()->Is_Directory((m_directory + "/" + name).c_str());

		if (is_file || is_dir)
		{
			can_create_folder = false;
		}

		if (is_dir)
		{
			m_is_filename_valid = false;
		}
	}

	m_create_folder_item->Set_Enabled(can_create_folder);

	// Toolbars clicked?
	if (m_create_folder_item->Was_Clicked() && can_create_folder)
	{
		std::string new_dir = m_directory + "/" + name;
		Platform::Get()->Create_Directory(new_dir.c_str(), false);	
		Set_Path(new_dir);
	}
	else if (m_level_up_item->Was_Clicked())
	{
		std::string dir = Platform::Get()->Extract_Directory(Platform::Get()->Extract_Directory(m_directory.c_str()));
		if (dir != m_directory && dir != "")
		{
			Set_Path(dir);
		}
	}

	// Path changed?
	if (m_path_text_box->Was_Entered())
	{
		Set_Path(m_path_text_box->Get_Value().c_str());
	}

	// Text changed?
	if (m_text_box->Was_Entered() && m_is_filename_valid)
	{
		DBG_LOG("Selecting file %s", m_text_box->Get_Value().c_str());
		scene->Dispatch_Event(manager, UIEvent(UIEventType::FileBrowser_Select, this));
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIFileBrowser::Draw_Item(const FrameTime& time, UIManager* manager, UIScene* scene, Rect2D item_box, std::string text, bool is_folder)
{
	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();

	Rect2D padded_box = item_box.Inflate(-m_item_padding, -m_item_padding);
	bool mouse_over = item_box.Intersects(mouse_pos);

	if (mouse_over == true)
	{
		m_select_background_frame.Draw_Frame(m_atlas_renderer, padded_box);

		if (mouse->Was_Button_Double_Clicked(InputBindings::Mouse_Left))
		{
			if (is_folder)
			{
				m_new_path = m_directory + "/" + text;
				DBG_LOG("Changing directory to %s", text.c_str());
			}
			else
			{
				DBG_LOG("Selecting file %s", text.c_str());
				m_text_box->Set_Value(text.c_str());	
				scene->Dispatch_Event(manager, UIEvent(UIEventType::FileBrowser_Select, this));
			}
		}
		else if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left))
		{
			if (!is_folder)
			{
				m_text_box->Set_Value(text.c_str());	
			}
		}
	}

	float icon_size = padded_box.Height - 6.0f;

	m_atlas_renderer.Draw_Frame(
		is_folder ? m_folder_icon : m_file_icon,
		Rect2D(padded_box.X + 2, padded_box.Y + 2 + ((padded_box.Height - 4) * 0.5f) - (icon_size * 0.5f), icon_size, icon_size),
		0.0f,
		Color::White);
			
	m_font_renderer.Draw_String(text.c_str(), Rect2D(padded_box.X + padded_box.Height + 4, padded_box.Y, padded_box.Width, padded_box.Height), 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
}

void UIFileBrowser::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Renderer* renderer			= Renderer::Get();
	RenderPipeline*	pipeline	= RenderPipeline::Get();

	// Draw background.
	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
	
	// Draw children.
 	UIElement::Draw(time, manager, scene);

	// Draw the items.
	Rect2D item_container_box = Rect2D(m_screen_box.X, m_screen_box.Y + m_tool_bar_height, m_screen_box.Width - m_slider_width, m_screen_box.Height - m_tool_bar_height - m_text_box_height);
	float offset = -Max(0, m_scroll_range - item_container_box.Height) * m_slider->Get_Progress();
	
	// Perform scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(item_container_box);

	m_new_path = "";

	for (std::vector<std::string>::iterator iter = m_directories.begin(); iter != m_directories.end(); iter++)
	{
		Rect2D item_box = Rect2D
		(
			item_container_box.X,
			item_container_box.Y + offset,
			item_container_box.Width,
			m_item_height
		);
		if (item_box.Y + item_box.Height >= item_container_box.Y && item_box.Y < item_container_box.Y + item_container_box.Height)
		{
			Draw_Item(time, manager, scene, item_box, *iter, true);
		}
		offset += m_item_height;
	}
	
	for (std::vector<std::string>::iterator iter = m_files.begin(); iter != m_files.end(); iter++)
	{
		Rect2D item_box = Rect2D
		(
			item_container_box.X,
			item_container_box.Y + offset,
			item_container_box.Width,
			m_item_height
		);
		if (item_box.Y + item_box.Height >= item_container_box.Y && item_box.Y < item_container_box.Y + item_container_box.Height)
		{
			Draw_Item(time, manager, scene, item_box, *iter, false);
		}
		offset += m_item_height;
	}
	
	if (m_new_path != "")
	{		
		Set_Path(m_new_path);
	}

	// Stop performing scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw the border.
	m_border_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
}
