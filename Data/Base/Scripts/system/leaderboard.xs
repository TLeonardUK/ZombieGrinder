// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Leaderboard"), 
	Description("Base class for all leaderboards. Instance of all derived classes is created at game start and each will recieve individual game events.") 
]
public native("Leaderboard") class Leaderboard
{
	// Gets the identifier of the achievement registered with the 
	// online platform.
	property string ID
	{
		native("Get_ID") string Get();
		native("Set_ID") void Set(string value);		
	}
	
	// Gets name - should match platform name.
	property string Name
	{
		native("Get_Name") string Get();
		native("Set_Name") void Set(string value);	
	}
		
	// If true scores are shown ascending.
	property bool Ascending
	{
		native("Get_Ascending") bool Get();
		native("Set_Ascending") void Set(bool value);			
	}
	
	// If true we always store the latest score not the highscore.
	property bool Replace_Score
	{
		native("Get_Replace_Score") bool Get();
		native("Set_Replace_Score") void Set(bool value);			
	}
	 
	// Submit a score the the leaderboard.
	native("Submit") void Submit(int score);
}

