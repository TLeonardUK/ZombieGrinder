// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Eeehhhhhhhhhhhhhhh, this whole file is pretty dam fugly. Need to figure
// out a better method of doing this while restricting manual work.

#ifndef ELEMENT_START
#error "ELEMENT_START must be defined when including this file!"
#endif
#ifndef ELEMENT_END
#error "ELEMENT_END must be defined when including this file!"
#endif
#ifndef ELEMENT_END
#error "ELEMENT_PROPERTY must be defined when including this file!"
#endif
#ifndef ELEMENT_PROPERTY_ENUM_START
#error "ELEMENT_PROPERTY_ENUM_START must be defined when including this file!"
#endif
#ifndef ELEMENT_PROPERTY_ENUM_END
#error "ELEMENT_PROPERTY_ENUM_END must be defined when including this file!"
#endif
#ifndef ELEMENT_ENUM_ENTRY
#error "ELEMENT_ENUM_ENTRY must be defined when including this file!"
#endif

#ifndef ELEMENT_ALLOW_CHILD
#error "ELEMENT_ALLOW_CHILD must be defined when including this file!"
#endif
#ifndef ELEMENT_ALLOW_ANY_CHILD
#error "ELEMENT_ALLOW_ANY_CHILD must be defined when including this file!"
#endif
#ifndef ELEMENT_FORCE_MAX_CHILDREN
#error "ELEMENT_FORCE_MAX_CHILDREN must be defined when including this file!"
#endif
#ifndef ELEMENT_MAX_CHILDREN
#error "ELEMENT_MAX_CHILDREN must be defined when including this file!"
#endif

// Define Definitions:
//
// ELEMENT_START(element-name, element-class-name)
//	ELEMENT_PROPERTY(property-name, property-field-name, property-type, is-required)
// ELEMENT_END()
//
// These define all the different properties of elements that can be
// loaded and saved from layout files.

// ===================================================================
// Menu Elements
// ===================================================================
#include "Engine/UI/Elements/UIMenuBar.h"
#include "Engine/UI/Elements/UIMenuItem.h"

ELEMENT_START(MenuBar, UIMenuBar)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY(Box,			m_box,			Rect2D,			true);
	ELEMENT_PROPERTY(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(ItemPadding,	m_item_padding,	Rect2D,			false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_CHILD				(UIMenuItem);
	ELEMENT_FORCE_MAX_CHILDREN		(false);
	ELEMENT_MAX_CHILDREN			(-1);
ELEMENT_END()

ELEMENT_START(MenuItem, UIMenuItem)
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(Value,			m_value,		std::string,	true);
	ELEMENT_PROPERTY(Icon,			m_icon_name,	std::string,	false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_CHILD				(UIMenuItem);
	ELEMENT_FORCE_MAX_CHILDREN		(false);
	ELEMENT_MAX_CHILDREN			(-1);
ELEMENT_END()

// ===================================================================
// Text Elements
// ===================================================================
#include "Engine/UI/Elements/UILabel.h"

ELEMENT_START(Label, UILabel)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY 			(Value,			m_value,			std::string,			true);
	ELEMENT_PROPERTY			(Padding,		m_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(ForeColor,		m_color,			Color,					false);
	ELEMENT_PROPERTY			(Frame,			m_frame_name,		std::string,			false);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY			(UseMarkup,		m_use_markup,		bool,					false);
	ELEMENT_PROPERTY			(WordWrap,		m_word_wrap,		bool,					false);	
	ELEMENT_PROPERTY			(Scale,			m_scale,		Vector2,			false);
	ELEMENT_PROPERTY			(GameStyle,		m_game_style,		bool,					false);
	
	ELEMENT_PROPERTY_ENUM_START	(HAlign,		m_halign,			TextAlignment,			false);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Left);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Right);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Center);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Justified);
	ELEMENT_PROPERTY_ENUM_END	();	
	
	ELEMENT_PROPERTY_ENUM_START	(VAlign,		m_valign,			TextAlignment,			false);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Top);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Bottom);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Center);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Justified);
		ELEMENT_ENUM_ENTRY		(TextAlignment, ScaleToFit);
	ELEMENT_PROPERTY_ENUM_END	();	

	ELEMENT_MAX_CHILDREN		(0);
	ELEMENT_FORCE_MAX_CHILDREN	(false);
