// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(false), 
	Name("Weapon Pickup"), 
	Description("Base class for weapon pickup logic.") 
]
public class Weapon_Pickup : Pickup
{	
	protected string Pickup_Sprite	= "actor_pickups_acid_gun_0";
	protected string Pickup_Name		= "";
	protected string Pickup_Sound		= "";
	protected Type   Weapon_Type		= null;

	Weapon_Pickup()
	{
		AI_Attractive = true;
	}

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
			if (!player.Has_Weapon(Weapon_Type) || 
				player.Needs_Ammo_For(Weapon_Type))
			{
				return true;
			}
		}

		return false;
	}

	public override string Get_Buy_Prompt(Pawn other)
	{
		return Locale.Get("#buy_generic_prompt").Format(new object[] { Pickup_Name });
	}

	public override void Use(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			player.Give_Weapon(Weapon_Type);

			if (Pickup_Sound != "")
			{
				Audio.Play2D(Pickup_Sound);
			}
		}
	}
}