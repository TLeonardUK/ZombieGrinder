// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Direct access to the display, rendering/etc.
// -----------------------------------------------------------------------------
public native("Display") class Display
{
	public static native("Get_Resolution") Vec2 Get_Resolution();
	public static native("Get_Game_Resolution") Vec2 Get_Game_Resolution();
	public static native("Get_Aspect_Ratio") float Get_Aspect_Ratio();
}
