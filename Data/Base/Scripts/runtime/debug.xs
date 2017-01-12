// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	This class is used to expose debugging functionality.
// -----------------------------------------------------------------------------
public static native("Debug") class Debug
{
	public static native("Error") 	  void 				Error(string message);
	public static native("Break") 	  void 				Break();
	public static native("Assert")    void 				Assert(bool result);
	public static native("AssertMsg") void 				Assert(bool result, string message);
}
