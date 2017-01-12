// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;
using game.weapons.grenade.weapon_turret;

[
	Placeable(true), 
	Name("Turret Pickup"), 
	Description("Base class for turret pickup logic.") 
]
public class Turret_Pickup : Pickup
{	
	protected string Pickup_Sprite		= "actor_pickups_turret_0";
	protected string Pickup_Name		= Locale.Get("#item_turret_name");
	protected string Pickup_Sound		= "sfx_objects_pickups_turret_pickup";
	protected Type	 Weapon_Type		= typeof(Weapon_Turret);
	protected Type	 Ammo_Type			= null;

	public override string Get_Sprite()
	{
		return Pickup_Sprite;
	}

	public override bool Can_Interact(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{	
			return true;
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
			if (Ammo_Type != null)
			{
				player.Give_Weapon_Modifier(Ammo_Type);
			}

			if (Pickup_Sound != "")
			{
				Audio.Play2D(Pickup_Sound);
			}
		}
	}
}