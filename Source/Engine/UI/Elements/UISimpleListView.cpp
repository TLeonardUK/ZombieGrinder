// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UISimpleListView.h"
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

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Generic/Math/Math.h"

UISimpleListView::UISimpleListView()
	: m_frame_color(Color::White)
{
}

UISimpleListView::~UISimpleListView()
{
}

void UISimpleListView::Add_Item(UISimpleListViewItem item)
{
	m_items.push_back(item);
}

void UISimpleListView::Add_Item(const char* text, void* data)
{
	UISimpleListViewItem item;
	item.MetaData = data;
	item.Values.push_back(text);

	Add_Item(item);
}

void UISimpleListView::Add_Item(std::vector<std::string> values, void* data)
{	
	UISimpleListViewItem item;
	item.MetaData = data;

	for (std::vector<std::string>::iterator iter = values.begin(); iter != values.end(); iter++)
	{
		item.Values.push_back(*iter);
	}

	Add_Item(item);
}

void UISimpleListView::Clear_Items()
{
	m_items.clear();
}

void UISimpleListView::Add_Column(UISimpleListViewColumn column)
{
	m_columns.push_back(column);
}

void UISimpleListView::Add_Column(const char* text, float width, bool limit_viewport, bool accept_markup)
{
	UISimpleListViewColumn column;
	column.Text = text;
	column.Width = width;
	column.LimitViewport = limit_viewport;
	column.AcceptMarkup = accept_markup;

	Add_Column(column);
}

void UISimpleListView::Clear_Columns()
{
	m_columns.clear();
}

const std::vector<UISimpleListViewItem>& UISimpleListView::Get_Items()
{
	return m_items;
}

const std::vector<UISimpleListViewColumn>& UISimpleListView::Get_Columns()
{
	return m_columns;
}

void UISimpleListView::Refresh()
{
	UIManager* manager	= GameEngine::Get()->Get_UIManager();

	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	m_atlas					= manager->Get_Atlas();
	m_atlas_renderer		= AtlasRenderer(m_atlas);
	m_font					= manager->Get_Font();
	m_font_renderer			= FontRenderer(m_font, false, false);
	m_markup_font_renderer	= MarkupFontRenderer(m_font, false, false);
	
	// Grab all atlas frames.
	m_background_frame			= UIFrame("screen_main_box_active_#");

	// Get children to calculate their screen box.
	UIElement::Refresh();
	RefreshBoxes(manager);

	UIElement::Refresh();
}

void UISimpleListView::RefreshBoxes(UIManager* manager)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float padding				= 0;
	float header_size			= 16.0f;
	float seperator_size		= 1.0f;
	float slider_size			= 8.0f;
	float item_height			= 10.0f;
	float item_header_padding	= 2.0f;
	float slider_padding		= 8.0f;

	m_padded_box = Rect2D
	(
		m_screen_box.X + (padding * ui_scale.X),
		m_screen_box.Y + (padding * ui_scale.Y),
		m_screen_box.Width - ((padding * 2.0f) * ui_scale.X),
		m_screen_box.Height - ((padding * 2.0f) * ui_scale.Y)
	);

	m_header_box = Rect2D
	(
		m_padded_box.X,
		m_padded_box.Y,
		m_padded_box.Width,
		header_size * ui_scale.Y
	);

	m_item_box = Rect2D
	(
		m_padded_box.X,
		m_padded_box.Y + (m_header_box.Height) + (item_header_padding * ui_scale.Y),
		m_padded_box.Width,
		m_padded_box.Height - (m_header_box.Height) - (item_header_padding * ui_scale.Y)
	);
}

void UISimpleListView::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

bool UISimpleListView::Is_Focusable()
{
	return false;
}

