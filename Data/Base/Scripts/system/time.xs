// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Time stuff
// -----------------------------------------------------------------------------
public static native("Time") class Time
{
	// Elapsed time since last frame.
	public static native("Get_Frame_Time") float Get_Frame_Time();
	
	// Delta for target speed.
	public static native("Get_Delta") float Get_Delta();
	
	// Frame_Time / 1000.0f
	public static native("Get_Delta_Seconds") float Get_Delta_Seconds();

	// Gets milliseconds since game started.
	public static native("Get_Ticks") float Get_Ticks();
		
	// Gets milliseconds since map started.
	public static native("Get_Map_Time") float Get_Map_Time();
		
	// Puts thread of execution to sleep for give number of seconds.
	public static native("Sleep") void Sleep(float seconds);
}

 