// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIListView.h"
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

UIListView::UIListView()
	: m_frame_color(Color::White)
	, m_selected_item_index(0)
	, m_slider(NULL)
	, m_hover_item_index(-1)
	, m_selection_just_changed(false)
	, m_last_item_count(0)
	, m_no_header(false)
	, m_selected_item_rect(0, 0, 0,0)
	, m_item_height(12.0f)
	, m_item_spacing(0.0f)
	, m_fake_item_count(-1)
	, m_simplified(false)
	, m_center_selection(false)
{
}

UIListView::~UIListView()
{
}

UISlider* UIListView::Get_Slider()
{
	return m_slider;
}

float UIListView::Get_Item_Height()
{
	return m_item_height;
}

void UIListView::Set_Item_Height(float val)
{
	m_item_height = val;
}

Vector2 UIListView::Get_Focus_Point()
{
	return m_focus_point;
}

bool UIListView::Focus_Up(UIManager* manager)
{
	if (m_selected_item_index > 0)
	{
		manager->Play_UI_Sound(UISoundType::Click);
		m_selected_item_index--;
		m_selection_just_changed = true;
		return true;
	}
	return false;
}

bool UIListView::Focus_Down(UIManager* manager)
{
	if (m_selected_item_index < Get_Item_Count() - 1)
	{
		manager->Play_UI_Sound(UISoundType::Click);
		m_selected_item_index++;
		m_selection_just_changed = true;
		return true;
	}
	return false;
}

void UIListView::Add_Item(UIListViewItem item)
{
	m_items.push_back(item);
}

void UIListView::Add_Item(const char* text, void* data)
{
	UIListViewItem item;
	item.MetaData = data;
	item.Values.push_back(text);

	Add_Item(item);
}

void UIListView::Add_Item(std::vector<std::string> values, void* data)
{	
	UIListViewItem item;
	item.MetaData = data;

	for (std::vector<std::string>::iterator iter = values.begin(); iter != values.end(); iter++)
	{
		item.Values.push_back(*iter);
	}

	Add_Item(item);
}

void UIListView::Clear_Items()
{
	m_items.clear();
}

void UIListView::Add_Column(UIListViewColumn column)
{
	m_columns.push_back(column);
}

void UIListView::Add_Column(const char* text, float width, bool limit_viewport, bool accept_markup)
{
	UIListViewColumn column;
	column.Text = text;
	column.Width = width;
	column.LimitViewport = limit_viewport;
	column.AcceptMarkup = accept_markup;

	Add_Column(column);
}

void UIListView::Clear_Columns()
{
	m_columns.clear();
}

const UIListViewItem& UIListView::Get_Selected_Item()
{
	return m_items.at(m_selected_item_index);
}

int UIListView::Get_Selected_Item_Index()
{
	return m_selected_item_index;
}

void UIListView::Set_Selected_Item_Index(int index)
{
	m_selected_item_index = Max(0, Min((int)Get_Item_Count() - 1, index));
	m_selection_just_changed = true;
}

Rect2D UIListView::Get_Selected_Item_Rectangle()
{
	return m_selected_item_rect;
}

void UIListView::Set_Faked_Item_Count(int count)
{
	m_fake_item_count = count;
}

bool UIListView::Is_Scrolling()
{
	return m_slider->Is_Moving();
}

int UIListView::Get_Item_Count()
{
	if (m_fake_item_count >= 0)
	{
		return m_fake_item_count;
	}
	else
	{
		return m_items.size();
	}
}

int UIListView::Get_Scroll_Offset()
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	Vector2 ui_scale = Get_Draw_Scale(manager);

	float progress = m_slider->Get_Progress();

	float item_height = (m_item_height + m_item_spacing) * ui_scale.Y;
	float items_in_view = (m_item_box.Height / item_height);
	float item_range  = m_fake_item_count - items_in_view;

	return (int)floorf(item_range * progress);
}

