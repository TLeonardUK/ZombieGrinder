// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIPROPERTYGRID_
#define _ENGINE_UI_ELEMENTS_UIPROPERTYGRID_

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

struct UIPropertyGridDataType
{
	enum Type
	{
		None,
		String,
		Int,
		Float,
		Bool,
		Combo,
		External,
	};
};

struct UIPropertyGridButton
{
	AtlasFrame* Frame;
	int			ID;
};

struct UIPropertyGridItem
{
	typedef void  (*Get_Value_Function)(UIPropertyGridItem* item, void* value);
	typedef void  (*Set_Value_Function)(UIPropertyGridItem* item, void* value);

	std::string							Name;
	UIPropertyGridItem*					Parent;
	UIPropertyGridDataType::Type		Type;
	void*								Value;
	std::vector<std::string>			Combos;
	std::vector<UIPropertyGridButton>	Buttons;
	std::vector<UIPropertyGridItem*>	Children;
	bool								Expanded;
	void*								Meta_Data;
	void*								Meta_Data_2;
	std::string							Meta_Data_3;
	int									ID;

	Get_Value_Function					Get_Function;
	Set_Value_Function					Set_Function;
};

class UIPropertyGrid : public UIElement 
{
	MEMORY_ALLOCATOR(UIPropertyGrid, "UI");

protected:	
	friend class UILayout;

	UIPropertyGridItem				 m_root;
	std::vector<UIPropertyGridItem*> m_nodes;

	AtlasFrame*		m_expand_frame;
	AtlasFrame*		m_collapse_frame;
	AtlasFrame*		m_check_box;
	AtlasFrame*		m_uncheck_box;
	
	UISlider*		m_slider;
	float			m_slider_width;
	float			m_scroll_range;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;
	UIManager*		m_manager;
	FontHandle*		m_font;
	FontRenderer	m_font_renderer;
	
	UITextBox*				m_input_text_box;
	bool					m_opened_input_box;
	UIPropertyGridItem*		m_input_box_node;
	UIFrame					m_icon_background_hover_frame;
	UIFrame					m_icon_background_pressed_frame;
	UIFrame					m_icon_background_seperator_frame;
	UIFrame					m_icon_background_frame;
	
	float m_node_padding;
	float m_node_height;
	float m_node_sub_x_spacing;

private:
	void Draw_Node(const FrameTime& time, UIManager* manager, UIScene* scene, UIPropertyGridItem* item, float& x_offset, float& y_offset);
	void Draw_Node_Children(const FrameTime& time, UIManager* manager, UIScene* scene, UIPropertyGridItem* item, float& x_offset, float& y_offset);

	void Get_Node_Value(UIPropertyGridItem* item, float* val);
	void Get_Node_Value(UIPropertyGridItem* item, int* val);
	void Get_Node_Value(UIPropertyGridItem* item, bool* val);
	void Get_Node_Value(UIPropertyGridItem* item, std::string* val);

	void Set_Node_Value(UIPropertyGridItem* item, float val);
	void Set_Node_Value(UIPropertyGridItem* item, int val);
	void Set_Node_Value(UIPropertyGridItem* item, bool val);
	void Set_Node_Value(UIPropertyGridItem* item, std::string val);

public:
	UIPropertyGrid();
	~UIPropertyGrid();

	void Save_Input_Change(UIManager* manager, UIScene* scene);
	
	void Refresh();
	void After_Refresh();

	void Clear_Items();


	UIPropertyGridItem* Add_Item(
		std::string name, 
		UIPropertyGridItem* parent = NULL, 
		UIPropertyGridDataType::Type = UIPropertyGridDataType::None, 
		void* value = 0, 
		std::vector<std::string> combo_values = std::vector<std::string>(), 
		void* meta = NULL, 
		int id = 0);
	
	UIPropertyGridItem* Add_Custom_Item(
		std::string name, 
		UIPropertyGridItem* parent = NULL, 
		UIPropertyGridDataType::Type = UIPropertyGridDataType::None, 
		UIPropertyGridItem::Get_Value_Function get_func = NULL,
		UIPropertyGridItem::Set_Value_Function set_func = NULL,
		std::vector<std::string> combo_values = std::vector<std::string>(), 
		void* meta = NULL, 
		int id = 0,
		void* meta_2 = NULL,
		std::string meta_3 = ""
		);

	void Add_Item_Button(UIPropertyGridItem* item, std::string icon, int id);

	void Remove_Item(UIPropertyGridItem* item);

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

