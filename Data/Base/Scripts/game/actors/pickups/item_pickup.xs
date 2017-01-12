// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(false), 
	Name("Item Pickup"), 
	Description("Base class for inventory item pickup logic.") 
]
public class Item_Pickup : Pickup
{	
	protected string Pickup_Sprite		= "actor_pickups_acid_gun_0";
	protected string Pickup_Name		= "";
	protected string Pickup_Sound		= "";

	protected Type   Archetype			= null;

	public override string Get_Sprite()
	{
		return Pickup_Sprite;
	}

	public override bool Can_Interact(Pawn other)
	{
		if (!Is_Spawned)
		{
			return false;
		}

		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			return true;
		}

		return false;
	}

	public override void Use(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null)  
		{
			NetUser user = player.Owner;
			if (user != null)
			{
                if (!Engine.In_Restricted_Mode())
                { 
				    Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
				    Game_HUD hud = <Game_HUD>mode.Get_HUD();
				    Item_Archetype arch = Item_Archetype.Find_By_Name(Archetype.Name);

				    if (user.Local_Profile.Has_Space_For(arch))
				    {
					    hud.Show_Message
					    (
						    Locale.Get("#message_pickup_gem").Format(new object[] { Locale.Get(arch.Name) }),
						    Locale.Get("#message_pickup_gem_title"),	
						    "tutorial_help"
					    );

					    user.Local_Profile.Add_Item(arch);
				    }
				    else
				    {
					    int sell_price = arch.Cost * user.Stat_Multipliers[Profile_Stat.Sell_Price];

					    hud.Show_Message
					    (
						    Locale.Get("#message_pickup_gem_sold").Format(new object[] { Locale.Get(arch.Name), <string>sell_price }),
						    Locale.Get("#message_pickup_gem_sold_title"),	
						    "tutorial_help"
					    );

					    user.Local_Profile.Coins += sell_price;
				    }
                }
			}

			if (Pickup_Sound != "")
			{
				Audio.Play2D(Pickup_Sound);
			}
		}
	}
}