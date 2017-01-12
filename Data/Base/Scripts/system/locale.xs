// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Language manipulation!
// -----------------------------------------------------------------------------
public static native("Locale") class Locale
{
	// Gets a localized string with the given ID.
	public static native("Get_String") string Get(string id);

	// Changes the current language.
	public static native("Change_Language") void Change_Language(string id);
}
