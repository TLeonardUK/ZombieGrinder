// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIGridTree.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"

#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIEvent.h"
#include "Engine/UI/UIManager.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Localise/Locale.h"

#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/PrimitiveRenderer.h"

UIGridTree::UIGridTree()
	: m_node_size(0, 0)
	, m_dragging(false)
	, m_selected_node(NULL)
	, m_should_lerp(true)
	, m_first_movement(true)
	, m_selection_time(false)
	, m_dragging_item(false)
	, m_mouse_moved(false)
	, m_auto_layout(true)
	, m_draw_node_bg(true)
	, m_selected_frame_name("screen_main_item_upgrade_socket_selected")
{
	m_last_mouse_position = Vector2(0.0f, 0.0f);
}

UIGridTree::~UIGridTree()
{
	Clear_Nodes();
}
	
void UIGridTree::Refresh()
{
	m_manager				= GameEngine::Get()->Get_UIManager();
	m_atlas					= m_manager->Get_Atlas();
	m_screen_box			= Calculate_Screen_Box();
	
	// Grab all atlas frames.
	m_background_frame			= UIFrame("screen_main_button_#");
	m_active_background_frame	= UIFrame("screen_main_box_inactive_#");
	m_arrow_frame				= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_page_arrow");

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIGridTree::After_Refresh()
{
}

bool UIGridTree::Is_Focusable()
{
	return true;
}

float UIGridTree::Get_Selection_Time()
{
	return m_selection_time;
}

Rect2D UIGridTree::Get_Selected_Item_Box()
{
	return m_selected_node->render_bounds;
}

UIGridTreeNode* UIGridTree::Get_Selected_Node()
{
	return m_selected_node;
}

void UIGridTree::Clear_Nodes()
{
	for (std::vector<UIGridTreeNode*>::iterator nodes = m_nodes.begin(); nodes != m_nodes.end(); nodes++)
	{
		SAFE_DELETE(*nodes);
	}
	m_nodes.clear();
	m_root_node = NULL;
}

UIGridTreeNode* UIGridTree::Add_Node(void* meta_data, UIGridTreeNode* parent, bool is_special, int manual_x, int manual_y, bool connect_left, bool connect_top, bool connect_right, bool connect_down)
{
	UIGridTreeNode* node = new UIGridTreeNode();
	node->meta_data = meta_data;
	node->parent = parent;
	node->depth = 0;
	node->width = 0;
	node->is_special = is_special;
	node->manual_x = manual_x;
	node->manual_y = manual_y;
	node->connect_left = connect_left;
	node->connect_top = connect_top;
	node->connect_right = connect_right;
	node->connect_down = connect_down;
	node->connect_left_enabled = true;
	node->connect_top_enabled = true;
	node->connect_right_enabled = true;
	node->connect_down_enabled = true;

	if (parent != NULL)
	{
		parent->children.push_back(node);
	}

	m_nodes.push_back(node);

	if (parent == NULL && m_root_node == NULL)
	{
		m_root_node = node;
	}

	return node;
}

void UIGridTree::Select_Node_By_Meta_Data(void* meta_data)
{
	for (std::vector<UIGridTreeNode*>::iterator nodes = m_nodes.begin(); nodes != m_nodes.end(); nodes++)
	{
		UIGridTreeNode* node = *nodes;
		if (node->meta_data == meta_data)
		{
			if (node != m_selected_node)
			{
				Change_Selected_Node(GameEngine::Get()->Get_UIManager(), node);
			}
			return;
		}
	}
}	

bool UIGridTree::Is_Dragging_Item()
{
	return m_dragging_item;
}

void UIGridTree::Drag_Node_By_Meta_Data(void* meta_data)
{
	for (std::vector<UIGridTreeNode*>::iterator nodes = m_nodes.begin(); nodes != m_nodes.end(); nodes++)
	{
		UIGridTreeNode* node = *nodes;
		if (node->meta_data == meta_data)
		{
			//if (node != m_selected_node)
			//{
				m_selected_node = node;
				m_should_lerp = true;
				m_dragging_item = true;
			//}
			return;
		}
	}
}

void UIGridTree::Refresh_Tree()
{
	m_tree_bounds = Rect2D(0, 0, 0, 0);
	m_tree_bounds_px = Rect2D(0, 0, 0, 0);
	m_selected_node = NULL;
	m_selection_time = 0.0f;

	// Calculate width/depth of tree.
	Refresh_Node(m_root_node, 0);
	
	// Work out all sub node positions.
	Calculate_Node_Position(m_root_node, 0, 0);

	m_first_movement = true;
	m_selected_node = m_root_node;
	m_tree_offset.X = -m_selected_node->bounds.X;
	m_tree_offset.Y =-m_selected_node->bounds.Y;
}

int UIGridTree::Refresh_Node(UIGridTreeNode* node, int depth)
{
	node->mid_width = (node->children.size() == 0 ? 1 : 0);
	node->depth = depth;
	node->left_width = 0;
	node->right_width = 0;

	int mid_point = (int)floorf(node->children.size() / 2.0f);
	bool is_even = (node->children.size() % 2) == 0;

	int index = 0;
	for (std::vector<UIGridTreeNode*>::iterator iter = node->children.begin(); iter != node->children.end(); iter++, index++)
	{
		UIGridTreeNode* sub = *iter;

		int sub_width = Refresh_Node(sub, depth + 1);

		if (index < mid_point)
		{
			node->left_width += sub_width;
		}
		else if (index > mid_point)
		{			
			node->right_width += sub_width;
		}
		else
		{
			if (is_even)
			{
				node->mid_width++;
				node->right_width += sub_width;
			}
			else
			{
				node->mid_width += sub_width;
			}
		}
	}

	node->width = node->left_width + node->right_width + node->mid_width;

	return node->width;
}

void UIGridTree::Calculate_Node_Position(UIGridTreeNode* node, int x, int y)
{
	// Tree is build horizontally;
	// eg.
	//
	//   s s
	//  s 
	//   s s
	//  s s s
	// r 
	//   s s s
	//  s 
	//	  s s s
	//	 s
	//	  s s s

	node->x = x;
	node->y = y + (node->left_width);
	
	if (!m_auto_layout)
	{
		node->x = node->manual_x;
		node->y = node->manual_y;
	}

	int mid_point = (int)floorf(node->children.size() / 2.0f);
	bool is_even = (node->children.size() % 2) == 0;
	
	// ABC DEF
	//    |
	//	ABCDE
	//	  |

	int offset = node->y - node->left_width;

	int index = 0;
	for (std::vector<UIGridTreeNode*>::iterator iter = node->children.begin(); iter != node->children.end(); iter++, index++)
	{
		UIGridTreeNode* sub = *iter;
		
		if (index < mid_point)
		{
			Calculate_Node_Position(sub, x + 1, offset);
			offset += sub->width;
		}
		else if (index == mid_point)
		{
			if (is_even)
			{
				offset++;
			}
			
			Calculate_Node_Position(sub, x + 1, offset);
			offset += sub->width;
		}
		else if (index > mid_point)
		{
			Calculate_Node_Position(sub, x + 1, offset);
			offset += sub->width;
		}
	}

	// Work out tree boundries.
	Vector2 ui_scale = Get_Draw_Scale(GameEngine::Get()->Get_UIManager());

	m_tree_bounds = m_tree_bounds.Union(Vector2(node->x, node->y));

	float x_padding = 25.0f * ui_scale.X;
	float y_padding = 5.0f * ui_scale.Y;

	Vector2 real_node_size = m_node_size * Vector2(m_screen_box.Height, m_screen_box.Height);
	
	float node_x = (node->x * (real_node_size.X * 2.0f));
	float node_y = (node->y * (real_node_size.Y * 1.5f));
	Rect2D node_box = Rect2D(
		node_x - (real_node_size.X * 0.5f),
		node_y - (real_node_size.Y * 0.5f),
		real_node_size.X,
		real_node_size.Y
	);

	node->bounds = node_box;

	m_tree_bounds_px = m_tree_bounds_px.Union(node_box);	
}

Vector2 UIGridTree::Get_Focus_Point()
{
	return m_focal_point;
}

UIGridTreeNode* UIGridTree::Get_Node_By_Position(int x, int y)
{
	for (std::vector<UIGridTreeNode*>::iterator iter = m_nodes.begin(); iter != m_nodes.end(); iter++)
	{
		UIGridTreeNode* node = *iter;
		if (node->x == x && node->y == y)
		{
			return node;
		}
	}
	return NULL;
}

void UIGridTree::Get_Above_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node)
{
	if (m_auto_layout)
	{
		if (node->x == x && node->y < y)
		{
			int new_diff = abs(node->y - y);
			int old_diff = result == NULL ? 999999 : abs(result->y - y);

			if (new_diff < old_diff)
			{
				result = node;
			}
		}

		if (node->x < x)
		{
			for (std::vector<UIGridTreeNode*>::iterator nodes = node->children.begin(); nodes != node->children.end(); nodes++)
			{
				Get_Above_Node(x, y, result, *nodes);
			}
		}
	}
	else
	{
		// keep going up rows and looking further horizontal till we find a node
		int tree_width = (int)(abs(m_tree_bounds.X) + m_tree_bounds.Width);
		for (int spread = 0; spread <= tree_width; spread++)
		{
			for (int ny = y - 1; ny >= m_tree_bounds.Y; ny--)
			{
				for (int nx = x - spread; nx <= x + spread; nx++)
				{					
					UIGridTreeNode* node = Get_Node_By_Position(nx, ny);
					if (node != NULL)
					{
						result = node;
						return;
					}
				}
			}
		}
	}
}

