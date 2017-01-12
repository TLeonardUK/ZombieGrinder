// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UISIMPLELISTVIEW_
#define _ENGINE_UI_ELEMENTS_UISIMPLELISTVIEW_

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

class AtlasHandle;
class UIManager;
struct AtlasFrame;

struct UISimpleListViewItem
{
	MEMORY_ALLOCATOR(UISimpleListViewItem, "UI");

public:
	std::vector<std::string> Values;

	void*		MetaData;
};

struct UISimpleListViewColumn
{
	MEMORY_ALLOCATOR(UISimpleListViewColumn, "UI");

public:
	std::string Text;
	float		Width;
	bool		LimitViewport;
	bool		AcceptMarkup;

};

class UISimpleListView : public UIElement 
{
	MEMORY_ALLOCATOR(UISimpleListView, "UI");

protected:	
	friend class UILayout;

	AtlasHandle*					m_atlas;
	AtlasRenderer					m_atlas_renderer;
		
	FontHandle*						m_font;
	FontRenderer					m_font_renderer;
	MarkupFontRenderer				m_markup_font_renderer;

	UIFrame							m_background_frame;

	Color							m_frame_color;

	std::vector<UISimpleListViewItem>	m_items;
	std::vector<UISimpleListViewColumn>	m_columns;

	Rect2D							m_padded_box;
	Rect2D							m_header_box;
	Rect2D							m_item_box;

public:
	UISimpleListView();
	~UISimpleListView();

	void RefreshBoxes(UIManager* manager);
	void Refresh();
	void After_Refresh();
	bool Is_Focusable();

	void Add_Item(UISimpleListViewItem item);
	void Add_Item(const char* text, void* data = NULL);
	void Add_Item(std::vector<std::string> values, void* data = NULL);
	void Clear_Items();

	void Add_Column(UISimpleListViewColumn column);
	void Add_Column(const char* text, float width, bool limit_viewport = true, bool accept_markup = false);
	void Clear_Columns();

	const UISimpleListViewItem& Get_Selected_Item();
	int Get_Selected_Item_Index();
	void Set_Selected_Item_Index(int index);
	Rect2D Get_Selected_Item_Rectangle();

	const std::vector<UISimpleListViewItem>& Get_Items();
	const std::vector<UISimpleListViewColumn>& Get_Columns();
	
	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

