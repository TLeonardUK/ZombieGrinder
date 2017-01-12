// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef EVENT
#error "EVENT must be defined when including this file!"
#endif

// General
EVENT(Element_Focus)

// Buttons
EVENT(Button_Click)

// Checkboxs
EVENT(CheckBox_Click)

// Menus
EVENT(MenuItem_Click)

// Toolbars
EVENT(ToolbarItem_Click)
EVENT(ToolbarItem_RightClick)

// ComboBoxs
EVENT(ComboBox_SelectedIndexChanged)

// ListViews
EVENT(ListView_Item_Click)

// TextBox
EVENT(TextBox_Enter)
EVENT(TextBox_JoystickSelect)

// Dialog closes.
EVENT(Dialog_Close)

// Grid
EVENT(Grid_Item_Click)
EVENT(Grid_Begin_Drag)
EVENT(Grid_Finish_Drag)

// GridTree
EVENT(GridTree_Item_Click)
EVENT(GridTree_Drop)

// PropertyGrid
EVENT(PropertyGrid_Changed)
EVENT(PropertyGrid_ButtonClicked)
EVENT(PropertyGrid_ExternalClicked)
EVENT(PropertyGrid_BeginEdit)

// CurveEditor
EVENT(CurveEditor_Modified)

// FileBrowser
EVENT(FileBrowser_Select)

// ResourceBrowser
EVENT(ResourceBrowser_Select)