void UIGridTree::Get_Below_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node)
{
	if (m_auto_layout)
	{
		if (node->x == x && node->y > y)
		{
			int new_diff = abs(node->y - y);
			int old_diff = result == NULL ? 999999 : abs(result->y - y);

			if (new_diff < old_diff)
			{
				result = node;
			}
		}

		if (node->x < x)
		{
			for (std::vector<UIGridTreeNode*>::iterator nodes = node->children.begin(); nodes != node->children.end(); nodes++)
			{
				Get_Below_Node(x, y, result, *nodes);
			}
		}
	}
	else
	{
		// keep going up rows and looking further horizontal till we find a node
		int tree_width = (int)(abs(m_tree_bounds.X) + m_tree_bounds.Width);
		for (int spread = 0; spread <= tree_width; spread++)
		{
			for (int ny = y + 1; ny <= m_tree_bounds.Y + m_tree_bounds.Height; ny++)
			{
				for (int nx = x - spread; nx <= x + spread; nx++)
				{
					UIGridTreeNode* node = Get_Node_By_Position(nx, ny);
					if (node != NULL)
					{
						result = node;
						return;
					}
				}
			}
		}
	}
}

void UIGridTree::Get_Left_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node)
{
	if (m_auto_layout)
	{
		// TODO: Unused atm.
	}
	else
	{
		// keep going up rows and looking further horizontal till we find a node
		int tree_height = (int)(abs(m_tree_bounds.Y) + m_tree_bounds.Height);
		for (int spread = 0; spread <= tree_height; spread++)
		{
			for (int nx = x - 1; nx >= m_tree_bounds.X; nx--)
			{
				for (int ny = y - spread; ny <= y + spread; ny++)
				{
					UIGridTreeNode* node = Get_Node_By_Position(nx, ny);
					if (node != NULL)
					{
						result = node;
						return;
					}
				}
			}
		}
	}
}

