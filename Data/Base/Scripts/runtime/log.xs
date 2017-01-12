// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	This class is used to read/write from the standard output console.
// -----------------------------------------------------------------------------
public static native("Log") class Log
{
	public static native("Write") 			void Write		    (string output);
	public static native("WriteOnScreen") 	void WriteOnScreen	(string id, Vec4 color, float lifetime, string output);
	public static native("Assert")			void Assert		    (bool value);
	public static native("Assert_Message")	void Assert	    	(bool value, string msg);
}
