// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIPropertyGrid.h"
#include "Engine/UI/Elements/UIToolbarItem.h"
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

#include <algorithm>

UIPropertyGrid::UIPropertyGrid()
	: m_node_padding(2.0f)
	, m_node_height(20.0f)
	, m_node_sub_x_spacing(20.0f)
	, m_slider(NULL)
	, m_slider_width(10.0f)
	, m_scroll_range(0.0f)
	, m_input_text_box(NULL)
	, m_input_box_node(NULL)
	, m_opened_input_box(false)
{
}

UIPropertyGrid::~UIPropertyGrid()
{
	Clear_Items();
}

void UIPropertyGrid::Clear_Items()
{
	for (std::vector<UIPropertyGridItem*>::iterator iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
	{
		UIPropertyGridItem* item = *iter;
		SAFE_DELETE(item);
	}
	m_root.Children.clear();
	m_nodes.clear();

	m_input_box_node = NULL;
	m_input_text_box->Set_Visible(false);
	m_input_text_box->Set_Enabled(false);
}

void UIPropertyGrid::Remove_Item(UIPropertyGridItem* item)
{
	// Remove from parent.
	if (item->Parent != NULL)
	{
		std::vector<UIPropertyGridItem*>::iterator iter = std::find(item->Parent->Children.begin(), item->Parent->Children.end(), item);
		if (iter != item->Parent->Children.end())
			item->Parent->Children.erase(iter);
	}

	// Remove all children.
	while (item->Children.size() > 0)
	{
		UIPropertyGridItem* it = item->Children.back();
		Remove_Item(it);
	}
	item->Children.clear();

	// Remove from global list.
	std::vector<UIPropertyGridItem*>::iterator iter = std::find(m_nodes.begin(), m_nodes.end(), item);
	if (iter != m_nodes.end())
		m_nodes.erase(iter);

	// Annnnd delete.
	SAFE_DELETE(item);
}

UIPropertyGridItem* UIPropertyGrid::Add_Item(std::string name, UIPropertyGridItem* parent, UIPropertyGridDataType::Type type, void* value, std::vector<std::string> combo_values, void* meta, int id)
{
	UIPropertyGridItem* item = new UIPropertyGridItem();
	item->Name = name;
	item->Parent = parent == NULL ? &m_root : parent;
	item->Type = type;
	item->Value = value;
	item->Parent->Children.push_back(item);
	item->Expanded = false;
	item->Combos = combo_values;
	item->Meta_Data = meta;
	item->ID = id;
	item->Set_Function = NULL;
	item->Get_Function = NULL;
	m_nodes.push_back(item);

	Refresh();

	return item;
}

UIPropertyGridItem* UIPropertyGrid::Add_Custom_Item(
	std::string name, 
	UIPropertyGridItem* parent, 
	UIPropertyGridDataType::Type type, 
	UIPropertyGridItem::Get_Value_Function get_func,
	UIPropertyGridItem::Set_Value_Function set_func,
	std::vector<std::string> combo_values, 
	void* meta, 
	int id,
	void* meta_2,
	std::string meta_3)
{
	UIPropertyGridItem* item = new UIPropertyGridItem();
	item->Name = name;
	item->Parent = parent == NULL ? &m_root : parent;
	item->Type = type;
	item->Value = NULL;
	item->Get_Function = get_func;
	item->Set_Function = set_func;
	item->Parent->Children.push_back(item);
	item->Expanded = false;
	item->Combos = combo_values;
	item->Meta_Data = meta;
	item->Meta_Data_2 = meta_2;
	item->Meta_Data_3 = meta_3;
	item->ID = id;
	m_nodes.push_back(item);

	Refresh();

	return item;
}

void UIPropertyGrid::Add_Item_Button(UIPropertyGridItem* item, std::string icon, int id)
{
	UIPropertyGridButton button;
	button.Frame = ResourceFactory::Get()->Get_Atlas_Frame(icon.c_str());
	button.ID = id;

	item->Buttons.push_back(button);

	Refresh();
}

void UIPropertyGrid::Refresh()
{
	// Calculate screen-space box.
	m_screen_box = Calculate_Screen_Box();
	
	m_manager		= GameEngine::Get()->Get_UIManager();
	m_atlas			= m_manager->Get_Atlas();
	m_atlas_renderer= AtlasRenderer(m_atlas);
	m_font			= m_manager->Get_Font();
	m_font_renderer	= FontRenderer(m_font, false, false);
	m_expand_frame   = ResourceFactory::Get()->Get_Atlas_Frame("property_grid_expand_0");
	m_collapse_frame = ResourceFactory::Get()->Get_Atlas_Frame("property_grid_expand_1");
	m_check_box		 = ResourceFactory::Get()->Get_Atlas_Frame("property_grid_check_0");
	m_uncheck_box	 = ResourceFactory::Get()->Get_Atlas_Frame("property_grid_check_1");
	
	m_icon_background_hover_frame		= UIFrame("toolbaritem_background_hover_#");
	m_icon_background_pressed_frame		= UIFrame("toolbaritem_background_pressed_#");
	m_icon_background_seperator_frame	= UIFrame("toolbaritem_background_seperator_#");
	m_icon_background_frame				= UIFrame("toolbaritem_background_#");
	
	// Add the slider.
	if (m_slider == NULL)
	{
		m_slider = new UISlider();
		m_slider->Set_Scene(m_scene);
		m_slider->Set_Parent(this);		
		m_slider->Set_Box(Rect2D(m_screen_box.Width - m_slider_width, 0.0f, m_slider_width, m_screen_box.Height));		
		m_slider->Set_Direction(UISliderDirection::Vertical);
		m_slider->Set_Focusable(false);
		m_slider->Set_Progress(1.0f);
		m_slider->Set_Game_Style(false);
		Add_Child(m_slider);
	}
	else
	{
		m_slider->Set_Box(Rect2D(m_screen_box.Width - m_slider_width, 0.0f, m_slider_width, m_screen_box.Height));		
	}
	
	// Add input text box.
	if (m_input_text_box == NULL)
	{
		m_input_text_box = new UITextBox();
		m_input_text_box->Set_Scene(m_scene);
		m_input_text_box->Set_Parent(this);
		m_input_text_box->Set_Box(Rect2D(10.0f, 10.0f, m_screen_box.Width - 20.0f, m_node_height));		
		m_input_text_box->Set_Enabled(false);
		m_input_text_box->Set_Visible(false);
		m_input_text_box->Set_Game_Style(false);
		Add_Child(m_input_text_box);
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIPropertyGrid::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIPropertyGrid::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 mouse_pos = Input::Get()->Get_Mouse_State()->Get_Position();

	if (m_screen_box.Intersects(mouse_pos) && m_scroll_range > 0.0f)
	{
		float item_height = m_node_height + m_node_padding;
		float delta = Input::Get()->Get_Mouse_State()->Get_Scroll_Value() * (item_height / m_scroll_range);
		m_slider->Set_Progress(Clamp(m_slider->Get_Progress() - delta, 0.0f, 1.0f));
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIPropertyGrid::Get_Node_Value(UIPropertyGridItem* item, float* val)
{
	if (item->Get_Function != NULL)
	{
		item->Get_Function(item, val);
	}
	else
	{
		*val = *reinterpret_cast<float*>(item->Value);
	}
}

void UIPropertyGrid::Get_Node_Value(UIPropertyGridItem* item, int* val)
{
	if (item->Get_Function != NULL)
	{
		item->Get_Function(item, val);
	}
	else
	{
		*val = *reinterpret_cast<int*>(item->Value);
	}
}

void UIPropertyGrid::Get_Node_Value(UIPropertyGridItem* item, bool* val)
{
	if (item->Get_Function != NULL)
	{
		item->Get_Function(item, val);
	}
	else
	{
		*val = *reinterpret_cast<bool*>(item->Value);
	}
}

void UIPropertyGrid::Get_Node_Value(UIPropertyGridItem* item, std::string* val)
{
	if (item->Get_Function != NULL)
	{
		item->Get_Function(item, val);
	}
	else
	{
		*val = *reinterpret_cast<std::string*>(item->Value);
	}
}

void UIPropertyGrid::Set_Node_Value(UIPropertyGridItem* item, float val)
{
	if (item->Set_Function != NULL)
	{
		item->Set_Function(item, &val);
	}
	else
	{
		*reinterpret_cast<float*>(item->Value) = val;
	}
}

void UIPropertyGrid::Set_Node_Value(UIPropertyGridItem* item, int val)
{
	if (item->Set_Function != NULL)
	{
		item->Set_Function(item, &val);
	}
	else
	{
		*reinterpret_cast<int*>(item->Value) = val;
	}
}

void UIPropertyGrid::Set_Node_Value(UIPropertyGridItem* item, bool val)
{
	if (item->Set_Function != NULL)
	{
		item->Set_Function(item, &val);
	}
	else
	{
		*reinterpret_cast<bool*>(item->Value) = val;
	}
}

void UIPropertyGrid::Set_Node_Value(UIPropertyGridItem* item, std::string val)
{
	if (item->Set_Function != NULL)
	{
		item->Set_Function(item, &val);
	}
	else
	{
		*reinterpret_cast<std::string*>(item->Value) = val;
	}
}

void UIPropertyGrid::Save_Input_Change(UIManager* manager, UIScene* scene)
{
	if (m_input_box_node == NULL)
		return;

	bool changed = false;

	switch (m_input_box_node->Type)
	{
	case UIPropertyGridDataType::String:
		{
			std::string new_val = m_input_text_box->Get_Value();
			std::string old_val = "";
			Get_Node_Value(m_input_box_node, &old_val);

			if (old_val != new_val)
			{
				Set_Node_Value(m_input_box_node, new_val);
				changed = true;
			}
			break;
		}
	case UIPropertyGridDataType::Int:
		{
			int new_val = atoi(m_input_text_box->Get_Value().c_str());
			int old_val = 0;
			Get_Node_Value(m_input_box_node, &old_val);

			if (old_val != new_val)
			{
				Set_Node_Value(m_input_box_node, new_val);
				changed = true;
			}
			break;
		}
	case UIPropertyGridDataType::Float:
		{
			float new_val = (float)atof(m_input_text_box->Get_Value().c_str());
			float old_val = 0.0f;
			Get_Node_Value(m_input_box_node, &old_val);

			if (old_val != new_val)
			{
				Set_Node_Value(m_input_box_node, new_val);
				changed = true;
			}
			break;
		}
	}
	
	if (changed)
		scene->Dispatch_Event(manager, UIEvent(UIEventType::PropertyGrid_Changed, this));

	m_input_box_node = NULL;
}

void UIPropertyGrid::Draw_Node(const FrameTime& time, UIManager* manager, UIScene* scene, UIPropertyGridItem* item, float& x_offset, float& y_offset)
{
	bool expandable = (item->Children.size() > 0) || item->Type == UIPropertyGridDataType::None;

	Rect2D item_box = Rect2D
	(
		m_screen_box.X + x_offset + m_node_padding,
		m_screen_box.Y + y_offset + m_node_padding,
		m_screen_box.Width - m_slider_width - (m_node_padding * 2) - x_offset,
		m_node_height
	);

	Rect2D full_item_box = Rect2D
	(
		m_screen_box.X + x_offset + m_node_padding,
		m_screen_box.Y + y_offset + m_node_padding,
		m_screen_box.Width - m_slider_width - (m_node_padding * 2) - x_offset,
		m_node_height
	);

	Rect2D expand_item_box = Rect2D
	(
		item_box.X + (m_node_height * 0.25f),
		item_box.Y + (m_node_height * 0.25f),
		m_node_height * 0.5f,
		m_node_height * 0.5f
	);
	
	Rect2D text_item_box = item_box;

	if (expandable)
	{
		text_item_box = Rect2D
		(
			item_box.X + m_node_sub_x_spacing,
			item_box.Y,
			item_box.Width - m_node_sub_x_spacing,
			item_box.Height
		);
	}

	Rect2D name_bg_box = Rect2D
	(
		item_box.X,
		item_box.Y,
		(item_box.Width * 0.5f) - 2,
		item_box.Height
	);
	Rect2D value_bg_box = Rect2D
	(
		item_box.X + (item_box.Width * 0.5f),
		item_box.Y,
		(item_box.Width * 0.5f),
		item_box.Height
	);

	Vector2 ui_scale = manager->Get_UI_Scale();
	
	PrimitiveRenderer pr;
	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();

	// Draw background.
	if (!expandable)
	{
		pr.Draw_Solid_Quad(name_bg_box, Color(255, 255, 255, 255));
		if (item->Type != UIPropertyGridDataType::External)
		{
			pr.Draw_Solid_Quad(value_bg_box, Color(255, 255, 255, 255));
		}
	}
	
	// Draw text.
	m_font_renderer.Draw_String(item->Name.c_str(), text_item_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);

	// Draw toolbar buttons on right side.
	float icon_spacing = 1.0f;
	float icon_size = full_item_box.Height - (icon_spacing * 2);

	float offset = full_item_box.X + full_item_box.Width - icon_size - icon_spacing;
	int index = 0;

	for (std::vector<UIPropertyGridButton>::iterator iter = item->Buttons.begin(); iter != item->Buttons.end(); iter++)
	{
		UIPropertyGridButton button = *iter;

		Rect2D icon_rect = Rect2D
		(
			(float)offset,
			(float)full_item_box.Y + icon_spacing,
			(float)icon_size,
			(float)icon_size
		);

		bool mouse_over = icon_rect.Intersects(mouse_pos);
		bool mouse_down = mouse->Is_Button_Down(InputBindings::Mouse_Left);
		bool mouse_clicked = mouse->Was_Button_Clicked(InputBindings::Mouse_Left);

		if (mouse_over == true && mouse_down == true)
		{
			m_icon_background_pressed_frame.Draw_Frame(m_atlas_renderer, icon_rect);
		}
		else if (mouse_over == true)
		{
			m_icon_background_hover_frame.Draw_Frame(m_atlas_renderer, icon_rect);
		}
		else
		{
			m_icon_background_frame.Draw_Frame(m_atlas_renderer, icon_rect);
		}
		
		if (button.Frame != NULL)
		{		
			Vector2 pos = Vector2(icon_rect.X + (icon_rect.Width / 2) - (button.Frame->Rect.Width / 2), 
							      icon_rect.Y + (icon_rect.Height / 2) - (button.Frame->Rect.Height / 2));
			m_atlas_renderer.Draw_Frame(button.Frame, pos, 0.0f, Color::White);
		}

		if (mouse_over == true && mouse_clicked == true)
		{
			UIEvent evt		= UIEvent(UIEventType::PropertyGrid_ButtonClicked, this);
			evt.Param		= button.ID;
			evt.Sub_Source	= item;
			scene->Dispatch_Event(manager, evt);
		}

		offset -= icon_size + icon_spacing;
		index++;
	}

	// Draw value.
	std::string value = "";
	switch (item->Type)
	{
	case UIPropertyGridDataType::String:
		{
			std::string res = "";
			Get_Node_Value(item, &res);

			value = res;
			m_font_renderer.Draw_String(value.c_str(), value_bg_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
			break;
		}
	case UIPropertyGridDataType::Int:
		{
			int res = 0;
			Get_Node_Value(item, &res);

			value = StringHelper::To_String(res);
			m_font_renderer.Draw_String(value.c_str(), value_bg_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
			break;
		}
	case UIPropertyGridDataType::Float:
		{
			float res = 0;
			Get_Node_Value(item, &res);

			value = StringHelper::To_String(res);
			m_font_renderer.Draw_String(value.c_str(), value_bg_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
			break;
		}
	case UIPropertyGridDataType::External:
		{
			value = "Click To Edit";
			m_font_renderer.Draw_String(value.c_str(), value_bg_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
			break;
		}
	case UIPropertyGridDataType::Combo:
		{
			int res = 0;
			Get_Node_Value(item, &res);

			value = item->Combos[res];
			m_font_renderer.Draw_String(value.c_str(), value_bg_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);
			break;
		}
	case UIPropertyGridDataType::Bool:
		{
			Rect2D check_item_box = Rect2D
			(
				(int)value_bg_box.X + 2,
				(int)(value_bg_box.Y + (value_bg_box.Height * 0.5f) - (m_check_box->Rect.Height * 0.5f)),
				(int)m_check_box->Rect.Width,
				(int)m_check_box->Rect.Height
			);
			
			if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && check_item_box.Intersects(mouse_pos))
			{
				bool res = false;
				Get_Node_Value(item, &res);
				Set_Node_Value(item, !res);
				//*reinterpret_cast<bool*>(item->Value) = !*reinterpret_cast<bool*>(item->Value);
			}
			
			bool res = false;
			Get_Node_Value(item, &res);

			value = StringHelper::To_String(res);

			if (res == false)
			{
				m_atlas_renderer.Draw_Frame(m_check_box, check_item_box, 0.0f, Color::White);
			}
			else
			{
				m_atlas_renderer.Draw_Frame(m_uncheck_box, check_item_box, 0.0f, Color::White);	
			}
	
			break;
		}
	}

	// Entering value?
	if (item->Type == UIPropertyGridDataType::String	||
		item->Type == UIPropertyGridDataType::Int		||
		item->Type == UIPropertyGridDataType::Float)
	{
		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && value_bg_box.Intersects(mouse_pos) && m_screen_box.Intersects(mouse_pos))
		{
			Save_Input_Change(manager, scene);	

			m_input_text_box->Set_Enabled(true);
			m_input_text_box->Set_Visible(true);
			m_input_text_box->Set_Box(Rect2D(value_bg_box.X - m_screen_box.X, value_bg_box.Y - m_screen_box.Y, value_bg_box.Width, value_bg_box.Height));
			m_input_text_box->Set_Value(value.c_str());
			m_input_text_box->Refresh();
			manager->Focus(m_input_text_box);
			
			scene->Dispatch_Event(manager, UIEvent(UIEventType::PropertyGrid_BeginEdit, this));

			m_input_box_node = item;
			m_opened_input_box = true;
		}
	}
	else if (item->Type == UIPropertyGridDataType::External)
	{
		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && value_bg_box.Intersects(mouse_pos) && m_screen_box.Intersects(mouse_pos))
		{
			UIEvent evt = UIEvent(UIEventType::PropertyGrid_ExternalClicked, this);
			evt.Sub_Source = item;
			evt.Param = item->ID;
			scene->Dispatch_Event(manager, evt);			
		}
	}
	else if (item->Type == UIPropertyGridDataType::Combo)
	{
		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && value_bg_box.Intersects(mouse_pos) && m_screen_box.Intersects(mouse_pos))
		{
			Save_Input_Change(manager, scene);	
			
			int subvalue = 0;
			Get_Node_Value(item, &subvalue);
			Set_Node_Value(item, (int)((subvalue + 1) % item->Combos.size()));

			//int value = (*reinterpret_cast<int*>(item->Value) + 1) % item->Combos.size();
			//*reinterpret_cast<int*>(item->Value) = value;
		}
	}

	// Expandable?
	if (expandable)
	{
		Rect2D expand_box = Rect2D(item_box.X, item_box.Y, offset - item_box.X, item_box.Height);
		if (item->Type == UIPropertyGridDataType::External)
			expand_box = name_bg_box;

		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && expand_box.Intersects(mouse_pos))
		{
			item->Expanded = !item->Expanded;
		}

		if (item->Expanded)
		{
			m_atlas_renderer.Draw_Frame(m_collapse_frame, expand_item_box, 0.0f, Color::White);
		}
		else
		{
			m_atlas_renderer.Draw_Frame(m_expand_frame, expand_item_box, 0.0f, Color::White);	
		}
	}

	// Move down and indent and draw children.
	y_offset += m_node_height + m_node_padding;

	if (item->Expanded)
	{
		x_offset += m_node_sub_x_spacing;
		Draw_Node_Children(time, manager, scene, item, x_offset, y_offset);
		x_offset -= m_node_sub_x_spacing;
	}
}

void UIPropertyGrid::Draw_Node_Children(const FrameTime& time, UIManager* manager, UIScene* scene, UIPropertyGridItem* item, float& x_offset, float& y_offset)
{
	for (std::vector<UIPropertyGridItem*>::iterator iter = item->Children.begin(); iter != item->Children.end(); iter++)
	{
		Draw_Node(time, manager, scene, *iter, x_offset, y_offset);
	}
}

void UIPropertyGrid::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	PrimitiveRenderer pr;

	RenderPipeline* pipeline = RenderPipeline::Get();	
	MouseState* mouse = Input::Get()->Get_Mouse_State();

	// Constrain drawing to our screen box, so if we scroll it dissappears correctly.
	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(m_screen_box);

	// Draw background.
	pr.Draw_Solid_Quad(m_screen_box, Color(240, 240, 240, 255));

	// Draw every node.
	float x_offset = 0.0f;
	float y_offset = -(m_scroll_range * m_slider->Get_Progress());
	float start_y_offset = y_offset;
	Draw_Node_Children(time, manager, scene, &m_root, x_offset, y_offset);
	
	// Range.
	m_scroll_range = Max(0, (y_offset - start_y_offset) - m_screen_box.Height);

	// CLose input box?
	if (m_opened_input_box == false)
	{
		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left))
		{			
			Save_Input_Change(manager, scene);	
			m_input_text_box->Set_Enabled(false);
			m_input_text_box->Set_Visible(false);
		}
	}
	m_opened_input_box = false;

	// Reset scissor rect. 
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw children.
 	UIElement::Draw(time, manager, scene);
}
