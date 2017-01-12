// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RUNTIME_CANVAS_
#define _ENGINE_RUNTIME_CANVAS_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Canvas
{
public:
	static CVMObjectHandle Get_Bounds(CVirtualMachine* vm, CVMValue self);
	static void Set_Bounds(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Display_Bounds(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Viewport(CVirtualMachine* vm, CVMValue self);
	static void Set_Viewport(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Color(CVirtualMachine* vm, CVMValue self);
	static void Set_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Global_Color(CVirtualMachine* vm, CVMValue self);
	static void Set_Global_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Scale(CVirtualMachine* vm, CVMValue self);
	static void Set_Scale(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static float Get_UI_Scale(CVirtualMachine* vm, CVMValue self);
	static float Get_Rotation(CVirtualMachine* vm, CVMValue self);
	static void Set_Rotation(CVirtualMachine* vm, CVMValue self, float value);
	static int Get_Blend(CVirtualMachine* vm, CVMValue self);
	static void Set_Blend(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Font_Shadowed(CVirtualMachine* vm, CVMValue self);
	static void Set_Font_Shadowed(CVirtualMachine* vm, CVMValue self, int value);
	static CVMString Get_Font(CVirtualMachine* vm, CVMValue self);
	static void Set_Font(CVirtualMachine* vm, CVMValue self, CVMString value);
	static void Draw_Frame(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle point, int flip_h, int flip_v);
	static void Draw_Animation(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle point, int flip_h, int flip_v);
	static void Draw_Frame_Rect(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle bounds, int flip_h, int flip_v);
	static void Draw_Frame_Box(CVirtualMachine* vm, CVMValue self, CVMString frame_name, CVMObjectHandle bounds, float scale);
	static void Draw_Rect(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bounds);
	static void Draw_Hollow_Rect(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bounds, float line_width);
	static void Draw_Oval(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bounds);
	static void Draw_Line(CVirtualMachine* vm, CVMValue self, CVMObjectHandle startp, CVMObjectHandle endp, float line_width);
	static void Draw_Text(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height, int h_align, int v_align);
	static void Draw_Text_Point(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle point, float font_height);
	static void Draw_Markup_Text(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height, int h_align, int v_align);
	static void Draw_Markup_Text_Point(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle point, float font_height);
	static CVMString Word_Wrap(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height);
	static CVMObjectHandle Calculate_Text_Size(CVirtualMachine* vm, CVMValue self, CVMString text, float font_height);
	static CVMString Word_Wrap_Markup(CVirtualMachine* vm, CVMValue self, CVMString text, CVMObjectHandle bounds, float font_height);
	static CVMObjectHandle Calculate_Markup_Text_Size(CVirtualMachine* vm, CVMValue self, CVMString text, float font_height);
	static CVMString Escape_Markup(CVirtualMachine* vm, CVMValue self, CVMString text);

	static CVMObjectHandle World_BBox_To_Screen(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bbox);
	static CVMObjectHandle Screen_BBox_To_World(CVirtualMachine* vm, CVMValue self, CVMObjectHandle bbox);

	static CVMObjectHandle Get_Frame_Size(CVirtualMachine* vm, CVMString frame);
	static int Get_Animation_Frame_Count(CVirtualMachine* vm, CVMString anim_name);
	static CVMString Get_Animation_Frame(CVirtualMachine* vm, CVMString anim_name, int frame);

	static void Bind(CVirtualMachine* machine);
};

#endif
