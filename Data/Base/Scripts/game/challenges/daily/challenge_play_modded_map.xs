// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Play_Modded_Map : Challenge
{
    Challenge_Play_Modded_Map()
    {
        ID = "CHALLENGE_PLAY_MODDED_MAP";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_modded_map_description";
        Timeframe = ChallengeTimeframe.Daily;
        Requires_Activation = false;
        Max_Progress = 1.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
        return Get_Standard_Rewards(1, 2);
    }

    public event void On_Map_Load()
    {
        if (Scene.Is_Workshop_Map())
        {
            if (Network.Get_User_Count() > 1)
            {
                Progress += 1.0f;
            }
        }
    }
}
