// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(true), 
	Name("Ammo Pickup"), 
	Description("Base class for weapon pickup logic.") 
]
public class Ammo_Pickup : Pickup
{	
	protected string Pickup_Sprite		= "actor_pickups_ammo_0";
	protected string Pickup_Name		= Locale.Get("#item_ammo_name");
	protected string Pickup_Sound		= "sfx_objects_weapons_uzi_reload";
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
			if (Ammo_Type == null)
			{
				return true;
			}
			else
			{
				Weapon active = player.Get_Active_Weapon();
				if (active != null)
				{				
					return active.Can_Accept_Weapon_Modifier(Ammo_Type);
				}
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
			if (Ammo_Type != null)
			{
				player.Give_Weapon_Modifier(Ammo_Type);
			}
			else
			{
				//player.Fill_Active_Reserve_Ammo(); 
				player.Fill_Active_Ammo(); 
			}

			if (Pickup_Sound != "")
			{
				Audio.Play2D(Pickup_Sound);
			}
		}
	}
}