void UIListView::Scroll_To(int offset)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	Vector2 ui_scale = Get_Draw_Scale(manager);

	float item_height = (m_item_height + m_item_spacing) * ui_scale.Y;
	float items_in_view = (m_item_box.Height / item_height);
	float item_range  = m_fake_item_count - items_in_view;

	//DBG_LOG("Scroll_To %i", offset);

	m_slider->Set_Progress(Max(0, offset - items_in_view) / item_range);
	m_slider->Cancel_Movement();
	m_selection_just_changed = true;
}

const std::vector<UIListViewItem>& UIListView::Get_Items()
{
	return m_items;
}

std::vector<UIListViewColumn>& UIListView::Get_Columns()
{
	return m_columns;
}

void UIListView::Refresh()
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

	// Initial focal point is center of element.
	m_focus_point = Vector2(m_screen_box.X - 10.0f, m_screen_box.Y + (m_screen_box.Height * 0.5f));

	// Add the slider.
	if (m_slider == NULL)
	{
		m_slider = new UISlider();
		m_slider->Set_Scene(m_scene);
		m_slider->Set_Parent(this);
		m_slider->Set_Box(Rect2D(m_slider_box.X - m_screen_box.X, m_slider_box.Y - m_screen_box.Y, m_slider_box.Width, m_slider_box.Height));
		m_slider->Set_Direction(UISliderDirection::Vertical);
		m_slider->Set_Focusable(false);
		Add_Child(m_slider);
	}

	UIElement::Refresh();
}

void UIListView::RefreshBoxes(UIManager* manager)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float padding				= 4;
	float header_size			= m_item_height + 4.0f;//16.0f;
	float seperator_size		= 4.0f;
	float slider_size			= 8.0f;
	float item_header_padding	= 2.0f;
	float slider_padding		= 8.0f;
	float header_box_y_offset	= 0.0f;

	if (m_no_header == true)
	{
		//seperator_size = 0.0f;
		header_size = 0.0f;
		item_header_padding = 0.0f;
	}
	if (m_simplified == true)
	{
		item_header_padding = 0.0f;
		slider_size = 0.0f;
		header_size = 10.0f;
		header_box_y_offset = 6.0f;
	}

	m_padded_box = Rect2D
	(
		m_screen_box.X + (padding * ui_scale.X),
		m_screen_box.Y + (padding * ui_scale.Y),
		m_screen_box.Width - ((padding * 2.0f) * ui_scale.X),
		m_screen_box.Height - ((padding * 2.0f) * ui_scale.Y)
	);
	
	m_slider_box = Rect2D
	(
		m_padded_box.X + m_padded_box.Width - (slider_size * ui_scale.X),
		m_padded_box.Y + (header_size * ui_scale.Y) + (item_header_padding * ui_scale.Y) + (slider_padding * ui_scale.Y),
		(slider_size * ui_scale.X),
		m_padded_box.Height - (header_size * ui_scale.Y) - ((slider_padding * 2.0f) * ui_scale.Y)
	);

	m_header_box = Rect2D
	(
		m_padded_box.X,
		m_padded_box.Y - header_box_y_offset,
		m_padded_box.Width - m_slider_box.Width - (seperator_size * ui_scale.X),
		header_size * ui_scale.Y
	);

	m_item_box = Rect2D
	(
		m_padded_box.X,
		m_padded_box.Y + (m_header_box.Height) + (item_header_padding * ui_scale.Y),
		m_padded_box.Width - m_slider_box.Width - (seperator_size * ui_scale.X),
		m_padded_box.Height - (m_header_box.Height) - (item_header_padding * ui_scale.Y)
	);

	// Update slider box.	
	if (m_slider != NULL)
	{
		m_slider->Set_Box(Rect2D(m_slider_box.X - m_screen_box.X, m_slider_box.Y - m_screen_box.Y, m_slider_box.Width, m_slider_box.Height));
		if (m_simplified)
		{
			m_slider->Set_Visible(false);
		}
	}
}

