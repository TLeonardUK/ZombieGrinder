// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
[
	Name("Achievement"), 
	Description("Base class for all achievements. Instance of all derived classes is created at game start and each will recieve individual game events.") 
]
public native("Achievement") class Achievement
{
	// Gets the identifier of the achievement registered with the 
	// online platform.
	property string ID
	{
		native("Get_ID") string Get();
		native("Set_ID") void Set(string value);		
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

	// Gets the current progress towards this achievement.
	property serialized(1) float Progress
	{
		native("Get_Progress") float Get();
		native("Set_Progress") void Set(float value);
	}
	
	// Gets the maximum progress for this achievement. When progress is >= to
	// this then the achievement unlocks.
	property float Max_Progress
	{
		native("Get_Max_Progress") float Get();
		native("Set_Max_Progress") void Set(float value);
	}

	// Should return the locked and unlocked icon frame.
	property string Locked_Frame
	{
		native("Get_Locked_Frame") string Get();
		native("Set_Locked_Frame") void Set(string value);
	}
	property string Unlocked_Frame
	{
		native("Get_Unlocked_Frame") string Get();
		native("Set_Unlocked_Frame") void Set(string value);
	}

	// If set then this achievement will track a statistic for its state.
	// When statistic > Max_Progress the unlock occurs.
	property Type Track_Statistic
	{
		native("Get_Track_Statistic") Type Get();
		native("Set_Track_Statistic") void Set(Type value);
	}

	// If set to true then Progress will be set the the current value
	// of Track_Statistic.
	property bool Use_Statistic_Progress
	{
		native("Get_Use_Statistic_Progress") bool Get();
		native("Set_Use_Statistic_Progress") void Set(bool value);
	}
	
	// Gets / Sets if this achievement is unlocked. Setting this will not unlock
	// the trophy it will just stop it updating. Set progres >= Max_Progerss to unlock.
	property serialized(1) bool Unlocked
	{
		native("Get_Unlocked") bool Get();
		native("Set_Unlocked") void Set(bool value);
	}
}
 

