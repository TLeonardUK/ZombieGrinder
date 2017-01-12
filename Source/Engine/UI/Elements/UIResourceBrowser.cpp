// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIResourceBrowser.h"
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

#include "Engine/Audio/Sounds/Sound.h"
#include "Engine/Audio/Sounds/SoundHandle.h"
#include "Engine/Audio/Sounds/SoundChannel.h"
#include "Engine/Resources/Compiled/Audio/CompiledAudio.h"

UIResourceBrowser::UIResourceBrowser()
	: m_tool_bar_height(30)
	, m_slider(NULL)
	, m_slider_width(10.0f)
	, m_filter_text_box(NULL)
	, m_scroll_range(0)
	, m_item_height(25.0f)
	, m_item_padding(4.0f)
	, m_tool_bar(NULL)
	, m_mode(UIResourceBrowserMode::Image)
	, m_filter("")
	, m_sfx_channel(NULL)
	, m_selected_resource("")
	, m_anim_timer(0.0f)
{
}

UIResourceBrowser::~UIResourceBrowser()
{
	if (m_sfx_channel != NULL)
	{
		m_sfx_channel->Pause();
		SAFE_DELETE(m_sfx_channel);
	}
}

std::string UIResourceBrowser::Get_Selected_Item()
{
	return m_selected_resource;
}

void UIResourceBrowser::Refresh()
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
		m_slider->Set_Box(Rect2D(m_screen_box.Width - m_slider_width, m_tool_bar_height, m_slider_width, m_screen_box.Height - m_tool_bar_height));		
		m_slider->Set_Direction(UISliderDirection::Vertical);
		m_slider->Set_Focusable(false);
		m_slider->Set_Progress(1.0f);
		m_slider->Set_Game_Style(false);
		Add_Child(m_slider);
	}
	else
	{
		m_slider->Set_Box(Rect2D(m_screen_box.Width - m_slider_width, m_tool_bar_height, m_slider_width, m_screen_box.Height - m_tool_bar_height));		
	}

	// Add the path text box.
	if (m_filter_text_box == NULL)
	{
		m_filter_text_box = new UITextBox();
		m_filter_text_box->Set_Scene(m_scene);
		m_filter_text_box->Set_Parent(this);		
		m_filter_text_box->Set_Box(Rect2D(0.0f, 0.0f, m_screen_box.Width, m_tool_bar_height));	
		m_filter_text_box->Set_Game_Style(false);		
		m_filter_text_box->Set_Draw_Border(false);
		Add_Child(m_filter_text_box);
	}
	else
	{
		m_filter_text_box->Set_Box(Rect2D(0.0f, 0.0f, m_screen_box.Width * 0.5f, m_tool_bar_height));		
	}

	// Set default state.
	Set_Filter(m_filter);
	//Set_Mode(UIResourceBrowserMode::Image);

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIResourceBrowser::Set_Filter(std::string filter)
{
	std::string old_filter = m_filter;
	m_filter = filter;

	std::vector<std::string> resources = m_items;

	// If not a sub-query, we need to do it on all resources.
	if (filter.size() < old_filter.size() ||
		filter.substr(0, old_filter.size()) != old_filter ||
		(old_filter == "" || filter == ""))
	{
		resources = m_resources;
	}

	// Apply filter.
	m_items.clear();
	m_items.reserve(resources.size());

	for (std::vector<std::string>::iterator iter = resources.begin(); iter != resources.end(); iter++)
	{
		std::string& name = *iter;
		if (m_filter == "" || name.find(m_filter.c_str()) != std::string::npos)//StringHelper::Match(name, m_filter))
		{
			m_items.push_back(name);
		}
	}

	DBG_LOG("Changing resource filter from '%s' to '%s' (%i items).", old_filter.c_str(), filter.c_str(), m_items.size());

	// Scroll update.
	m_scroll_range = m_items.size() * m_item_height;
	m_slider->Set_Progress(0.0f);
	m_selected_resource = "";

	if (m_filter != m_filter_text_box->Get_Value())
		m_filter_text_box->Set_Value(m_filter);
}

void UIResourceBrowser::Set_Mode(UIResourceBrowserMode::Type mode)
{
	ResourceFactory* res_factory = ResourceFactory::Get();

	m_mode = mode;

	m_resources.clear();

	switch (m_mode)
	{
	case UIResourceBrowserMode::Image:
		{
			DBG_LOG("Resource browser mode: Image");
			ThreadSafeHashTable<AtlasFrame*, unsigned int>& frames = res_factory->Get_Atlas_Frames();
			for (ThreadSafeHashTable<AtlasFrame*, unsigned int>::Iterator iter = frames.Begin(); iter != frames.End(); iter++)
			{
				m_resources.push_back((*iter)->Name);
			}
			m_item_height = 48.0f;
			break;
		}
	case UIResourceBrowserMode::Audio:
		{
			DBG_LOG("Resource browser mode: Audio");
			ThreadSafeHashTable<SoundHandle*, unsigned int>& frames = res_factory->Get_Sounds();
			for (ThreadSafeHashTable<SoundHandle*, unsigned int>::Iterator iter = frames.Begin(); iter != frames.End(); iter++)
			{
				m_resources.push_back((*iter)->Get()->Get_Name());
			}
			m_item_height = 25.0f;
			break;
		}
	case UIResourceBrowserMode::Animation:
		{
			DBG_LOG("Resource browser mode: Animation");
			ThreadSafeHashTable<AtlasAnimation*, unsigned int>& frames = res_factory->Get_Atlas_Animations();
			for (ThreadSafeHashTable<AtlasAnimation*, unsigned int>::Iterator iter = frames.Begin(); iter != frames.End(); iter++)
			{
				m_resources.push_back((*iter)->Name);
			}
			m_item_height = 48.0f;
			break;
		}
	}
}

