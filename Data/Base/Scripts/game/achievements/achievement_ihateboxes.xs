// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_boxes_destroyed;

public class Achievement_IHateBoxes : Achievement
{
	Achievement_IHateBoxes()
	{
		ID 				= "ACHIEVEMENT_IHATEBOXES";
		Name 			= Locale.Get("#achievement_ihateboxes_name");
		Description 	= Locale.Get("#achievement_ihateboxes_description");
		Progress 		= 0;
		Max_Progress 	= 1000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_024_ihateboxes";
		Track_Statistic	= typeof(Statistic_Boxes_Destroyed);
		Use_Statistic_Progress = true;
	}
}