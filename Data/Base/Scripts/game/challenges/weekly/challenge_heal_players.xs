// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Heal_Players : Challenge
{
    Challenge_Heal_Players()
    {
        ID = "CHALLENGE_HEAL_PLAYERS";
        Name = "#menu_game_setup_challenge_weekly";
        Description = "#challenge_heal_players_description";
        Timeframe = ChallengeTimeframe.Weekly;
        Requires_Activation = false;
        Max_Progress = 10000.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
        return Get_Standard_Rewards(2, <int>(Max_Progress / 2000));
    }

    public event void Seeded_Setup()
    {
		Max_Progress = <int>(Math.Rand(20, 80) * 100.0f); 
        Description = Locale.Get("#challenge_heal_players_description").Format(new object[] { <object>(<int>Max_Progress) });
	}

	public event void On_Pawn_Hurt(Event_Pawn_Hurt evt)
	{
		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Harmer;
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
			if (evt.Damage < 0.0f)
			{
				Progress += -evt.Damage;
			}
		}
	}
}