void UIResourceBrowser::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIResourceBrowser::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Filter changed?
	std::string filter = m_filter_text_box->Get_Value();
	if (filter != m_filter)
	{
		DBG_LOG("Changed filter to %s.", filter.c_str());
		Set_Filter(filter);
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIResourceBrowser::Draw_Item(const FrameTime& time, UIManager* manager, UIScene* scene, Rect2D item_box, std::string text)
{
	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();

	Rect2D padded_box = item_box.Inflate(-m_item_padding, -m_item_padding);
	bool mouse_over = item_box.Intersects(mouse_pos) && m_screen_box.Intersects(mouse_pos);

	if (mouse_over == true || m_selected_resource == text)
	{
		m_select_background_frame.Draw_Frame(m_atlas_renderer, padded_box);
	}

	if (mouse_over == true)
	{
		if (mouse->Was_Button_Double_Clicked(InputBindings::Mouse_Left))
		{
			m_selected_resource = text;
			scene->Dispatch_Event(manager, UIEvent(UIEventType::ResourceBrowser_Select, this));
		}
		else if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left))
		{	
			switch (m_mode)
			{
			case UIResourceBrowserMode::Audio:
				{
					SoundHandle* sfx = ResourceFactory::Get()->Get_Sound(text.c_str());

					if (m_sfx_channel != NULL)
					{
						DBG_LOG("Stopping old preview audio.");
						m_sfx_channel->Pause();
						SAFE_DELETE(m_sfx_channel);
					}

					if (sfx != NULL)	
					{
						DBG_LOG("Previewing audio: %s", text.c_str());
						m_sfx_channel = sfx->Get()->Allocate_Channel();
						sfx->Get()->Play(m_sfx_channel, false);
					}

					break;
				}
			}

			m_selected_resource = text;
		}
	}

	float icon_size = padded_box.Height - 6.0f;

	AtlasFrame* frame = NULL;

	if (m_mode == UIResourceBrowserMode::Image)
	{
		frame = ResourceFactory::Get()->Get_Atlas_Frame(text.c_str());
	}
	else if (m_mode == UIResourceBrowserMode::Animation)
	{
		AtlasAnimation* anim = ResourceFactory::Get()->Get_Atlas_Animation(text.c_str());
		if (anim != NULL)
		{
			int idx = (int)(m_anim_timer / anim->Speed) % anim->Frames.size();
			frame = anim->Frames.at(idx);
		}
	}

	if (frame == NULL)
		frame = m_file_icon;

	Rect2D frame_box = Rect2D(padded_box.X + 2, padded_box.Y + 2 + ((padded_box.Height - 4) * 0.5f) - (icon_size * 0.5f), icon_size, icon_size);
	Vector2 frame_center = frame_box.Center();

	if (frame->Rect.Width > frame->Rect.Height)
	{
		float aspect = (frame->Rect.Height / frame->Rect.Width);
		float width = frame_box.Width;
		float height = frame_box.Height * aspect;
		frame_box = Rect2D(frame_center.X - (width*0.5f), frame_center.Y - (height*0.5f), width, height);
	}
	else if (frame->Rect.Width < frame->Rect.Height)
	{
		float aspect = (frame->Rect.Width / frame->Rect.Height);
		float width = frame_box.Width * aspect;
		float height = frame_box.Height;
		frame_box = Rect2D(frame_center.X - (width*0.5f), frame_center.Y - (height*0.5f), width, height);
	}

	// Offset by origin so its centered.
	float scale_x = frame_box.Width / frame->Rect.Width;
	float scale_y = frame_box.Height / frame->Rect.Height;
	frame_box.X += frame->Origin.X;
	frame_box.Y += frame->Origin.Y;

	//	frame_box.X += (frame->Origin.X * (scale_x));
//	frame_box.Y += (frame->Origin.Y * (scale_y));

	m_atlas_renderer.Draw_Frame(
		frame,
		frame_box,
		0.0f,
		Color::White);

	m_font_renderer.Draw_String(text.c_str(), Rect2D(padded_box.X + padded_box.Height + 4, padded_box.Y, padded_box.Width, padded_box.Height), 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
}

void UIResourceBrowser::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Renderer* renderer			= Renderer::Get();
	RenderPipeline*	pipeline	= RenderPipeline::Get();

	m_anim_timer += time.Get_Delta_Seconds() * 1000;

	// Draw background.
	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box);

	// Draw children.
	UIElement::Draw(time, manager, scene);

	// Draw the items.
	Rect2D item_container_box = Rect2D(m_screen_box.X, m_screen_box.Y + m_tool_bar_height, m_screen_box.Width - m_slider_width, m_screen_box.Height - m_tool_bar_height);
	float offset = -Max(0, m_scroll_range - item_container_box.Height) * m_slider->Get_Progress();

	// Perform scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(item_container_box);

	for (std::vector<std::string>::iterator iter = m_items.begin(); iter != m_items.end(); iter++)
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
			Draw_Item(time, manager, scene, item_box, *iter);
		}
		offset += m_item_height;
	}

	// Stop performing scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw the border.
	m_border_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
}
