// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
[
	Name("DLC"), 
	Description("Base class for all dlc add-ons. Mainly used for premium accounts etc.") 
]
public native("DLC") class DLC
{
	// Identifier used to register this DLC with first-party.
	property int ID 
	{
		native("Get_ID") int Get();
		native("Set_ID") void Set(int value);		
	}
	
	// Gets name and description - should match platform name/description.
	property string Name
	{
		native("Get_Name") string Get();
		native("Set_Name") void Set(string value);	
	}
	property string Description
	{
		native("Get_Description") string Get();
		native("Set_Description") void Set(string value);		
	}
	
	// Gets if this DLC has been purchased on the first-party market place or not.
	property bool Purchased
	{
		native("Get_Purchased") bool Get();
	}

	// Invoked when a account has this DLC applied to it. Use it to give items etc.
	public virtual void On_Recieve(Profile profile)
	{	
		// Give user items etc.
		Log.Write("DLC.On_Recieve = " + this.Name);
	}
}
