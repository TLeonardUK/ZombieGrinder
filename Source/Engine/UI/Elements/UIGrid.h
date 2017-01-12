// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIGRID_
#define _ENGINE_UI_ELEMENTS_UIGRID_

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
class UIGrid;
class UIManager;
class UIScene;

struct UIGridDrawItemData
{
public:
	UIGrid*			view;
	Rect2D			view_bounds;
	Rect2D			item_bounds;
	FrameTime		time;
	UIManager*		manager;
	UIScene*		scene;
	int				item_index;
	Vector2			ui_scale;
	bool			selected;
	bool			dragging;
};

struct UIGridCanDragData
{
public:
	int				item_index;
	bool			allow_drag;
};

class UIGrid : public UIElement 
{
	MEMORY_ALLOCATOR(UIGrid, "UI");

protected:	
	friend class UILayout;

	UIManager*			m_manager;
	AtlasHandle*		m_atlas;
	AtlasRenderer		m_atlas_renderer;

	Vector2				m_dimensions;
	Vector2				m_grid_spacing;

	UIFrame				m_background_frame;
	UIFrame				m_active_background_frame;
					
	int					m_selected_index;
	int					m_selected_drag_index;
	int					m_selected_drop_index;
	std::vector<Rect2D> m_item_boxes;

	Vector2				m_last_mouse_position;

	Vector2				m_focus_point;

	AtlasFrame*			m_arrow_frame;

	int					m_items_per_page;

	int					m_current_page;
	int					m_total_pages;
	int					m_total_items;

	float				m_page_arrow_offset_timer;
	bool				m_page_arrow_offset_direction;
	float				m_page_arrow_hover_timer;

	float				m_selection_time;
	
	bool				m_draw_item_background;

	float				m_mouse_down_timer;
	bool				m_is_dragging;

	enum
	{
		HOVER_PAGE_CHANGE_DELAY = 300,
		DRAG_DELAY = 100
	};

private:
	void Refresh_Boxes(UIManager* manager);

public:	
	Event<UIGridDrawItemData> On_Draw_Item;
	Event<UIGridCanDragData> On_Can_Drag;

public:
	UIGrid();
	~UIGrid();

	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	void Set_Total_Items(int items);

	void Set_Draw_Item_Background(bool value);

	int Get_Selected_Index();
	int Get_Drop_Selected_Index();
	int Get_Drag_Selected_Index();
	float Get_Selection_Time();
	Rect2D Get_Selected_Item_Box();
	void Drag_Item_Index(int item_index);

	Vector2 Get_Focus_Point();
	
	bool Focus_Up(UIManager* manager);
	bool Focus_Down(UIManager* manager);
	bool Focus_Left(UIManager* manager);
	bool Focus_Right(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

