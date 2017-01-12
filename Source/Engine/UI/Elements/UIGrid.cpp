// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIGrid.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIEvent.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Localise/Locale.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

static const float PAGE_ARROW_OFFSET_MAX = -0.25f;
static const int   PAGE_ARROW_OFFSET_INTERVAL = 1000;

UIGrid::UIGrid()
	: m_dimensions(0, 0)
	, m_selected_index(0)
	, m_last_mouse_position(0, 0)
	, m_items_per_page(0)
	, m_page_arrow_offset_timer(0.0f)
	, m_page_arrow_offset_direction(false)
	, m_selection_time(0.0f)
	, m_draw_item_background(true)
	, m_page_arrow_hover_timer(0.0f)
	, m_mouse_down_timer(0.0f)
	, m_is_dragging(false)
	, m_selected_drag_index(-1)
	, m_selected_drop_index(-1)
{
}

UIGrid::~UIGrid()
{
}
	
void UIGrid::Refresh()
{
	m_manager				= GameEngine::Get()->Get_UIManager();
	m_atlas					= m_manager->Get_Atlas();
	m_screen_box			= Calculate_Screen_Box();
	
	// Grab all atlas frames.
	m_background_frame			= UIFrame("screen_main_box_active_#");
	m_active_background_frame	= UIFrame("screen_main_box_inactive_#");
	m_arrow_frame				= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_page_arrow");

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIGrid::After_Refresh()
{
	Refresh_Boxes(m_manager);
}

bool UIGrid::Is_Focusable()
{
	return true;
}

Vector2 UIGrid::Get_Focus_Point()
{
	return m_focus_point;
}

void UIGrid::Set_Draw_Item_Background(bool value)
{
	m_draw_item_background = value;		
}

void UIGrid::Set_Total_Items(int items)
{
	int item_span_x = (int)m_dimensions.X;
	int item_span_y = (int)m_dimensions.Y;

	m_total_items = items;
	m_total_pages = (int)ceilf((float)m_total_items / m_items_per_page);
	//m_current_page = 0;//Max(0, m_total_pages - 1);
	m_current_page = Clamp(m_current_page, 0, m_total_pages - 1);
}

void UIGrid::Drag_Item_Index(int item_index)
{
	m_selected_drag_index = item_index;// % m_items_per_page;
	m_is_dragging = true;
}

bool UIGrid::Focus_Up(UIManager* manager)
{
	if (m_selected_index >= (int)m_dimensions.X)
	{
		if (m_is_dragging == false)
		{
			manager->Play_UI_Sound(UISoundType::Click);
			m_selected_index -= (int)m_dimensions.X;
			m_selection_time = 0.0f;
		}
		return true;
	}
	return false;
}

bool UIGrid::Focus_Down(UIManager* manager)
{
	if (m_selected_index < m_items_per_page - (int)m_dimensions.X)
	{
		if (m_is_dragging == false)
		{
			manager->Play_UI_Sound(UISoundType::Click);
			m_selected_index += (int)m_dimensions.X;
			m_selection_time = 0.0f;
		}
		return true;
	}
	return false;
}

bool UIGrid::Focus_Left(UIManager* manager)
{
	int column = m_selected_index % (int)m_dimensions.X;
	int row = m_selected_index / (int)m_dimensions.X;
	if (column > 0)
	{
		if (m_is_dragging == false)
		{
			manager->Play_UI_Sound(UISoundType::Click);
			m_selected_index--;
			m_selection_time = 0.0f;
		}
		return true;
	}
	else
	{
		// Previous page?
		if (m_current_page > 0)
		{			
			if (m_is_dragging == false)
			{
				manager->Play_UI_Sound(UISoundType::Click);
				m_selected_index += (int)(m_dimensions.X - 1);
				m_selection_time = 0.0f;
				m_current_page--;
			}
			return true;
		}

		// Go up a row?
		else if (row > 0)
		{
			if (m_is_dragging == false)
			{
				manager->Play_UI_Sound(UISoundType::Click);
				m_selection_time = 0.0f;
				m_selected_index--;
			}
			return true;
		}
	}
	return false;
}

bool UIGrid::Focus_Right(UIManager* manager)
{
	int column = m_selected_index % (int)m_dimensions.X;
	int row = m_selected_index / (int)m_dimensions.X;
	if (column < (int)m_dimensions.X - 1)
	{
		if (m_is_dragging == false)
		{
			manager->Play_UI_Sound(UISoundType::Click);
			m_selection_time = 0.0f;
			m_selected_index++;
		}
		return true;
	}
	else
	{
		// Next page?
		if (m_current_page < m_total_pages - 1)
		{			
			if (m_is_dragging == false)
			{
				manager->Play_UI_Sound(UISoundType::Click);
				m_selection_time = 0.0f;
				m_selected_index -= (int)(m_dimensions.X - 1);
				m_current_page++;
			}
			return true;
		}

		// Go down a row?
		else if (row < (int)m_dimensions.Y - 1)
		{
			if (m_is_dragging == false)
			{
				manager->Play_UI_Sound(UISoundType::Click);
				m_selection_time = 0.0f;
				m_selected_index++;
			}
			return true;
		}
	}
	return false;
}

void UIGrid::Refresh_Boxes(UIManager* manager)
{
	float item_width  = ((m_screen_box.Width + m_grid_spacing.X) / m_dimensions.X) - m_grid_spacing.X;
	float item_height = ((m_screen_box.Height + m_grid_spacing.Y) / m_dimensions.Y) - m_grid_spacing.Y;

	float item_size = item_width;
	int item_span_x = (int)m_dimensions.X;
	int item_span_y = (int)m_dimensions.Y;

	m_items_per_page = (item_span_x * item_span_y);

	if (item_height < item_size)
	{
		item_size = item_height;
	}
	
	m_item_boxes.clear();
	
	for (int y = 0; y < item_span_y; y++)
	{
		for (int x = 0; x < item_span_x; x++)
		{
			Rect2D item_box = Rect2D(
				m_screen_box.X + (x * (item_size + m_grid_spacing.X)),	
				m_screen_box.Y + (y * (item_size + m_grid_spacing.Y)),
				item_size,
				item_size				
			);

			m_item_boxes.push_back(item_box);
		}
	}
}

void UIGrid::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);

	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();
	Vector2 mouse_delta = (mouse_position - m_last_mouse_position);

	bool mouse_down = mouse->Is_Button_Down(InputBindings::Mouse_Left);
	bool mouse_click = mouse->Was_Button_Down(InputBindings::Mouse_Left) && !mouse_down;

	bool mouse_moved = (fabs(mouse_delta.X) > 0 || fabs(mouse_delta.Y) > 0);
	bool mouse_drag_moved = (fabs(mouse_delta.X) > 2 || fabs(mouse_delta.Y) > 2);
	bool focusable = manager->Is_Element_Focusable(this);
	int last_selected_item = m_selected_index;
	
	int item_index = 0;
	for (std::vector<Rect2D>::iterator iter = m_item_boxes.begin(); iter != m_item_boxes.end(); iter++, item_index++)
	{
		Rect2D item_box = *iter;
		
		// Select using mouse?
		if (item_box.Intersects(mouse_position) && !mouse_down && focusable == true && mouse_moved)
		{
			if (m_is_dragging == false)
			{
				m_selected_index = item_index;
			}
		}

		// Clicked item / dragging item.
		if (mouse_click && item_box.Intersects(mouse_position) && focusable == true && !m_is_dragging)
		{
			m_selected_index = item_index;
			scene->Dispatch_Event(manager, UIEvent(UIEventType::Grid_Item_Click, this));
		}
	}

	// Dragging?
	if (mouse_down)
	{
		if (mouse_drag_moved && m_mouse_down_timer >= DRAG_DELAY && m_is_dragging == false)
		{
			UIGridCanDragData data;
			data.allow_drag = true;
			data.item_index = (m_current_page * m_items_per_page) + m_selected_index;

			if (data.item_index < m_total_items)
			{
				if (On_Can_Drag.Registered())
				{
					On_Can_Drag.Fire(&data);

					if (data.allow_drag == true)
					{
						m_selected_drag_index = data.item_index;
						m_is_dragging = true;

						scene->Dispatch_Event(manager, UIEvent(UIEventType::Grid_Begin_Drag, this));
					}
				}
			}
		}
	}

	if (mouse_down == true)
	{
		m_mouse_down_timer += time.Get_Frame_Time();
	}
	else
	{
		m_mouse_down_timer = 0.0f;

		if (m_is_dragging == true)
		{
			m_selected_drop_index = -1;
			
			int sub_item_index = 0;

			for (std::vector<Rect2D>::iterator iter = m_item_boxes.begin(); iter != m_item_boxes.end(); iter++, sub_item_index++)
			{
				Rect2D item_box = *iter;
				if (item_box.Intersects(mouse_position))
				{
					m_selected_drop_index = (m_current_page * m_items_per_page) + sub_item_index;
					break;
				}
			}

			scene->Dispatch_Event(manager, UIEvent(UIEventType::Grid_Finish_Drag, this));
			m_is_dragging = false;	
		}
	}
	
	// Selected item?
	if (manager->Was_Pressed(OutputBindings::GUI_Select) && focusable == true && Is_Focused(manager) && m_selected_index >= 0 && m_selected_index < m_items_per_page)
	{
		scene->Dispatch_Event(manager, UIEvent(UIEventType::Grid_Item_Click, this));
	}

	// Select different page with mouse.
	float arrow_spacing = 10 * ui_scale.X;

	bool bLeftHovering = false;
	bool bRightHovering = false;

	// Draw previous page arrow.
	if (m_current_page > 0)
	{
		if (Rect2D(m_screen_box.X - (arrow_spacing * 2), m_screen_box.Y, arrow_spacing * 2, m_screen_box.Height).Intersects(mouse_position))
		{
			bLeftHovering = true;

			if (mouse_click)
			{
				m_page_arrow_hover_timer = 0.0f;
				m_current_page--;
				manager->Play_UI_Sound(UISoundType::Click);
			}
			else
			{
				m_page_arrow_hover_timer += time.Get_Frame_Time();
				if (m_page_arrow_hover_timer > HOVER_PAGE_CHANGE_DELAY && m_is_dragging)
				{
					m_current_page--;
					manager->Play_UI_Sound(UISoundType::Click);
					m_page_arrow_hover_timer = 0.0f;
				}
			}
		}
	}

	// Draw next page arrow.
	if (m_total_pages > 1 && m_current_page < m_total_pages - 1)
	{
		if (Rect2D(m_screen_box.X + m_screen_box.Width, m_screen_box.Y, arrow_spacing * 2, m_screen_box.Height).Intersects(mouse_position))
		{
			bRightHovering = true;

			if (mouse_click == true)
			{
				m_page_arrow_hover_timer = 0.0f;
				m_current_page = Min(m_current_page + 1, m_total_pages - 1);
				manager->Play_UI_Sound(UISoundType::Click);
			}
			else
			{
				m_page_arrow_hover_timer += time.Get_Frame_Time();
				if (m_page_arrow_hover_timer > HOVER_PAGE_CHANGE_DELAY && m_is_dragging)
				{
					m_current_page = Min(m_current_page + 1, m_total_pages - 1);
					manager->Play_UI_Sound(UISoundType::Click);
					m_page_arrow_hover_timer = 0.0f;
				}
			}
		}
	}

	if (!bLeftHovering && !bRightHovering)
	{
		m_page_arrow_hover_timer = 0.0f;
	}

	// Play movement sound?
	if (last_selected_item != m_selected_index)
	{
		m_selection_time = 0.0f;
		manager->Play_UI_Sound(UISoundType::Click);
	}
	else
	{
		m_selection_time += time.Get_Frame_Time();
	}

	// Calculate focus point.
	Rect2D item_box = m_item_boxes.at(m_selected_index);
	m_focus_point = Vector2(item_box.X - 2, item_box.Y + (item_box.Height * 0.5f));
	
	// Perform a little in-out offset animation for the page icon scales.
	m_page_arrow_offset_timer += time.Get_Frame_Time();
	if (m_page_arrow_offset_timer >= PAGE_ARROW_OFFSET_INTERVAL)
	{
		m_page_arrow_offset_timer = 0.0f;
		m_page_arrow_offset_direction = !m_page_arrow_offset_direction;
	}

	m_last_mouse_position = mouse_position;
	UIElement::Tick(time, manager, scene);
}