void UIGridTree::Get_Right_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node)
{
	if (m_auto_layout)
	{
		// TODO: Unused atm.
	}
	else
	{
		// keep going up rows and looking further horizontal till we find a node
		int tree_height = (int)(abs(m_tree_bounds.Y) + m_tree_bounds.Height);
		for (int spread = 0; spread <= tree_height; spread++)
		{
			for (int nx = x + 1; nx <= m_tree_bounds.X + m_tree_bounds.Width; nx++)
			{
				for (int ny = y - spread; ny <= y + spread; ny++)
				{
					UIGridTreeNode* node = Get_Node_By_Position(nx, ny);
					if (node != NULL)
					{
						result = node;
						return;
					}
				}
			}
		}
	}
}

void UIGridTree::Change_Selected_Node(UIManager* manager, UIGridTreeNode* node)
{
	if (m_dragging_item == true)
	{
		return;
	}

	m_selected_node = node;		
	m_selection_time = 0.0f;
	manager->Play_UI_Sound(UISoundType::Click);
	m_should_lerp = true;
}

bool UIGridTree::Focus_Up(UIManager* manager)
{
	if (!m_auto_layout)
	{
		if (m_selected_node)
		{
			UIGridTreeNode* node = NULL;
			Get_Above_Node(m_selected_node->x, m_selected_node->y, node, m_selected_node);

			if (node != NULL)
			{
				Change_Selected_Node(manager, node);
				return true;
			}
		}
	}
	else
	{
		if (m_selected_node != NULL)
		{
			// No children, move to right sibling.
			if (m_selected_node->parent != NULL)// && m_selected_node->children.size() == 0)
			{
				/*
				int our_index = std::find(m_selected_node->parent->children.begin(),
										  m_selected_node->parent->children.end(),
										  m_selected_node) - m_selected_node->parent->children.begin();
				if (our_index > 0)
				{
					m_selected_node = m_selected_node->parent->children.at(our_index - 1);		
					m_selection_time = 0.0f;
					manager->Play_UI_Sound(UISoundType::Click);
					m_should_lerp = true;
					return true;
				}*/
			
				UIGridTreeNode* node = NULL;
				Get_Above_Node(m_selected_node->x, m_selected_node->y, node, m_root_node);

				if (node != NULL)
				{
					Change_Selected_Node(manager, node);
					return true;
				}
			}

			// Move to left child.
			if (m_selected_node->children.size() > 1)
			{
				Change_Selected_Node(manager, m_selected_node->children.at((m_selected_node->children.size() / 2) - 1));
				return true;
			}
		}
	}

	return false;
}