ELEMENT_END()

// ===================================================================
// Container Elements
// ===================================================================
#include "Engine/UI/Elements/UISplitContainer.h"
#include "Engine/UI/Elements/UISplitContainerItem.h"

ELEMENT_START(SplitContainer, UISplitContainer)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Resizable,		m_resizable,		bool,					true);
	ELEMENT_PROPERTY			(Scale,			m_scale,			Vector2,					false);
	ELEMENT_PROPERTY			(Offset,		m_split_offset,		float,					false);

	
	ELEMENT_PROPERTY_ENUM_START	(Direction,		m_direction,		SplitContainerDirection,true);
		ELEMENT_ENUM_ENTRY		(SplitContainerDirection, Horizontal);
		ELEMENT_ENUM_ENTRY		(SplitContainerDirection, Vertical);
	ELEMENT_PROPERTY_ENUM_END	();	
	
	ELEMENT_ALLOW_CHILD			(UISplitContainerItem);
	ELEMENT_FORCE_MAX_CHILDREN	(true);
	ELEMENT_MAX_CHILDREN		(2);
ELEMENT_END()

ELEMENT_START(SplitContainerItem, UISplitContainerItem)
	ELEMENT_PROPERTY			(Size,			m_size,				float,					false);
	ELEMENT_PROPERTY			(Fixed,			m_fixed,			bool,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Panel Elements
// ===================================================================
#include "Engine/UI/Elements/UIPanel.h"

ELEMENT_START(Panel, UIPanel)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(BackColor,		m_backcolor,		Color,					false);
	ELEMENT_PROPERTY			(BackImage,		m_backimage_name,	std::string,			false);
	ELEMENT_PROPERTY			(ForeImage,		m_foreimage_name,	std::string,			false);
	ELEMENT_PROPERTY			(Frame,			m_frame_name,		std::string,			false);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY			(Scale,			m_scale,		Vector2,			false);
	ELEMENT_PROPERTY			(FlatBottom,	m_flat_bottom,		bool,					false);
	
	ELEMENT_PROPERTY_ENUM_START	(BGScaleMode,		m_bg_scale_mode,	UIPanelBGScaleMode,false);
		ELEMENT_ENUM_ENTRY		(UIPanelBGScaleMode, Scale);
		ELEMENT_ENUM_ENTRY		(UIPanelBGScaleMode, Zoom);
		ELEMENT_ENUM_ENTRY		(UIPanelBGScaleMode, Center);
		ELEMENT_ENUM_ENTRY		(UIPanelBGScaleMode, FullWidth);
		ELEMENT_ENUM_ENTRY		(UIPanelBGScaleMode, FullHeight);
	ELEMENT_PROPERTY_ENUM_END	();		


	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
	
// ===================================================================
// PropertyGrid Elements
// ===================================================================
#include "Engine/UI/Elements/UIPropertyGrid.h"

ELEMENT_START(PropertyGrid, UIPropertyGrid)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
		
// ===================================================================
// CurveEditor Elements
// ===================================================================
#include "Engine/UI/Elements/UICurveEditor.h"

ELEMENT_START(CurveEditor, UICurveEditor)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Progress Elements
// ===================================================================
#include "Engine/UI/Elements/UIProgressBar.h"

ELEMENT_START(ProgressBar, UIProgressBar)
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Progress,		m_progress,			float,					false);
	ELEMENT_PROPERTY			(PendingProgress,		m_pending_progress,			float,					false);
	ELEMENT_PROPERTY			(ForeColor,		m_fore_color,			Color,					false);
	ELEMENT_PROPERTY			(BackColor,		m_bg_color,			Color,					false);
	ELEMENT_PROPERTY			(ShowProgress,	m_show_progress,	bool,					false);
	ELEMENT_PROPERTY			(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Button Elements
// ===================================================================
#include "Engine/UI/Elements/UIButton.h"

ELEMENT_START(Button, UIButton)
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY 			(Value,			m_value,			std::string,			true);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY			(Scale,			m_scale,		Vector2,			false);
	ELEMENT_PROPERTY			(UseMarkup,		m_use_markup,		bool,					false);
	
	ELEMENT_PROPERTY			(Padding,		m_padding,			Rect2D,					false);	
	ELEMENT_PROPERTY			(NoFrame,		m_no_frame,			bool,					false);
	ELEMENT_PROPERTY			(GameStyle,		m_game_style,		bool,					false);
	ELEMENT_PROPERTY			(FastAnim,		m_fast_anim,		bool,					false);

	ELEMENT_PROPERTY_ENUM_START	(HAlign,		m_halign,			TextAlignment,			false);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Left);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Right);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Center);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Justified);
	ELEMENT_PROPERTY_ENUM_END	();	
	
	ELEMENT_PROPERTY_ENUM_START	(VAlign,		m_valign,			TextAlignment,			false);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Top);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Bottom);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Center);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Justified);
		ELEMENT_ENUM_ENTRY		(TextAlignment, ScaleToFit);
	ELEMENT_PROPERTY_ENUM_END	();	

	ELEMENT_PROPERTY_ENUM_START	(HotKey,		m_hotkey,			OutputBindings,			false);
		ELEMENT_ENUM_ENTRY		(OutputBindings, GUI_Select);
		ELEMENT_ENUM_ENTRY		(OutputBindings, GUI_Back);
	ELEMENT_PROPERTY_ENUM_END	();		

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Toolbar Elements
// ===================================================================
#include "Engine/UI/Elements/UIToolbar.h"
#include "Engine/UI/Elements/UIToolbarItem.h"