void UIGrid::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	
	int item_index = 0;
	for (std::vector<Rect2D>::iterator iter = m_item_boxes.begin(); iter != m_item_boxes.end(); iter++, item_index++)
	{
		Rect2D item_box = *iter;
		
		int real_item_index = (m_current_page * m_items_per_page) + item_index;
		
		if (m_draw_item_background == true || real_item_index >= m_total_items)
		{
			if (item_index == m_selected_index)
				m_active_background_frame.Draw_Frame(m_atlas_renderer, item_box, ui_scale, Color(255, 255, 255, 200));
			else
				m_background_frame.Draw_Frame(m_atlas_renderer, item_box, ui_scale, Color(255, 255, 255, 200));
		}

		if (On_Draw_Item.Registered() && real_item_index < m_total_items)
		{
			UIGridDrawItemData data;
			data.view			= this;
			data.view_bounds	= m_screen_box;
			data.item_bounds	= item_box;
			data.time			= time;
			data.manager		= manager;
			data.scene			= scene;
			data.ui_scale		= ui_scale;
			data.item_index		= real_item_index;
			data.selected		= (item_index == m_selected_index);
			data.dragging		= false;

			On_Draw_Item.Fire(&data);
		}
	}
	
	float delta = m_page_arrow_offset_timer / PAGE_ARROW_OFFSET_INTERVAL;
	if (m_page_arrow_offset_direction == true)
	{
		delta = 1.0f - delta;
	}

	float arrow_spacing = 10 * ui_scale.X;
	float arrow_scale = Math::SmoothStep(PAGE_ARROW_OFFSET_MAX, 0, delta);

	// Draw item.
	if (m_is_dragging == true && m_selected_drag_index >= 0)
	{
		Rect2D box = m_item_boxes.at(m_selected_drag_index % m_items_per_page);

		Vector2 mouse_position = Input::Get()->Get_Mouse_State()->Get_Position();
		
		if (On_Draw_Item.Registered() && m_selected_drag_index < m_total_items)
		{
			UIGridDrawItemData data;
			data.view			= this;
			data.view_bounds	= m_screen_box;
			data.item_bounds	= Rect2D(
				mouse_position.X - (box.Width * 0.45f),
				mouse_position.Y - (box.Height * 0.45f),
				box.Width * 0.9f,
				box.Height * 0.9f
			);;
			data.time			= time;
			data.manager		= manager;
			data.scene			= scene;
			data.ui_scale		= ui_scale;
			data.item_index		= m_selected_drag_index;
			data.selected		= true;
			data.dragging		= true;

			On_Draw_Item.Fire(&data);
		}
	}

	// Draw previous page arrow.
	if (m_current_page > 0)
	{
		m_atlas_renderer.Draw_Frame(m_arrow_frame, Vector2(m_screen_box.X - arrow_spacing, m_screen_box.Y + (m_screen_box.Height * 0.5f)), 0.0f, Color::White, true, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale * (1.0f + arrow_scale));
	}

	// Draw next page arrow.
	if (m_total_pages > 1 && m_current_page < m_total_pages - 1)
	{
		m_atlas_renderer.Draw_Frame(m_arrow_frame, Vector2(m_screen_box.X + m_screen_box.Width + arrow_spacing, m_screen_box.Y + (m_screen_box.Height * 0.5f)), 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, ui_scale * (1.0f + arrow_scale));
	}

	// Draw each menu item.
	UIElement::Draw(time, manager, scene);
}

int UIGrid::Get_Selected_Index()
{
	return (m_current_page * m_items_per_page) + m_selected_index;
}

int UIGrid::Get_Drop_Selected_Index()
{
	if (m_selected_drop_index < 0)
	{
		return -1;
	}
	return m_selected_drop_index;
}

int UIGrid::Get_Drag_Selected_Index()
{
	if (m_selected_drag_index < 0)
	{
		return -1;
	}
	return m_selected_drag_index;
}

float UIGrid::Get_Selection_Time()
{
	return m_selection_time;
}

Rect2D UIGrid::Get_Selected_Item_Box()
{
	if (m_selected_index >= 0 && m_selected_index < (int)m_item_boxes.size())
	{
		return m_item_boxes.at(m_selected_index);
	}
	else
	{
		return Rect2D(0.0f, 0.0f, 0.0f, 0.0f);
	}
}