bool UIGridTree::Focus_Down(UIManager* manager)
{
	if (!m_auto_layout)
	{
		if (m_selected_node)
		{
			UIGridTreeNode* node = NULL;
			Get_Below_Node(m_selected_node->x, m_selected_node->y, node, m_selected_node);

			if (node != NULL)
			{
				Change_Selected_Node(manager, node);
				return true;
			}
		}
	}
	else
	{
		if (m_selected_node != NULL)
		{
			if (m_selected_node->parent != NULL)// && m_selected_node->children.size() == 0)
			{
				/*
				int our_index = std::find(m_selected_node->parent->children.begin(),
										  m_selected_node->parent->children.end(),
										  m_selected_node) - m_selected_node->parent->children.begin();
				if (our_index < m_selected_node->parent->children.size() - 1)
				{
					m_selected_node = m_selected_node->parent->children.at(our_index + 1);		
					m_selection_time = 0.0f;
					manager->Play_UI_Sound(UISoundType::Click);
					m_should_lerp = true;
					return true;
				}*/

				UIGridTreeNode* node = NULL;
				Get_Below_Node(m_selected_node->x, m_selected_node->y, node, m_root_node);

				if (node != NULL)
				{
					Change_Selected_Node(manager, node);
					return true;
				}
			}

			// Move to right child.
			if (m_selected_node->children.size() > 1)
			{
				Change_Selected_Node(manager, m_selected_node->children.at(m_selected_node->children.size() / 2));
				return true;
			}
		}
	}

	// No children, move to right sibling.
	return false;
}

bool UIGridTree::Focus_Left(UIManager* manager)
{
	if (!m_auto_layout)
	{
		if (m_selected_node)
		{
			UIGridTreeNode* node = NULL;
			Get_Left_Node(m_selected_node->x, m_selected_node->y, node, m_selected_node);

			if (node != NULL)
			{
				Change_Selected_Node(manager, node);
				return true;
			}
		}
	}
	else
	{
		// Move to parent.
		if (m_selected_node != NULL)
		{
			if (m_selected_node->parent != NULL)
			{
				Change_Selected_Node(manager, m_selected_node->parent);	
				return true;
			}
		}
	}

	return false;
}

bool UIGridTree::Focus_Right(UIManager* manager)
{
	if (!m_auto_layout)
	{
		if (m_selected_node)
		{
			UIGridTreeNode* node = NULL;
			Get_Right_Node(m_selected_node->x, m_selected_node->y, node, m_selected_node);

			if (node != NULL)
			{
				Change_Selected_Node(manager, node);
				return true;
			}
		}
	}
	else
	{
		// Move to mid-point child.
		if (m_selected_node != NULL)
		{
			if ((m_selected_node->children.size() % 2) != 0)
			{
				Change_Selected_Node(manager, m_selected_node->children.at((m_selected_node->children.size() / 2)));
				return true;
			}
			else if (m_selected_node->children.size() > 0) // Better to move to a random child than go to back/ok button.
			{
				Change_Selected_Node(manager, m_selected_node->children.at((m_selected_node->children.size() / 2) - 1));
				return true;
			}
		}
	}

	return false;
}