void UIListView::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

bool UIListView::Is_Focusable()
{
	return true;
}

void UIListView::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	bool focusable = manager->Is_Element_Focusable(this);
	
	Vector2 ui_scale = Get_Draw_Scale(manager);

	int item_count = Get_Item_Count();

	float item_height_plus_spacing = ((m_item_height + m_item_spacing) * ui_scale.Y);

	// Adjust slider if we have added items.
	// TODO: Clean this shit up, pretty sure there is a nicer way to do this lol.
	if (m_last_item_count != item_count)
	{	
		float new_all_items_height = ((m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y)) * item_count;
		float new_item_range	   = new_all_items_height - m_item_box.Height;
		float new_item_offset	   = -(new_item_range * m_slider->Get_Progress());

		float old_all_items_height = ((m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y)) * m_last_item_count;
		float old_item_range	   = old_all_items_height - m_item_box.Height;
		float old_item_offset	   = -(old_item_range * m_slider->Get_Progress());

		if (new_item_offset != 0)
		{
			float p = -old_item_offset / -new_item_offset;

			if (m_center_selection)
			{
				p = ((int)(p / item_height_plus_spacing)) * item_height_plus_spacing;
			}

		//	DBG_LOG("Scroll to item count change %i / %i", old_item_offset, new_item_offset);
			m_slider->Set_Progress(m_slider->Get_Progress() * p);
		}

		m_selection_just_changed = true;
	}
	m_last_item_count = item_count;

	// Calculate offset.
	float all_items_height = ((m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y)) * item_count;
	float item_range	   = Max(0, all_items_height - m_item_box.Height);

	// Make sure select index is valid.
	if (m_selected_item_index < 0)
		m_selected_item_index = 0;
	if (m_selected_item_index >= (int)item_count)
		m_selected_item_index = item_count - 1;

	// Reset hover index.
	m_hover_item_index = -1;

	// Look for selected items.
	float item_offset = -(item_range * m_slider->Get_Progress());
	float single_item_offset = item_count / item_range;
	Rect2D selected_item_rect;
	bool update_selected_item_rect = false;
	bool selected_item_just_changed = false;
	bool auto_select = false;

	for (int item_index = 0; item_index < Get_Item_Count(); item_index++)
	{
		Rect2D item_rect = Rect2D
		(
			m_item_box.X,
			m_item_box.Y + (item_offset),
			m_item_box.Width,
			(m_item_height * ui_scale.Y)
		);

		// Selected by cursor?
		if (item_rect.Intersects(mouse_position) && focusable == true)
		{
			m_hover_item_index = item_index;

			if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && 
				m_selected_item_index != item_index)
			{
				m_selected_item_index = item_index;
				m_selection_just_changed = true;
				selected_item_just_changed = true;
				m_selected_item_rect = item_rect;
				auto_select = true;
			}
		}

		// It this the selection? Should we shift selection into screen?
		if (m_selection_just_changed == true && item_index == m_selected_item_index)
		{
			float progress     = m_slider->Get_Progress();
			float shift_offset = item_rect.Y + (item_rect.Height * 0.5f);
 			float pixel_offset = 0.0f;

			float draw_height  = item_rect.Height;

			if (m_center_selection)
			{
				if (item_rect.Y < m_item_box.Y || 
					item_rect.Y + item_rect.Height > m_item_box.Y + m_item_box.Height)
				{
					pixel_offset = (m_selected_item_index * item_height_plus_spacing) - (draw_height * 0.5f);
					pixel_offset = ((int)(pixel_offset / item_height_plus_spacing)) * item_height_plus_spacing;
					//DBG_LOG("Shifting %i to center offset %f/%f", m_selected_item_index, pixel_offset, item_range);

					m_slider->Set_Progress(pixel_offset / item_range);
				}
			}
			else
			{
				if (item_rect.Y < m_item_box.Y)
				{
					pixel_offset = m_selected_item_index * item_height_plus_spacing;
					m_slider->Set_Progress(pixel_offset / item_range);
					//DBG_LOG("Shifting %i to screen a", m_selected_item_index);
				}
				else if (item_rect.Y + item_rect.Height > m_item_box.Y + m_item_box.Height)
				{
					pixel_offset = ((m_selected_item_index * item_height_plus_spacing) + item_height_plus_spacing) - (m_item_box.Height);
					m_slider->Set_Progress(pixel_offset / item_range);
					//DBG_LOG("Shifting %i to screen b", m_selected_item_index);
				}
			}
		}
		
		// Update focal point.
		if (m_selection_just_changed == false && item_index == m_selected_item_index)
		{
			m_focus_point = Vector2(m_screen_box.X - 10.0f, item_rect.Y + (item_rect.Height * 0.5f));
			selected_item_rect = item_rect;
			update_selected_item_rect = true;
		}
		
		item_offset += (m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y);
	}

	if (update_selected_item_rect == true)
	{
		// Selection above first on screen? Move selection down.
		while (selected_item_rect.Y + (selected_item_rect.Height * 0.5f) < m_item_box.Y)
		{
			selected_item_rect.Y += (m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y);
			m_selected_item_index = Min((int)item_count - 1, m_selected_item_index + 1);
		}

		// Selection below first on screen? Move selection up.
		while (selected_item_rect.Y + (selected_item_rect.Height * 0.5f) > m_item_box.Y + m_item_box.Height)
		{
			selected_item_rect.Y -= (m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y);
			m_selected_item_index = Max(0, m_selected_item_index - 1);
		}
	}

	// Reset selection.
	m_selection_just_changed = false;

	// Update boxes.
	RefreshBoxes(manager);

	// Click item?
	if (Is_Focused(manager) && update_selected_item_rect == true)
	{
		if (manager->Was_Pressed(OutputBindings::GUI_Select) ||
			auto_select ||
			(
				mouse->Was_Button_Clicked(InputBindings::Mouse_Left) &&
				selected_item_rect.Intersects(mouse_position) /*&&
				selected_item_just_changed == false*/
			))
		{
			scene->Dispatch_Event(manager, UIEvent(UIEventType::ListView_Item_Click, this));
			//DBG_LOG("Dispatched Item Click");
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIListView::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Renderer* renderer			= Renderer::Get();
	RenderPipeline*	pipeline	= RenderPipeline::Get();
	
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float seperator_size		= 1.0f;
	float slider_size			= 8.0f;
	float header_size = m_item_height + 4.0f;//16.0f;
	float item_header_padding	= 2.0f;
	float text_padding			= 2.0f;
	float offset				= 0.0f;
	
	int item_count = Get_Item_Count();

	PrimitiveRenderer p;

	// Draw background.
	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_frame_color);
	
	if (m_no_header == false)
	{
		// Draw the column headers.
		for (std::vector<UIListViewColumn>::iterator iter = m_columns.begin(); iter != m_columns.end(); iter++)
		{
			UIListViewColumn& column = *iter;

			Vector2 position = Vector2
			(
				m_header_box.X + (m_header_box.Width * offset),
				m_header_box.Y
			);

			m_font_renderer.Draw_String(column.Text.c_str(), position, header_size * 0.5f, Color(180, 180, 180, 255), Vector2(0.0f, 0.0f), ui_scale);
			offset += column.Width;
		}

		if (!m_simplified)
		{
			// Draw the header seperator.	
			p.Draw_Solid_Quad(Rect2D(m_header_box.X, m_header_box.Y + m_header_box.Height - (seperator_size * ui_scale.Y), m_header_box.Width, seperator_size * ui_scale.Y), Color(180, 180, 180, 255));
		}
	}

	// Calculate offset.
	float all_items_height = ((m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y)) * item_count;
	float item_range	   = Max(0, all_items_height - m_item_box.Height);
	
	// Perform scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(m_item_box);

	int counter = 0;

	// Draw all the items.
	float item_offset = -(item_range * m_slider->Get_Progress());
	for (int item_index = 0; item_index < Get_Item_Count(); item_index++)
	{
		Rect2D item_rect = Rect2D
		(
			m_item_box.X,
			m_item_box.Y + (item_offset),
			m_item_box.Width,
			(m_item_height * ui_scale.Y)
		);

		// Store selected item rectangle (used for making popup menus and shit).
		if (m_selected_item_index == item_index)
		{
			// Do screen-space, not render-space (eg. without adjustment for aspect ratio)
			m_selected_item_rect = Rect2D
			(
				m_item_box.X,
				m_item_box.Y + (item_offset),
				m_item_box.Width,
				m_item_height * ui_scale.Y
			);
		}
		
		// On-Screen?
		if (item_rect.Y <= m_screen_box.Y + m_screen_box.Height &&
			item_rect.Y + (m_item_height * ui_scale.Y) >= m_screen_box.Y)
		{
			counter++;

			pipeline->Set_Render_Batch_Scissor_Rectangle(Rect2D(item_rect.X, m_item_box.Y, m_item_box.Width, m_item_box.Height));

			if (On_Draw_Item.Registered())
			{
				UIListViewItem& item = m_items.at(item_index);

				UIListViewDrawItemData data;
				data.view			= this;
				data.view_bounds	= m_screen_box;
				data.item_bounds	= item_rect;
				data.time			= time;
				data.manager		= manager;
				data.scene			= scene;
				data.item			= &item;
				data.ui_scale		= ui_scale;
				data.selected		= (m_selected_item_index == item_index);
				data.hovering		= (m_hover_item_index == item_index);
				data.item_index		= item_index;

				On_Draw_Item.Fire(&data);
			}
			else
			{
				UIListViewItem item;

				if (m_fake_item_count >= 0)
				{
					if (On_Get_Item_Data.Registered())
					{
						UIListViewGetItemData data;
						data.view = this;
						data.item = &item;
						data.index = item_index;

						On_Get_Item_Data.Fire(&data);
					}
				}
				else
				{
					item = m_items.at(item_index);
				}

				int columns = Min(m_columns.size(), item.Values.size());
		
				// If this is the selected-item draw a background.
				if (m_selected_item_index == item_index)
				{
					if (Is_Focused(manager))
					{
						p.Draw_Solid_Quad(item_rect, Color(57, 74, 98, 255));
					}
					else
					{
						p.Draw_Solid_Quad(item_rect, Color(57, 74, 98, 128));
					}
				}
				else if (m_hover_item_index == item_index)
				{
					p.Draw_Solid_Quad(item_rect, Color(57, 74, 98, 64));
				}

			//	DBG_LOG("This:0x%08x Pipeline:0x%08x", this, pipeline);

				// Draw each string.
				offset = 0.0f;
				for (int i = 0; i < columns; i++)
				{
				//	DBG_LOG("i=%i columns=%i m_columns.size()=%i", i, columns, m_columns.size());
					UIListViewColumn column = m_columns.at(i);
					std::string value = item.Values.at(i);
					//DBG_LOG("VALUE=%s", value.c_str());
			
					Vector2 position = Vector2
					(
						item_rect.X + (text_padding * ui_scale.X) + (m_header_box.Width * offset),
						item_rect.Y - (1.0f * ui_scale.Y)
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
						m_markup_font_renderer.Draw_String(value.c_str(), position, header_size * 0.5f, Color::White, Vector2(0.0f, 0.0f), ui_scale);
					}
					else
					{
						m_font_renderer.Draw_String(value.c_str(), position, header_size * 0.5f, Color::White, Vector2(0.0f, 0.0f), ui_scale);
					}

					offset += column.Width; 
				}
			}
		}

		item_offset += (m_item_height * ui_scale.Y) + (m_item_spacing * ui_scale.Y);
	}

	// Stop performing scissor testing.
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
