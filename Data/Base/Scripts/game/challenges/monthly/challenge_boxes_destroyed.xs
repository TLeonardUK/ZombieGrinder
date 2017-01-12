// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Boxes_Destroyed : Challenge
{
    Challenge_Boxes_Destroyed()
    {
        ID = "CHALLENGE_BOXES_DESTROYED";
        Name = "#menu_game_setup_challenge_monthly";
        Description = "#challenge_boxes_destroyed_description";
        Timeframe = ChallengeTimeframe.Monthly;
        Requires_Activation = false;
        Max_Progress = 100000.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int Max_Total = <int>Max_Progress / 500; 
        return Get_Standard_Rewards(Max_Total, Max_Total);
    }

    public event void Seeded_Setup()
    {
		int Max = Math.Rand(2, 4);
		Max_Progress = 500 * Max;
        Description = Locale.Get("#challenge_boxes_destroyed_description").Format(new object[] { <object>(<int>Max_Progress) });
	}

	public event void On_Movable_Destroyed(Event_Movable_Destroyed data)
	{
		Progress += 1.0f;
	}
}