void UIGridTree::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	double ticks = Platform::Get()->Get_Ticks();

	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	m_mouse_moved = (mouse_position != m_last_mouse_position);
	
	Vector2 ui_scale = Get_Draw_Scale(manager);
	Vector2 real_node_size = m_node_size * Vector2(m_screen_box.Height, m_screen_box.Height);

	bool focusable = manager->Is_Element_Focusable(this);

	m_selection_time += time.Get_Frame_Time();

	if (mouse->Is_Button_Down(InputBindings::Mouse_Left) &&
		focusable == true)
	{
		if (m_dragging_item == true)
		{
			/// Do de do do
		}
		else if (m_selected_node && m_selected_node->render_bounds.Intersects(mouse_position))
		{
			UIGridTreeCanDragData can_drag;
			can_drag.node = m_selected_node;
			can_drag.allow_drag = true;

			On_Can_Drag.Fire(&can_drag);

			m_dragging_item = can_drag.allow_drag;
		}
		else
		{
			if (m_screen_box.Intersects(mouse_position))
			{
				if (m_dragging == true)
				{
					Vector2 mouse_delta = mouse_position - m_last_mouse_position;
					m_tree_offset.X += mouse_delta.X;
					m_tree_offset.Y += mouse_delta.Y;

				//	m_tree_offset.X = Min(m_tree_offset.X, m_screen_box.Width - real_node_size.X);
				//	m_tree_offset.X = Max(m_tree_offset.X, -(m_tree_bounds_px.Width - real_node_size.X));
				//				m_tree_offset.Y = Min(m_tree_offset.Y, m_tree_bounds_px.Height - (m_screen_box.Height * 0.5f) + (real_node_size.Y * 0.5f));
				//				m_tree_offset.Y = Max(m_tree_offset.Y, -(m_tree_bounds_px.Height + real_node_size.Y));

					m_tree_offset.X = Clamp(m_tree_offset.X, -m_tree_bounds_px.Width, m_tree_bounds_px.Width);
					m_tree_offset.Y = Clamp(m_tree_offset.Y, -m_tree_bounds_px.Height, m_tree_bounds_px.Height);
					
				}
				else
				{
					m_dragging = true;
					m_dragging_item = false;
				}
			}
		}
	}
	else
	{
		if (m_dragging_item == true)
		{
			scene->Dispatch_Event(manager, UIEvent(UIEventType::GridTree_Drop, this, NULL));
		}

		m_dragging = false;
		m_dragging_item = false;
	}

	if (m_selected_node != NULL && manager->Was_Pressed(OutputBindings::GUI_Select) && manager->Is_Element_Focusable(this) && Is_Focused(manager) && !manager->Transition_In_Progress())
	{
		scene->Dispatch_Event(manager, UIEvent(UIEventType::GridTree_Item_Click, this, NULL));
		manager->Play_UI_Sound(UISoundType::Select);
	}

	if (mouse_position != m_last_mouse_position && m_screen_box.Intersects(mouse_position) && m_first_movement == false)
	{		
		m_should_lerp = false;
	}

	if (m_selected_node != NULL)
	{
		if (m_should_lerp == true || m_first_movement == true)
		{
			float delta = 0.75f * time.Get_Delta();
			if (m_first_movement == true)
			{
				delta = 1.0f;
			}

			m_tree_offset.X = Math::Lerp(m_tree_offset.X, -m_selected_node->bounds.X, delta);
			m_tree_offset.Y = Math::Lerp(m_tree_offset.Y, -m_selected_node->bounds.Y, delta);
		}
	}

	m_first_movement = false;
	m_last_mouse_position = mouse_position;

	UIElement::Tick(time, manager, scene);
}

