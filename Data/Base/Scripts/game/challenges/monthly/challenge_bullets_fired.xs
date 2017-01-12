// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Bullets_Fired : Challenge
{
    Challenge_Bullets_Fired()
    {
        ID = "CHALLENGE_BULLETS_FIRED";
        Name = "#menu_game_setup_challenge_monthly";
        Description = "#challenge_bullets_fired_description";
        Timeframe = ChallengeTimeframe.Monthly;
        Requires_Activation = false;
        Max_Progress = 100000.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int Max_Total = <int>Max_Progress / 10000; 
        return Get_Standard_Rewards(Max_Total, Max_Total);
    }

    public event void Seeded_Setup()
    {
		int Max = Math.Rand(2, 4);
		Max_Progress = 10000 * Max;
        Description = Locale.Get("#challenge_bullets_fired_description").Format(new object[] { <object>(<int>Max_Progress) });
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
					Progress += pawn.Get_Shots_Fired_Delta();
				}
			}
		}
	}
}
