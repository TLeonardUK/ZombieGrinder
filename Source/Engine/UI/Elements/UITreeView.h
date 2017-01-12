// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UITREEVIEW_
#define _ENGINE_UI_ELEMENTS_UITREEVIEW_

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UITextBox.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Generic/Types/Variant.h"

class AtlasHandle;
class UIManager;
struct AtlasFrame;
class UIToolbarItem;

struct UITreeViewItem
{
	std::string							Name;
	UITreeViewItem*						Parent;
	std::vector<UITreeViewItem*>		Children;
	bool								Expanded;
	bool								Is_Selectable;
	void*								Meta_Data;
};

class UITreeView : public UIElement 
{
	MEMORY_ALLOCATOR(UITreeView, "UI");

protected:	
	friend class UILayout;

	UITreeViewItem				 m_root;
	std::vector<UITreeViewItem*> m_nodes;

	AtlasFrame*		m_expand_frame;
	AtlasFrame*		m_collapse_frame;
	AtlasFrame*		m_check_box;
	AtlasFrame*		m_uncheck_box;
	
	UIFrame			m_unselect_box;
	UIFrame			m_select_box;

	UITreeViewItem*	m_selected_item;
	
	UISlider*		m_slider;
	float			m_slider_width;
	float			m_scroll_range;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;
	UIManager*		m_manager;
	FontHandle*		m_font;
	FontRenderer	m_font_renderer;
	
	float			m_node_padding;
	float			m_node_height;
	float			m_node_sub_x_spacing;

private:
	void Draw_Node(const FrameTime& time, UIManager* manager, UIScene* scene, UITreeViewItem* item, float& x_offset, float& y_offset);
	void Draw_Node_Children(const FrameTime& time, UIManager* manager, UIScene* scene, UITreeViewItem* item, float& x_offset, float& y_offset);

public:
	UITreeView();
	~UITreeView();

	void Refresh();
	void After_Refresh();

	UITreeViewItem* Get_Selected_Item()
	{
		return m_selected_item;
	}

	void Clear_Items();
	UITreeViewItem* Add_Item(std::string name, UITreeViewItem* parent = NULL, bool selectable = false, void* meta = NULL);
	
	void Remove_Item(UITreeViewItem* item);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