void UIGridTree::Draw_Node(const FrameTime& time, UIManager* manager, UIGridTreeNode* node, Rect2D* parent_box, UIScene* scene)
{
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();
	
	Vector2 ui_scale = Get_Draw_Scale(manager);

	float padding_left = 16.0f * ui_scale.X;
	Rect2D offset = Rect2D
	(
		(m_screen_box.X + (m_screen_box.Width * 0.5f)) - (node->bounds.Width * 0.5f),
		(m_screen_box.Y + (m_screen_box.Height * 0.5f)) - (node->bounds.Height * 0.5f),// + (m_tree_bounds_px.Y),// - (m_tree_bounds_px.Height * 0.5f),
		0.0f,
		0.0f
	);
	Rect2D node_box = m_tree_offset + offset + node->bounds;

	node->render_bounds = node_box;

	// Select node.
	if (m_screen_box.Intersects(mouse_position))
	{
		if (m_dragging == false && m_dragging_item == false)
		{
			if (node_box.Intersects(mouse_position))
			{
				//DBG_LOG("Intersects with node: %i, %i, %i", m_last_mouse_position != mouse_position, m_first_movement == false, m_selected_node != node);
				if (mouse->Was_Button_Clicked(InputBindings::Mouse_Left) && manager->Is_Element_Focusable(this) && !manager->Transition_In_Progress() && Is_Focused(manager))
				{
					if (node != m_selected_node)
					{
						m_selected_node = node;
						m_selection_time = 0.0f;
					}
					scene->Dispatch_Event(manager, UIEvent(UIEventType::GridTree_Item_Click, this, NULL));
					manager->Play_UI_Sound(UISoundType::Select);
				}
				else if (m_first_movement == false && m_selected_node != node && m_mouse_moved)
				{
					m_selected_node = node;
					m_selection_time = 0.0f;
					manager->Play_UI_Sound(UISoundType::Click);
				}
			}
		}
	}

	// Draw background rect.
	if (m_draw_node_bg)
	{
		m_background_frame.Draw_Frame(m_atlas_renderer, node_box, ui_scale, Color(255, 255, 255, 200));
	}

	PrimitiveRenderer pr;
	Color line_color = Color(50, 63, 69, 255);

	Color enabled_line_color = Color(50 * 1.0f, 63 * 1.0f, 69 * 1.0f, 255.0f);
	Color disabled_line_color = Color(50 * 0.3f, 63 * 0.3f, 69 * 0.3f, 255.0f);

	if (parent_box != NULL)
	{
		if (m_auto_layout)
		{
			// Draw line from parent to us.
			if (node->parent->is_special == true)
				line_color = Color(150, 63, 69, 255);

			Draw_Node_Connection(node_box, *parent_box, ui_scale, line_color);

			/*
			// Straight line to the node.
			if (node_box.Y == parent_box->Y)
			{		
				pr.Draw_Line(Vector3(parent_box->X + parent_box->Width, parent_box->Y + (parent_box->Width * 0.5f), 0), Vector3(node_box.X, parent_box->Y + (parent_box->Width * 0.5f), 0), 4.0f * ui_scale.Y, line_color);
			}

			// Right angle line.
			else
			{
				Vector3 node_point		= Vector3(node_box.X, node_box.Y + (node_box.Height * 0.5f), 0.0f);
				Vector3 mid_point		= Vector3(parent_box->X + (parent_box->Width * 0.5f), node_box.Y + (node_box.Height * 0.5f), 0.0f);
				Vector3 parent_point;

				if (parent_box->Y > node_box.Y)
				{
					parent_point = Vector3(parent_box->X + (parent_box->Width * 0.5f), parent_box->Y, 0.0f);
				}
				else
				{
					parent_point = Vector3(parent_box->X + (parent_box->Width * 0.5f), parent_box->Y + parent_box->Height, 0.0f);
				}

				Vector3 padding_x = Vector3((2.0f * ui_scale.Y) - 2.0f, 0.0f, 0.0f);
				Vector3 padding_y = Vector3(0.0f, (2.0f * ui_scale.Y) - 1.0f, 0.0f);
			
				if (parent_box->Y <= node_box.Y)
				{
					padding_y = -padding_y;
				}

				pr.Draw_Line(parent_point, mid_point - padding_y, 4.0f * ui_scale.Y, line_color);
				pr.Draw_Line(node_point, mid_point + padding_x, 4.0f * ui_scale.Y, line_color);
			}*/

		}
		else
		{
			if (node->connect_top == true)
			{
				UIGridTreeNode* connect_node = NULL;
				Get_Above_Node(node->x, node->y, connect_node, node);

				if (connect_node)
				{
					Rect2D connect_node_bounds = m_tree_offset + offset + connect_node->bounds;
					Draw_Node_Connection(node_box, connect_node_bounds, ui_scale, node->connect_top_enabled ? enabled_line_color : disabled_line_color);
				}
			}
			if (node->connect_down == true)
			{
				UIGridTreeNode* connect_node = NULL;
				Get_Below_Node(node->x, node->y, connect_node, node);

				if (connect_node)
				{
					Rect2D connect_node_bounds = m_tree_offset + offset + connect_node->bounds;
					Draw_Node_Connection(node_box, connect_node_bounds, ui_scale, node->connect_down_enabled ? enabled_line_color : disabled_line_color);
				}
			}
			if (node->connect_left == true)
			{
				UIGridTreeNode* connect_node = NULL;
				Get_Left_Node(node->x, node->y, connect_node, node);

				if (connect_node)
				{
					Rect2D connect_node_bounds = m_tree_offset + offset + connect_node->bounds;
					Draw_Node_Connection(node_box, connect_node_bounds, ui_scale, node->connect_left_enabled ? enabled_line_color : disabled_line_color);
				}
			}
			if (node->connect_right == true)
			{
				UIGridTreeNode* connect_node = NULL;
				Get_Right_Node(node->x, node->y, connect_node, node);

				if (connect_node)
				{
					Rect2D connect_node_bounds = m_tree_offset + offset + connect_node->bounds;
					Draw_Node_Connection(node_box, connect_node_bounds, ui_scale, node->connect_right_enabled ? enabled_line_color : disabled_line_color);
				}
			}
		}
	}

	// Draw icon.	
	UIGridTreeDrawItemData data;
	data.view = this;
	data.view_bounds = m_screen_box;
	data.item_bounds = node_box;
	data.time = time;
	data.manager = manager;
	data.ui_scale = ui_scale;
	data.node = node;
	data.selected = (m_selected_node == node);

	if (On_Draw_Item.Registered())
	{
		On_Draw_Item.Fire(&data);
	}
	else
	{
		PrimitiveRenderer pr;
		pr.Draw_Solid_Quad(data.item_bounds, Color::Red);
	}

	// Draw sub-nodes.
	for (std::vector<UIGridTreeNode*>::iterator iter = node->children.begin(); iter != node->children.end(); iter++)
	{
		UIGridTreeNode* sub_node = *iter;
		Draw_Node(time, manager, sub_node, &node_box, scene);
	}
}