void UISimpleListView::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	// Update boxes.
	RefreshBoxes(manager);

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UISimpleListView::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Renderer* renderer			= Renderer::Get();
	RenderPipeline*	pipeline	= RenderPipeline::Get();
	
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float seperator_size		 = 1.0f;
	float slider_size			 = 8.0f;
	float item_height			 = 12.0f;
	float item_header_padding	 = 3.0f;
	float item_subheader_padding = 2.0f;
	float text_padding			 = 2.0f;
	float offset				 = 0.0f;

	PrimitiveRenderer p;
	
	// Draw background.
	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
	
	// Draw the column headers.
	for (std::vector<UISimpleListViewColumn>::iterator iter = m_columns.begin(); iter != m_columns.end(); iter++)
	{
		UISimpleListViewColumn& column = *iter;

		Vector2 position = Vector2
		(
			m_header_box.X + (text_padding * ui_scale.X) + (m_header_box.Width * offset),
			m_header_box.Y + (item_header_padding * ui_scale.Y) 
		);

		m_font_renderer.Draw_String(column.Text.c_str(), position, 16.0f * 0.5f, Color(180, 180, 180, 255), Vector2(0.0f, 0.0f), ui_scale);
		offset += column.Width;
	}
	
	// Calculate offset.
	float all_items_height = (item_height * ui_scale.Y) * m_items.size();
	float item_range	   = all_items_height - m_item_box.Height;

	// Perform scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(m_item_box);

	int counter = 0;
	
	//p.Draw_Solid_Quad(m_item_box, Color(255, 255, 255, 255));
	//p.Draw_Solid_Quad(m_header_box, Color(255, 0, 0, 255));

	// Draw all the items.
	float item_offset = 0.0f;
	bool use_alt_color = true;
	for (std::vector<UISimpleListViewItem>::iterator iter = m_items.begin(); iter != m_items.end(); iter++)
	{
		UISimpleListViewItem& item = *iter;
		int item_index = iter - m_items.begin();

		int columns = Min(m_columns.size(), item.Values.size());
		
		// If there is no value in the last column, we assume a header.
		std::string& last_value = item.Values.at(columns - 1);
		if (last_value == "")
		{
			use_alt_color = false;
			if (item_offset != 0)
			{
				item_offset += item_subheader_padding * ui_scale.Y;
			}
		}

		Rect2D item_rect = Rect2D
		(
			m_item_box.X,
			m_item_box.Y + (item_offset),
			m_item_box.Width,
			(item_height * ui_scale.Y)
		);

		// On-Screen?
		if (item_rect.Y <= m_screen_box.Y + m_screen_box.Height &&
			item_rect.Y + (item_height * ui_scale.Y) >= m_screen_box.Y)
		{
			counter++;

			pipeline->Set_Render_Batch_Scissor_Rectangle(Rect2D(item_rect.X, m_item_box.Y, m_item_box.Width, m_item_box.Height));

			// If this is the selected-item draw a background.
			if (use_alt_color == true)
			{
				p.Draw_Solid_Quad(item_rect, Color(31, 38, 48, 128));
			}
			else
			{
				//p.Draw_Solid_Quad(item_rect, Color(57, 74, 98, 128));
			}

			// Draw each string.
			offset = 0.0f;
			for (int i = 0; i < columns; i++)
			{
				UISimpleListViewColumn& column = m_columns.at(i);
				std::string& value = item.Values.at(i);
			
				Vector2 position = Vector2
				(
					item_rect.X + (text_padding * ui_scale.X) + (m_header_box.Width * offset),
					item_rect.Y
				);
	
				// Limit the viewport if neccessary.
				if (column.LimitViewport == true)
				{
					pipeline->Set_Render_Batch_Scissor_Rectangle(Rect2D(item_rect.X + (m_header_box.Width * offset) + (text_padding * ui_scale.X) , m_item_box.Y, (m_header_box.Width * column.Width) - ((text_padding * ui_scale.X) * 2.0f), m_item_box.Height));
				}
				else
				{
					pipeline->Set_Render_Batch_Scissor_Rectangle(Rect2D(item_rect.X, m_item_box.Y, m_item_box.Width, m_item_box.Height));
				}

				if (column.AcceptMarkup == true)
				{
					m_markup_font_renderer.Draw_String(value.c_str(), position, 16.0f * 0.5f, Color::White, Vector2(0.0f, 0.0f), ui_scale);
				}
				else
				{
					m_font_renderer.Draw_String(value.c_str(), position, 16.0f * 0.5f, Color::White, Vector2(0.0f, 0.0f), ui_scale);
				}

				offset += column.Width; 
			}
		}

		use_alt_color = !use_alt_color;

		item_offset += (item_height * ui_scale.Y);
	}
	
	// Stop performing scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(false);
	
	// Draw children.
	UIElement::Draw(time, manager, scene);
}
