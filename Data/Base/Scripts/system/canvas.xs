  // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Helper class, holds some commonly used colors.
// -----------------------------------------------------------------------------
public static class Color
{
	public static Vec4 White = new Vec4(255, 255, 255, 255);
	public static Vec4 Black = new Vec4(0, 	0, 	 0,   255);
}

// -----------------------------------------------------------------------------
//	Graphics defines. Keep synced with Renderer.h stuff.
// -----------------------------------------------------------------------------
public enum Blend_Mode 
{
	Solid	= 0,
	Alpha	= 1,
	Light	= 2
}
public enum Text_Alignment
{
	Left,
	Right,
	Center,
	Top,
	Bottom,
	Justified,
	ScaleToFit
}

// -----------------------------------------------------------------------------
//	Helper class for rendering to the screen.
// -----------------------------------------------------------------------------
public native("Canvas") class Canvas
{
	// Current canvas state.
	property Vec4 Bounds
	{
		public native("Get_Bounds") Vec4 Get();
		public native("Set_Bounds") void Set(Vec4 value);	
	}
	property Vec4 Display_Bounds
	{
		public native("Get_Display_Bounds") Vec4 Get();
	}
	property Vec4 Viewport
	{	
		public native("Get_Viewport") Vec4 Get();
		public native("Set_Viewport") void Set(Vec4 value);	
	}
	property Vec4 Color 
	{	
		public native("Get_Color") Vec4 Get();
		public native("Set_Color") void Set(Vec4 value);	
	}
	property Vec4 Global_Color 
	{	
		public native("Get_Global_Color") Vec4 Get();
		public native("Set_Global_Color") void Set(Vec4 value);	
	}
	property Vec2 Scale
	{
		public native("Get_Scale") Vec2 Get();
		public native("Set_Scale") void Set(Vec2 value);	
	}
	property float Rotation
	{
		public native("Get_Rotation") float Get();
		public native("Set_Rotation") void Set(float value);	
	}
	property Blend_Mode Blend
	{
		public native("Get_Blend") Blend_Mode Get();
		public native("Set_Blend") void Set(Blend_Mode value);	
	}
	property string Font
	{
		public native("Get_Font") string Get();
		public native("Set_Font") void Set(string value);	
	}
	property bool Font_Shadowed
	{
		public native("Get_Font_Shadowed") bool Get();
		public native("Set_Font_Shadowed") void Set(bool value);	
	}
	property float UI_Scale
	{
		public native("Get_UI_Scale") float Get();
	}
	 
	// Atlas frames.
	public native("Draw_Frame") void Draw_Frame(string frame_name, Vec2 point, bool flip_h = false, bool flip_v = false);
	public native("Draw_Animation") void Draw_Animation(string anim_name, Vec4 bounds, bool flip_h = false, bool flip_v = false);
	public native("Draw_Frame_Rect") void Draw_Frame(string frame_name, Vec4 bounds, bool flip_h = false, bool flip_v = false);
	public native("Draw_Frame_Box") void Draw_Frame_Box(string frame_name, Vec4 bounds, float scale = 1.0f);
	
	// Primitives.
	public native("Draw_Rect") void Draw_Rect(Vec4 bounds);
	public native("Draw_Hollow_Rect") void Draw_Hollow_Rect(Vec4 bounds, float line_width = 1.0);
	public native("Draw_Oval") void Draw_Oval(Vec4 bounds);
	public native("Draw_Line") void Draw_Line(Vec2 startp, Vec2 endp, float line_width = 1.0);
	
	// Text
	public native("Draw_Text") void Draw_Text(string text, Vec4 bounds, float font_height, Text_Alignment h_align = Text_Alignment.Left, Text_Alignment v_align = Text_Alignment.Top);
	public native("Draw_Text_Point") void Draw_Text(string text, Vec2 point, float font_height);
	public native("Word_Wrap") string Word_Wrap(string text, Vec4 bounds, float font_height);
	public native("Calculate_Text_Size") Vec2 Calculate_Text_Size(string text, float font_height);
	
	// Markup Text
	public native("Draw_Markup_Text") void Draw_Markup_Text(string text, Vec4 bounds, float font_height, Text_Alignment h_align = Text_Alignment.Left, Text_Alignment v_align = Text_Alignment.Top);
	public native("Draw_Markup_Text_Point") void Draw_Markup_Text(string text, Vec2 point, float font_height);
	public native("Word_Wrap_Markup") string Word_Wrap_Markup(string text, Vec4 bounds, float font_height);
	public native("Calculate_Markup_Text_Size") Vec2 Calculate_Markup_Text_Size(string text, float font_height);
	
	public native("Escape_Markup") string Escape_Markup(string text);

	// Projection.
	public native("World_BBox_To_Screen") Vec4 World_BBox_To_Screen(Vec4 bbox);
	public native("Screen_BBox_To_World") Vec4 Screen_BBox_To_World(Vec4 bbox);

	// Static information.
	public static native("Get_Frame_Size") Vec2 Get_Frame_Size(string frame);
	public static native("Get_Animation_Frame_Count") int Get_Animation_Frame_Count(string anim_name);
	public static native("Get_Animation_Frame") string Get_Animation_Frame(string anim_name, int frame);
}