void UIGridTree::Draw_Node_Connection(Rect2D src_box, Rect2D dst_box, Vector2 ui_scale, Color line_color)
{
	bool bAngled = !((src_box.X == dst_box.X) || (src_box.Y == dst_box.Y));

	// Is it angled?
	if (bAngled)
	{
		// Work out where to connect to/from.
		Vector3 src;
		Vector3 dst;
		Vector3 mid;

		// ----
		// |
		// |
		//   or
		// |
		// |
		// ----
		if (src_box.X > dst_box.X)
		{
			// src is connected at top or bottom
			// dst is connected at left or right

			// ----
			// |
			// |
			if (src_box.Y < dst_box.Y)
			{
				src = Vector3(src_box.X, src_box.Y + (dst_box.Height * 0.5f), 0.0f);
				dst = Vector3(dst_box.X + (dst_box.Width * 0.5f), dst_box.Y, 0.0f);
				mid = Vector3(dst.X, src.Y, 0.0f);
			}

			// |
			// |
			// ----
			else
			{
				src = Vector3(src_box.X, src_box.Y + (src_box.Height * 0.5f), 0.0f);
				dst = Vector3(dst_box.X + (dst_box.Width * 0.5f), dst_box.Y + dst_box.Height, 0.0f);
				mid = Vector3(dst.X, src.Y, 0.0f);
			}
		}

		// ----
		//    |
		//    |
		//   or
		//    |
		//    |
		// ----
		else
		{
			// dst is connected at left or right
			// src is connected at top or bottom

			// ----
			//    |
			//    |
			if (src_box.Y < dst_box.Y)
			{
				src = Vector3(src_box.X + src_box.Width, src_box.Y + (src_box.Width * 0.5f), 0.0f);
				dst = Vector3(dst_box.X + (dst_box.Width * 0.5f), dst_box.Y, 0.0f);
				mid = Vector3(dst.X, src.Y, 0.0f);
			}

			//    |
			//    |
			// ----
			else
			{
				src = Vector3(src_box.X + src_box.Width, src_box.Y + (src_box.Width * 0.5f), 0.0f);
				dst = Vector3(dst_box.X + (dst_box.Width * 0.5f), dst_box.Y + dst_box.Height, 0.0f);
				mid = Vector3(dst.X, src.Y, 0.0f);
			}
		}

		float size = 3.0f * ui_scale.Y;

		PrimitiveRenderer pr;
		pr.Draw_Line(src, mid, 4.0f * ui_scale.Y, line_color);
		pr.Draw_Line(dst, mid, 4.0f * ui_scale.Y, line_color);
		pr.Draw_Solid_Oval(Rect2D(mid.X - (size*0.5f), mid.Y - (size*0.5f), size, size), line_color);

	}

	// Nope straight, much easier.
	else
	{
		// Work out where to connect to/from.
		Vector3 src;
		Vector3 dst;

		if (src_box.Y == dst_box.Y)
		{
			if (src_box.X < dst_box.X)
			{
				src = Vector3(src_box.X + src_box.Width, src_box.Y + (src_box.Height * 0.5f), 0.0f); // right side
				dst = Vector3(dst_box.X, dst_box.Y + (dst_box.Height * 0.5f), 0.0f); // left side
			}
			else
			{
				src = Vector3(src_box.X, src_box.Y + (src_box.Height * 0.5f), 0.0f); // left side
				dst = Vector3(dst_box.X + dst_box.Width, dst_box.Y + (dst_box.Height * 0.5f), 0.0f); // right side
			}
		}
		else
		{
			if (src_box.Y < dst_box.Y)
			{
				src = Vector3(src_box.X + (src_box.Width * 0.5f), src_box.Y + src_box.Height, 0.0f); // bottom side
				dst = Vector3(dst_box.X + (src_box.Width * 0.5f), dst_box.Y, 0.0f); // top side
			}
			else
			{
				src = Vector3(src_box.X + (src_box.Width * 0.5f), src_box.Y, 0.0f); // top side
				dst = Vector3(dst_box.X + (dst_box.Width * 0.5f), dst_box.Y + dst_box.Height, 0.0f); // bottom side
			}
		}

		PrimitiveRenderer pr;
		pr.Draw_Line(src, dst, 4.0f * ui_scale.Y, line_color);
	}
}