ELEMENT_START(Toolbar, UIToolbar)
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY(Box,			m_box,			Rect2D,			true);
	ELEMENT_PROPERTY(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(ItemPadding,	m_item_padding,	Rect2D,			false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_CHILD				(UIToolbarItem);
	ELEMENT_FORCE_MAX_CHILDREN		(false);
	ELEMENT_MAX_CHILDREN			(-1);
ELEMENT_END()

ELEMENT_START(ToolbarItem, UIToolbarItem)
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(Icon,			m_icon_name,	std::string,	true);
	ELEMENT_PROPERTY(Value,			m_value,		std::string,			false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_FORCE_MAX_CHILDREN		(true);
	ELEMENT_MAX_CHILDREN			(0);
ELEMENT_END()

// ===================================================================
// Checkbox Elements
// ===================================================================
#include "Engine/UI/Elements/UICheckBox.h"

ELEMENT_START(CheckBox, UICheckBox)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(Enabled,		m_enabled,	bool,					false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY 			(Value,			m_value,			std::string,			true);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Slider Elements
// ===================================================================
#include "Engine/UI/Elements/UISlider.h"

ELEMENT_START(Slider, UISlider)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Progress,		m_progress,			float,					false);
	ELEMENT_PROPERTY			(ForeColor,		m_color,			Color,					false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// ComboBox Elements
// ===================================================================
#include "Engine/UI/Elements/UIComboBox.h"

ELEMENT_START(ComboBox, UIComboBox)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY			(Scale,			m_scale,		Vector2,			false);
	ELEMENT_PROPERTY			(GameStyle,		m_game_style,			bool,					false);
	ELEMENT_PROPERTY			(UseMarkup,		m_use_markup,		bool,					false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// ListView Elements
// ===================================================================
#include "Engine/UI/Elements/UIListView.h"

ELEMENT_START(ListView, UIListView)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,					Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,					std::string,			true);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,			Color,					false);
	ELEMENT_PROPERTY			(NoHeader,		m_no_header,			bool,					false);
	ELEMENT_PROPERTY			(Simplified,	m_simplified,			bool,					false);
	ELEMENT_PROPERTY			(ItemHeight,	m_item_height,			float,					false);
	ELEMENT_PROPERTY			(ItemSpacing,	m_item_spacing,			float,					false);
	ELEMENT_PROPERTY			(Scale,			m_scale,				Vector2,			false);
	ELEMENT_PROPERTY			(CenterSelection,	m_center_selection,			bool,					false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// SimpleListView Elements
// ===================================================================
#include "Engine/UI/Elements/UISimpleListView.h"			

ELEMENT_START(SimpleListView, UISimpleListView)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// TextPanel Elements
// ===================================================================
#include "Engine/UI/Elements/UITextPanel.h"

ELEMENT_START(TextPanel, UITextPanel)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);
	ELEMENT_PROPERTY			(ShadowedText,	m_shadowed_text,		bool,					false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// TextBox Elements
// ===================================================================
#include "Engine/UI/Elements/UITextBox.h"

ELEMENT_START(TextBox, UITextBox)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(LeftElement,	m_left_element_name,		std::string, false);
	ELEMENT_PROPERTY			(RightElement,	m_right_element_name,		std::string, false);
	ELEMENT_PROPERTY			(UpElement,		m_up_element_name,			std::string, false);
	ELEMENT_PROPERTY			(DownElement,	m_down_element_name,		std::string, false);

	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY 			(Value,			m_value,			std::string,			false);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);
	ELEMENT_PROPERTY			(AlwaysActive,	m_always_active,	bool,					false);
	
	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// DropDownMenu Elements
// ===================================================================
#include "Engine/UI/Elements/UIDropDownMenu.h"

ELEMENT_START(DropDownMenu, UIDropDownMenu)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,			Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(FrameColor,	m_frame_color,		Color,					false);
	ELEMENT_PROPERTY(Scale,			m_scale,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// UIGrid Elements
// ===================================================================
#include "Engine/UI/Elements/UIGrid.h"

ELEMENT_START(Grid, UIGrid)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Scale,			m_scale,			Vector2,			false);
	ELEMENT_PROPERTY			(Dimensions,	m_dimensions,		Vector2,			false);
	ELEMENT_PROPERTY			(GridSpacing,	m_grid_spacing,		Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
	
// ===================================================================
// UIGridTree Elements
// ===================================================================
#include "Engine/UI/Elements/UIGridTree.h"

ELEMENT_START(GridTree, UIGridTree)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Scale,			m_scale,			Vector2,				false);
	ELEMENT_PROPERTY			(NodeSize,		m_node_size,		Vector2,				false);
	ELEMENT_PROPERTY			(AutoLayout,	m_auto_layout,		bool,					false);
	ELEMENT_PROPERTY			(DrawNodeBg,	m_draw_node_bg,		bool,					false);	
	ELEMENT_PROPERTY			(SelectedFrameName,			m_selected_frame_name,				std::string,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
		
// ===================================================================
// UIFileBrowser Elements
// ===================================================================
#include "Engine/UI/Elements/UIFileBrowser.h"

ELEMENT_START(FileBrowser, UIFileBrowser)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Scale,			m_scale,			Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
			
// ===================================================================
// UIResourceBrowser Elements
// ===================================================================
#include "Engine/UI/Elements/UIResourceBrowser.h"

ELEMENT_START(ResourceBrowser, UIResourceBrowser)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Scale,			m_scale,			Vector2,			false);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
			

// ===================================================================
// UITreeView Elements
// ===================================================================
#include "Engine/UI/Elements/UITreeView.h"

ELEMENT_START(TreeView, UITreeView)
	ELEMENT_PROPERTY			(AspectScaling,	m_aspect_scaling,	bool,					false);
	ELEMENT_PROPERTY			(BoxPadding,	m_box_padding,		Rect2D,					false);
	ELEMENT_PROPERTY			(Box,			m_box,				Rect2D,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Scale,			m_scale,			Vector2,				false);
		
	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()
