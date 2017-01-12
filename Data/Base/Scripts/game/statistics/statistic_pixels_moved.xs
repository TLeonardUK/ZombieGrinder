// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Pixels_Moved : Statistic
{
	Statistic_Pixels_Moved()
	{
		ID 					= "PIXELS_MOVED";
		Name 				= Locale.Get("#statistic_pixels_moved_name");
		Description 		= Locale.Get("#statistic_pixels_moved_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Tick()
	{		
		NetUser[] users = Network.Get_Local_Users();	
		foreach (NetUser user in users)
		{			
			Controller controller = user.Controller;
			if (controller != null)
			{
				Pawn pawn = controller.Possessed;
				if (pawn != null)
				{
					Value += pawn.Get_Last_Frame_Movement().Length();
				}
			}
		}
	}
}
