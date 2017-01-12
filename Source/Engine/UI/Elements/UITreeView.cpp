// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UITreeView.h"
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

UITreeView::UITreeView()
	: m_node_padding(4.0f)
	, m_node_height(20.0f)
	, m_node_sub_x_spacing(20.0f)
	, m_slider(NULL)
	, m_slider_width(10.0f)
	, m_scroll_range(0.0f)
	, m_selected_item(NULL)
{
}

UITreeView::~UITreeView()
{
	Clear_Items();
}

void UITreeView::Clear_Items()
{
	for (std::vector<UITreeViewItem*>::iterator iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
	{
		UITreeViewItem* item = *iter;
		SAFE_DELETE(item);
	}
	m_root.Children.clear();
	m_nodes.clear();
}

void UITreeView::Remove_Item(UITreeViewItem* item)
{
	// Remove from parent.
	if (item->Parent != NULL)
	{
		std::vector<UITreeViewItem*>::iterator iter = std::find(item->Parent->Children.begin(), item->Parent->Children.end(), item);
		if (iter != item->Parent->Children.end())
			item->Parent->Children.erase(iter);
	}

	// Remove all children.
	while (item->Children.size() > 0)
	{
		UITreeViewItem* it = item->Children.back();
		Remove_Item(it);
	}
	item->Children.clear();

	// Remove from global list.
	std::vector<UITreeViewItem*>::iterator iter = std::find(m_nodes.begin(), m_nodes.end(), item);
	if (iter != m_nodes.end())
		m_nodes.erase(iter);

	// Annnnd delete.
	SAFE_DELETE(item);
}

UITreeViewItem* UITreeView::Add_Item(std::string name, UITreeViewItem* parent, bool selectable, void* meta)
{
	UITreeViewItem* item = new UITreeViewItem();
	item->Name = name;
	item->Parent = parent == NULL ? &m_root : parent;
	item->Parent->Children.push_back(item);
	item->Expanded = false;
	item->Meta_Data = meta;
	item->Is_Selectable = selectable;
	m_nodes.push_back(item);

	Refresh();

	return item;
}

void UITreeView::Refresh()
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
	m_select_box	 = UIFrame("toolbaritem_background_hover_#");
	m_unselect_box	 = UIFrame("toolbaritem_background_#");
	
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

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UITreeView::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UITreeView::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
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

void UITreeView::Draw_Node(const FrameTime& time, UIManager* manager, UIScene* scene, UITreeViewItem* item, float& x_offset, float& y_offset)
{
	bool expandable = (item->Children.size() > 0);
	
	Rect2D bg_box = Rect2D
	(
		m_screen_box.X + m_node_padding,
		m_screen_box.Y + y_offset + m_node_padding,
		m_screen_box.Width - m_slider_width - (m_node_padding * 2),
		m_node_height
	);
	
	Rect2D item_box = Rect2D
	(
		m_screen_box.X + x_offset + m_node_padding,
		m_screen_box.Y + y_offset + m_node_padding,
		m_screen_box.Width - m_slider_width - (m_node_padding * 2) - x_offset,
		m_node_height
	);
	
	Rect2D icon_box = Rect2D
	(
		item_box.X + m_node_padding,
		item_box.Y + m_node_padding,
		item_box.Height - (m_node_padding * 2),
		item_box.Height - (m_node_padding * 2)
	);

	Rect2D text_box = Rect2D
	(
		item_box.X + icon_box.Width + (m_node_padding * 2),
		item_box.Y,
		item_box.Width - icon_box.Width - (m_node_padding * 4),
		item_box.Height
	);

	Vector2 ui_scale = manager->Get_UI_Scale();
	
	PrimitiveRenderer pr;
	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();

	// Draw background.
	if (m_selected_item == item)
	{
		if (item->Is_Selectable)
			m_select_box.Draw_Frame(m_atlas_renderer, bg_box);
		else
			m_unselect_box.Draw_Frame(m_atlas_renderer, bg_box);
	}

	// Draw text.
	m_font_renderer.Draw_String(item->Name.c_str(), text_box, 8.0f, Color::Black, TextAlignment::Left, TextAlignment::Center);

	// Select item.
	if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && item_box.Intersects(mouse_pos))
	{
		m_selected_item = item;
	}

	// Expandable?
	if (expandable)
	{
		if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && item_box.Intersects(mouse_pos))
		{
			item->Expanded = !item->Expanded;
		}

		if (item->Expanded)
		{
			m_atlas_renderer.Draw_Frame(m_collapse_frame, icon_box, 0.0f, Color::White);
		}
		else
		{
			m_atlas_renderer.Draw_Frame(m_expand_frame, icon_box, 0.0f, Color::White);	
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

void UITreeView::Draw_Node_Children(const FrameTime& time, UIManager* manager, UIScene* scene, UITreeViewItem* item, float& x_offset, float& y_offset)
{
	for (std::vector<UITreeViewItem*>::iterator iter = item->Children.begin(); iter != item->Children.end(); iter++)
	{
		Draw_Node(time, manager, scene, *iter, x_offset, y_offset);
	}
}

void UITreeView::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
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

	// Reset scissor rect. 
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw children.
 	UIElement::Draw(time, manager, scene);
}
