// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Revive_Players : Challenge
{
    Challenge_Revive_Players()
    {
        ID = "CHALLENGE_REVIVE_PLAYERS";
        Name = "#menu_game_setup_challenge_weekly";
        Description = "#challenge_revive_players_description";
        Timeframe = ChallengeTimeframe.Weekly;
        Requires_Activation = false;
        Max_Progress = 10000.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
        return Get_Standard_Rewards(2, <int>(Max_Progress / 20));
    }

    public event void Seeded_Setup()
    {
		Max_Progress = <int>(Math.Rand(40, 80) / 20.0f) * 20; 
        Description = Locale.Get("#challenge_revive_players_description").Format(new object[] { <object>(<int>Max_Progress) });
	}
	
	event void On_Pawn_Revived(Event_Pawn_Revived evt)
	{
		if (evt.Reviver != null && evt.Reviver.Owner != null && evt.Reviver.Owner.Is_Local == true)
		{
			Progress += 1.0f;
		}
	}
}
