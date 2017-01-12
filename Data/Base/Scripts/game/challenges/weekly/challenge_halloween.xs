// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Halloween : Challenge
{	
    Challenge_Halloween()
    {
        ID = "CHALLENGE_HALLOWEEN";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_halloween_description";
        Timeframe = ChallengeTimeframe.Daily;
        Requires_Activation = false;
        Max_Progress = 50.0f;

		Specific_Date = true;
		Start_Day = 31;
		Start_Month = 10;
		
		Announce = true;
		Announce_Image = "screen_challenge_announce_ghost";
		Announce_Name = "#challenge_halloween_announce_name";
		Announce_Description = "#challenge_halloween_announce_description";
    }

    public override Item_Archetype[] Get_Rewards()
    {
		Item_Archetype[] rewards = new Item_Archetype[1]; 
		rewards[0] = Item_Archetype.Find_By_Type(typeof(Item_Pumpkin));		
		return rewards;
    }

    public event void Seeded_Setup()
    {
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

				if (killer_user.Controller != null)
				{
					if (killer_user.Controller.Possessed != null)
					{
						killer_actor = killer_user.Controller.Possessed;
					}
				}
			}
		}

		// Give rewards to local players.
		if (killer_user != null && 
			killer_user.Is_Local == true)
		{ 
			if (Math.Rand(0, 100) <= 20)
			{
				Log.Write("[Halloween] Spawning candy corn!");
				
				Actor drop = Scene.Spawn(typeof(Candy_Corn_Pickup), null);
				drop.Layer = evt.Dead.Layer;
				drop.Position = Vec3(evt.Dead.World_Bounding_Box.Center(), 0.0f) - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f);
			}
		}
	}		
	
	event void On_Candy_Corn_Picked_Up(Event_Candy_Corn_Picked_Up evt)
	{
		Log.Write("[Halloween] User picked up candy corn.");			
		Progress += 1.0f;
	}
}