void UIGridTree::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{	
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	Vector2 ui_scale = Get_Draw_Scale(manager);

	RenderPipeline* pipeline = RenderPipeline::Get();

	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box.Inflate(4.0f * ui_scale.X, 0.0f), ui_scale, Color(255, 255, 255, 220));

	pipeline->Set_Render_Batch_Scissor_Test(true);
	pipeline->Set_Render_Batch_Scissor_Rectangle(m_screen_box);

	if (m_selected_node != NULL)
	{
		float padding_left = 16.0f * ui_scale.X;
		Rect2D offset = Rect2D
			(
				(m_screen_box.X + (m_screen_box.Width * 0.5f)) - (m_root_node->bounds.Width * 0.5f),
				(m_screen_box.Y + (m_screen_box.Height * 0.5f)) - (m_root_node->bounds.Height * 0.5f),// + (m_tree_bounds_px.Y),// - (m_tree_bounds_px.Height * 0.5f),
				0.0f,
				0.0f
				);
		Rect2D node_box = m_tree_offset + offset + m_selected_node->bounds;

		m_focal_point = Vector2(node_box.X - (2 * ui_scale.X), node_box.Y + (node_box.Height * 0.5f));

		AtlasFrame* base_frame = ResourceFactory::Get()->Get_Atlas_Frame("screen_main_item_upgrade_socket");
		AtlasFrame* frame = ResourceFactory::Get()->Get_Atlas_Frame(m_selected_frame_name.c_str());

		float diff_x = frame->Rect.Width - base_frame->Rect.Width;
		float diff_y = frame->Rect.Height - base_frame->Rect.Height;

		float scale_x = node_box.Width / base_frame->Rect.Width;
		float scale_y = node_box.Height / base_frame->Rect.Height;

		float inc_x = (diff_x * scale_x) * 0.5f;
		float inc_y = (diff_y * scale_y) * 0.5f;

		Rect2D node_area(
			node_box.X - inc_x,
			node_box.Y - inc_y,
			node_box.Width + (inc_x * 2.0f),
			node_box.Height + (inc_y * 2.0f)
			);

		AtlasRenderer ar;
		ar.Draw_Frame(frame, node_area, 0.0f, Color::White, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	}

	Draw_Node(time, manager, m_root_node, NULL, scene);
	
	pipeline->Set_Render_Batch_Scissor_Test(false);

	// Draw item at mouse if dragging.
	if (m_dragging_item == true && m_selected_node != NULL)
	{
		UIGridTreeDrawItemData data;
		data.view			= this;
		data.view_bounds	= m_screen_box;
		data.item_bounds	= Rect2D(
			mouse_position.X - (m_selected_node->bounds.Width * 0.45f),
			mouse_position.Y - (m_selected_node->bounds.Height * 0.45f),
			m_selected_node->bounds.Width * 0.9f,
			m_selected_node->bounds.Height * 0.9f
		);
		data.time			= time;
		data.manager		= manager;
		data.ui_scale		= ui_scale;
		data.node			= m_selected_node;
		data.selected		= true;

		if (On_Draw_Item.Registered())
		{
			On_Draw_Item.Fire(&data);
		}
		else
		{
			PrimitiveRenderer pr;
			pr.Draw_Solid_Quad(data.item_bounds, Color::Red);
		}
	}

	/*
	Vector2 ui_scale = Get_Draw_Scale(manager);
	
	int item_index = 0;
	for (std::vector<Rect2D>::iterator iter = m_item_boxes.begin(); iter != m_item_boxes.end(); iter++, item_index++)
	{
		Rect2D item_box = *iter;

		if (item_index == m_selected_index)
			m_active_background_frame.Draw_Frame(m_atlas_renderer, item_box, ui_scale, Color(255, 255, 255, 200));
		else
			m_background_frame.Draw_Frame(m_atlas_renderer, item_box, ui_scale, Color(255, 255, 255, 200));

		int real_item_index = (m_current_page * m_items_per_page) + item_index;

		if (On_Draw_Item.Registered() && real_item_index < m_total_items)
		{
			UIGridTreeDrawItemData data;
			data.view			= this;
			data.view_bounds	= m_screen_box;
			data.item_bounds	= item_box;
			data.time			= time;
			data.manager		= manager;
			data.scene			= scene;
			data.ui_scale		= ui_scale;

			On_Draw_Item.Fire(&data);
		}
	}
	
	float delta = m_page_arrow_offset_timer / PAGE_ARROW_OFFSET_INTERVAL;
	if (m_page_arrow_offset_direction == true)
	{
		delta = 1.0f - delta;
	}
	*/

	// Draw each menu item.
	UIElement::Draw(time, manager, scene);
}
