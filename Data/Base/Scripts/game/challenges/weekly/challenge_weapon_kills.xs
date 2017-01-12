// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Weapon_Kills : Challenge
{
    Weapon_SubType m_sub_type;
    int m_type_index;

	private Weapon_SubType[] Weapon_Type_Table = 
	{
        Weapon_SubType.Melee,
	    Weapon_SubType.Heavy,
	    Weapon_SubType.Turret,
	    Weapon_SubType.Small_Arms,
	    Weapon_SubType.Rifle,
	    Weapon_SubType.Grenade
    };
	
	private string[] Weapon_Table_Names = 
	{
        "#challenge_weapon_sub_type_melee",
	    "#challenge_weapon_sub_type_heavy",
	    "#challenge_weapon_sub_type_turret",
	    "#challenge_weapon_sub_type_small_arms",
	    "#challenge_weapon_sub_type_rifle",
	    "#challenge_weapon_sub_type_grenade"
    };
	
    Challenge_Weapon_Kills()
    {
        ID = "CHALLENGE_WEAPON_KILLS";
        Name = "#menu_game_setup_challenge_weekly";
        Description = "#challenge_weapon_kills_description";
        Timeframe = ChallengeTimeframe.Weekly;
        Requires_Activation = false;
        Max_Progress = 1.0f;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int MaxRewards = Math.Min(4, 1 + ((<int>Max_Progress - 100000) / 50000));
        return Get_Standard_Rewards(1, MaxRewards);
    }

    public event void Seeded_Setup()
    {
		Max_Progress = <int>(Math.Rand(2, 6) * 50000);
        m_type_index = Math.Rand(0, Weapon_Type_Table.Length());
        m_sub_type = Weapon_Type_Table[m_type_index];
        Description = Locale.Get("#challenge_weapon_kills_description").Format(new object[] { <object>(<int>Max_Progress), <object>Locale.Get(Weapon_Table_Names[m_type_index]) });
	}
 
	event void On_Pawn_Hurt(Event_Pawn_Hurt evt)
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
			if (evt.Damage > 1.0f)
			{
                if (evt.Weapon_Sub_Type == m_sub_type)
                {
    				Progress += evt.Damage;
                }
			}
		}
	}	
}
