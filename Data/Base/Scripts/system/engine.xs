// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Engine stuff
// -----------------------------------------------------------------------------
public static native("Engine") class Engine
{
	// Returns true if crash sentinel was activated.
	public static native("Has_Crashed") bool Has_Crashed();
	// Returns true if user has uploaded something to the workshop this game.
	public static native("Has_Uploaded_To_Workshop") bool Has_Uploaded_To_Workshop();
    // Returns true if in restricted mode.
    public static native("In_Map_Restricted_Mode") bool In_Map_Restricted_Mode();
    // Returns true if in temporary map restricted mode (unapproved map).
    public static native("In_Restricted_Mode") bool In_Restricted_Mode();
    // Returns the highest profile level any local user has.
    public static native("Get_Highest_Profile_Level") int Get_Highest_Profile_Level();
}

 