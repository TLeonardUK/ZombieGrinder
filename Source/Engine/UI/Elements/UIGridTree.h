// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIGRIDTREE_
#define _ENGINE_UI_ELEMENTS_UIGRIDTREE_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Input/OutputBindings.h"

#include "Generic/Events/Event.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;
class UIGridTree;
class UIManager;
class UIScene;

struct UIGridTreeNode
{
public:
	void*							meta_data;
	UIGridTreeNode*					parent;
	std::vector<UIGridTreeNode*>	children;
	int								width;
	int								left_width;
	int								mid_width;
	int								right_width;
	int								depth;
	Rect2D							bounds;
	Rect2D							render_bounds;
	int								x, y;
	int								manual_x, manual_y;
	bool							is_special;

	bool							connect_top;
	bool							connect_left;
	bool							connect_right;
	bool							connect_down;

	bool							connect_top_enabled;
	bool							connect_left_enabled;
	bool							connect_right_enabled;
	bool							connect_down_enabled;
};

struct UIGridTreeDrawItemData
{
public:
	UIGridTree*		view;
	Rect2D			view_bounds;
	Rect2D			item_bounds;
	FrameTime		time;
	UIManager*		manager;
	Vector2			ui_scale;
	UIGridTreeNode*	node;
	bool			selected;
};

struct UIGridTreeCanDragData
{
public:
	UIGridTreeNode*	node;
	bool			allow_drag;
};

class UIGridTree : public UIElement 
{
	MEMORY_ALLOCATOR(UIGridTree, "UI");

protected:	
	friend class UILayout;

	UIManager*						m_manager;
	AtlasHandle*					m_atlas;
	AtlasRenderer					m_atlas_renderer;

	Vector2							m_node_size;

	UIFrame							m_background_frame;
	UIFrame							m_active_background_frame;
	AtlasFrame*						m_arrow_frame;
	
	std::vector<UIGridTreeNode*>	m_nodes;
	UIGridTreeNode*					m_root_node;
	Rect2D							m_tree_bounds;
	Rect2D							m_tree_bounds_px;

	Rect2D							m_tree_offset;

	bool							m_dragging;
	Vector2							m_last_mouse_position;

	UIGridTreeNode*					m_selected_node;
	Vector2							m_focal_point;

	bool							m_should_lerp;
	bool							m_first_movement;

	float							m_selection_time;

	bool							m_dragging_item;

	bool							m_auto_layout;

	bool							m_mouse_moved;

	bool							m_draw_node_bg;

	std::string						m_selected_frame_name;

private:
	UIGridTreeNode* Get_Node_By_Position(int x, int y);

	int Refresh_Node(UIGridTreeNode* node, int depth);
	void Calculate_Node_Position(UIGridTreeNode* node, int parent_x, int parent_y);

	void Draw_Node(const FrameTime& time, UIManager* manager, UIGridTreeNode* node, Rect2D* parent_box, UIScene* scene);

	void Draw_Node_Connection(Rect2D src_box, Rect2D dst_box, Vector2 ui_scale, Color line_color);

public:	
	Event<UIGridTreeDrawItemData> On_Draw_Item;
	Event<UIGridTreeCanDragData> On_Can_Drag;

public:
	UIGridTree();
	~UIGridTree();

	void Get_Above_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node);
	void Get_Below_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node);
	void Get_Left_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node);
	void Get_Right_Node(int x, int y, UIGridTreeNode*& result, UIGridTreeNode* node);

	bool Is_Dragging_Item();

	void Clear_Nodes();
	UIGridTreeNode* Add_Node(void* meta_data, UIGridTreeNode* parent, bool is_special, int manual_x = -1, int manual_y = -1, bool connect_left = false, bool connect_top = false, bool connect_right = false, bool connect_down = false);

	float Get_Selection_Time();
	Rect2D Get_Selected_Item_Box();
	UIGridTreeNode* Get_Selected_Node();

	void Change_Selected_Node(UIManager* manager, UIGridTreeNode* node);
	void Select_Node_By_Meta_Data(void* meta_data);
	void Drag_Node_By_Meta_Data(void* meta_data);

	void Refresh_Tree();

	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	Vector2 Get_Focus_Point();
	
	bool Focus_Up(UIManager* manager);
	bool Focus_Down(UIManager* manager);
	bool Focus_Left(UIManager* manager);
	bool Focus_Right(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

