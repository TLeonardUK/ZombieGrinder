// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_PVP_Top_Place : Challenge
{
    Challenge_PVP_Top_Place()
    {
        ID = "CHALLENGE_PVP_TOP_PLACE";
        Name = "#menu_game_setup_challenge_monthly";
        Description = "#challenge_pvp_top_place_description";
        Timeframe = ChallengeTimeframe.Weekly;
        Requires_Activation = false;
        Max_Progress = 100000.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int Max_Total = <int>Max_Progress / 10; 
        return Get_Standard_Rewards(Max_Total, Max_Total);
    }

    public event void Seeded_Setup()
    {
		int Max = Math.Rand(10, 30);
		Max_Progress = Max;
        Description = Locale.Get("#challenge_pvp_top_place_description").Format(new object[] { <object>(<int>Max_Progress) });
	}

    event void On_Game_Over()
    {
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		if (mode && mode.Is_PVP())
        {
            int rank = mode.Get_Scoreboard_Rank(Network.Get_Primary_Local_User());
            if (rank == 0)
            {
                Progress += 1.0f;
            }
        }
    }	
}
