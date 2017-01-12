// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UILISTVIEW_
#define _ENGINE_UI_ELEMENTS_UILISTVIEW_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Renderer/Text/MarkupFontRenderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Input/OutputBindings.h"

#include "Generic/Events/Event.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;
class UIListView;

struct UIListViewItem
{
	MEMORY_ALLOCATOR(UIListViewItem, "UI");

public:
	std::vector<std::string> Values;

	void*		MetaData;
};

struct UIListViewColumn
{
	MEMORY_ALLOCATOR(UIListViewColumn, "UI");

public:
	std::string Text;
	float		Width;
	bool		LimitViewport;
	bool		AcceptMarkup;

};

struct UIListViewDrawItemData
{
public:
	UIListView*		view;
	Rect2D			view_bounds;
	Rect2D			item_bounds;
	FrameTime		time;
	UIManager*		manager;
	UIScene*		scene;
	UIListViewItem*	item;
	Vector2			ui_scale;
	bool			selected;
	bool			hovering;
	int				item_index;
};

struct UIListViewGetItemData
{
public:
	UIListView*		view;
	UIListViewItem*	item;
	int				index;
};

class UIListView : public UIElement 
{
	MEMORY_ALLOCATOR(UIListView, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*					m_atlas;
	AtlasRenderer					m_atlas_renderer;
		
	FontHandle*						m_font;
	FontRenderer					m_font_renderer;
	MarkupFontRenderer				m_markup_font_renderer;

	UIFrame							m_background_frame;

	Color							m_frame_color;

	std::vector<UIListViewItem>		m_items;
	std::vector<UIListViewColumn>	m_columns;
	int								m_selected_item_index;
	int								m_hover_item_index;
	Rect2D							m_selected_item_rect;

	UISlider*						m_slider;
	
	Rect2D							m_padded_box;
	Rect2D							m_slider_box;
	Rect2D							m_header_box;
	Rect2D							m_item_box;

	bool							m_selection_just_changed;

	int								m_last_item_count;

	Vector2							m_focus_point;

	bool							m_no_header;
	bool							m_simplified;

	float							m_item_height;
	float							m_item_spacing;

	int								m_fake_item_count;
	
	bool							m_center_selection;

public:
	Event<UIListViewDrawItemData>	On_Draw_Item;
	Event<UIListViewGetItemData>	On_Get_Item_Data;

public:
	UIListView();
	~UIListView();

	void RefreshBoxes(UIManager* manager);
	void Refresh();
	void After_Refresh();
	bool Is_Focusable();
	
	UISlider* Get_Slider() ;

	Vector2 Get_Focus_Point();

	bool Is_Scrolling();

	int Get_Item_Count();

	float Get_Item_Height();
	void Set_Item_Height(float val);

	void Set_Faked_Item_Count(int count);
	int  Get_Scroll_Offset();
	void Set_Scroll_Offset(float offset);
	void Scroll_To(int offset);

	void Add_Item(UIListViewItem item);
	void Add_Item(const char* text, void* data = NULL);
	void Add_Item(std::vector<std::string> values, void* data = NULL);
	void Clear_Items();

	void Add_Column(UIListViewColumn column);
	void Add_Column(const char* text, float width, bool limit_viewport = true, bool accept_markup = false);
	void Clear_Columns();

	const UIListViewItem& Get_Selected_Item();
	int Get_Selected_Item_Index();
	void Set_Selected_Item_Index(int index);
	Rect2D Get_Selected_Item_Rectangle();

	const std::vector<UIListViewItem>& Get_Items();
	std::vector<UIListViewColumn>& Get_Columns();
	
	bool Focus_Up(UIManager* manager);
	bool Focus_Down(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

