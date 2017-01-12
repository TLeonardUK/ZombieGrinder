// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIComboBox.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

#include "Generic/Math/Math.h"

UIComboBox::UIComboBox()
	: m_frame_color(Color::White)
	, m_selected_item_index(0)
	, m_highleft_left_arrow_timer(0.0f)
	, m_highleft_right_arrow_timer(0.0f)
	, m_game_style(true)
	, m_use_markup(false)
{
}

UIComboBox::~UIComboBox()
{
}

bool UIComboBox::Focus_Left(UIManager* manager)
{
	return true;
}

bool UIComboBox::Focus_Right(UIManager* manager)
{
	return true;
}

void UIComboBox::Add_Item(UIComboBoxItem item)
{
	m_items.push_back(item);
}

void UIComboBox::Add_Item(const char* txt, void* meta)
{
	UIComboBoxItem item;
	item.Text = txt;
	item.MetaData1 = meta;

	Add_Item(item);
}

void UIComboBox::Clear_Items()
{
	m_items.clear();
	m_selected_item_index = 0;
}

const UIComboBoxItem& UIComboBox::Get_Selected_Item()
{
	return m_items.at(m_selected_item_index);
}

int UIComboBox::Get_Selected_Item_Index()
{
	return m_selected_item_index;
}

const std::vector<UIComboBoxItem>& UIComboBox::Get_Items()
{
	return m_items;
}

void UIComboBox::Set_Selected_Item_Index(int index)
{
	m_selected_item_index = Max(0, Min((int)m_items.size() - 1, index));
}

void UIComboBox::Select_Item_By_MetaData(void* data)
{
	int counter = 0;

	for (std::vector<UIComboBoxItem>::iterator iter = m_items.begin(); iter != m_items.end(); iter++)
	{
		UIComboBoxItem& combo = *iter;
		if (combo.MetaData1 == data)
		{
			m_selected_item_index = counter;
			return;
		}

		counter++;
	}
}

void UIComboBox::Refresh()
{
	UIManager* manager	= GameEngine::Get()->Get_UIManager();

	// Calculate screen-space box.
	m_screen_box			= Calculate_Screen_Box();
	
	m_atlas					= manager->Get_Atlas();
	m_atlas_renderer		= AtlasRenderer(m_atlas);
	m_font					= manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font, false, m_game_style);
	m_markup_font_renderer	= MarkupFontRenderer(m_font, false, m_game_style);
	
	// Grab all atlas frames.
	if (m_game_style == false)
	{
		m_background_frame			= UIFrame("toolbaritem_background_#");
		m_background_active_frame	= UIFrame("toolbaritem_background_hover_#");
		m_inactive_arrow_frame		= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_arrows_light_0");
		m_active_arrow_frame		= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_arrows_light_1");
	}
	else
	{
		m_background_frame			= UIFrame("screen_main_input_inactive_#");
		m_background_active_frame	= UIFrame("screen_main_input_active_#");
		m_inactive_arrow_frame		= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_arrows_0");
		m_active_arrow_frame		= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_arrows_1");
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
	Refresh_Boxes(manager);
}

void UIComboBox::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

bool UIComboBox::Is_Focusable()
{
	return true;
}

void UIComboBox::Refresh_Boxes(UIManager* manager)
{	
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float spacing	= 2.0f;

	float box_height   = m_screen_box.Height - ((spacing * 2) * ui_scale.Y);
	float aspect_scale = (box_height / m_active_arrow_frame->Rect.Height);

	m_left_arrow_box = Rect2D
	(
		floorf(m_screen_box.X + (spacing * ui_scale.X)),
		floorf(m_screen_box.Y + (spacing * ui_scale.Y)),
		floorf(m_active_arrow_frame->Rect.Width * aspect_scale),
		floorf(box_height)
	);

	m_right_arrow_box = Rect2D
	(
		floorf((m_screen_box.X + m_screen_box.Width) - (m_active_arrow_frame->Rect.Width * aspect_scale) - (spacing * ui_scale.X)),
		floorf(m_screen_box.Y + (spacing * ui_scale.Y)),
		floorf(m_active_arrow_frame->Rect.Width * aspect_scale),
		floorf(box_height)
	);
}

void UIComboBox::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	// Update boxes.
	Refresh_Boxes(manager);

	// Reduce arrow highlights.
	m_highleft_left_arrow_timer -= time.Get_Frame_Time();
	m_highleft_right_arrow_timer -= time.Get_Frame_Time();

	// Change selection.
	if (Is_Focused(manager) || !m_game_style)
	{
		if (manager->Was_Pressed(OutputBindings::GUI_Left) || 
			(mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && m_left_arrow_box.Intersects(mouse_position))
			)
		{
			if (m_items.size() > 0)
			{
				m_selected_item_index--;
				if (m_selected_item_index < 0)
				{
					m_selected_item_index = m_items.size() - 1;
				}
			}
			
			scene->Dispatch_Event(manager, UIEvent(UIEventType::ComboBox_SelectedIndexChanged, this));

			manager->Play_UI_Sound(UISoundType::Click);
			m_highleft_left_arrow_timer = ARROW_HIGHLIGHT_DURATION;
		}
		else if (manager->Was_Pressed(OutputBindings::GUI_Right) || 
				(mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && m_right_arrow_box.Intersects(mouse_position))
				)
		{
			if (m_items.size() > 0)
			{
				m_selected_item_index++;
				if (m_selected_item_index >= (int)m_items.size())
				{
					m_selected_item_index = 0;
				}
			}
			
			scene->Dispatch_Event(manager, UIEvent(UIEventType::ComboBox_SelectedIndexChanged, this));

			manager->Play_UI_Sound(UISoundType::Click);
			m_highleft_right_arrow_timer = ARROW_HIGHLIGHT_DURATION;
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIComboBox::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	
	if (m_game_style == false)
	{
		ui_scale = Vector2(1.0f, 1.0f);
	}

	// Draw background frame.
	if (Is_Focused(manager))
	{
		m_background_active_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
	}
	else
	{
		m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
	}

	// Draw selection text.
	if (m_items.size() > 0)
	{
		UIComboBoxItem& item = m_items.at(m_selected_item_index);
		if (m_use_markup)
		{
			m_markup_font_renderer.Draw_String(
				S(item.Text.c_str()), 
				m_screen_box, 
				16.0f,
				m_game_style == false ? Color::Black : Color::White, 
				TextAlignment::Center,
				TextAlignment::Center, 
				m_game_style == false ? ui_scale * 0.5f : ui_scale * 0.5f);	
		}
		else
		{
			m_font_renderer.Draw_String(
				S(item.Text.c_str()), 
				m_screen_box, 
				16.0f,
				m_game_style == false ? Color::Black : Color::White, 
				TextAlignment::Center,
				TextAlignment::Center, 
				m_game_style == false ? ui_scale * 0.5f : ui_scale * 0.5f);	
		}
	}

	// Draw arrows.
	if (m_highleft_left_arrow_timer >= 0)
	{
		m_atlas_renderer.Draw_Frame(m_active_arrow_frame, m_left_arrow_box, 0, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}
	else
	{
		m_atlas_renderer.Draw_Frame(m_inactive_arrow_frame, m_left_arrow_box, 0, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}

	if (m_highleft_right_arrow_timer >= 0)
	{
		m_atlas_renderer.Draw_Frame(m_active_arrow_frame, m_right_arrow_box, 0, Color::White, true, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}
	else
	{
		m_atlas_renderer.Draw_Frame(m_inactive_arrow_frame, m_right_arrow_box, 0, Color::White, true, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
