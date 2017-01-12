// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Zombie_Kills : Challenge
{
    Challenge_Zombie_Kills()
    {
        ID = "CHALLENGE_ZOMBIE_KILLS";
        Name = "#menu_game_setup_challenge_monthly";
        Description = "#challenge_zombie_kills_description";
        Timeframe = ChallengeTimeframe.Monthly;
        Requires_Activation = false;
        Max_Progress = 100000.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int Max_Total = <int>Max_Progress / 5000; 
        return Get_Standard_Rewards(Max_Total, Max_Total);
    }

    public event void Seeded_Setup()
    {
		int Max = Math.Rand(2, 4);
		Max_Progress = 5000 * Max;
        Description = Locale.Get("#challenge_zombie_kills_description").Format(new object[] { <object>(<int>Max_Progress) });
	}
	
	event void On_Pawn_Killed(Event_Pawn_Killed evt)
	{		
		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Killer;
		NetUser killer_user  = killer_actor == null ? null : killer_actor.Owner;

		if (killer_actor != null)
		{
			NetUser imposer = killer_actor.Get_Imposing_As();
			if (imposer != null)
			{
				killer_user = imposer;			
				killer_actor = null;
			}
		}

		// Give rewards to local players.
		if (killer_user != null && 
			killer_user.Is_Local == true)
		{
			if ((<Enemy>evt.Dead) != null)
			{
				Progress += 1.0f;
			}
		}
	}
}
