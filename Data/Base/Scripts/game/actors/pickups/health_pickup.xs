// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(false), 
	Name("Health Pickup"), 
	Description("Base class for health pickup logic.") 
]
public class Health_Pickup : Pickup
{	
	protected string Pickup_Sprite	= "actor_pickups_acid_gun_0";
	protected string Pickup_Name		= "";
	protected string Pickup_Sound		= "";

	protected float  Health_Points	= 0;
	protected float  Armor_Points	= 0;
	
	Health_Pickup()
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
			if ((Health_Points > 0 && player.Health < player.Max_Health) ||
				(Armor_Points > 0 && player.Armor < player.Max_Armor))
			{
				return true;
			}
		}

		return false;
	}

	public override string Get_Buy_Prompt(Pawn other)
	{
		if (Health_Points > 0) 
		{
			return Locale.Get("#buy_health_prompt").Format(new object[] { Pickup_Name, <int>Health_Points });		
		}
		else
		{
			return Locale.Get("#buy_armour_prompt").Format(new object[] { Pickup_Name, <int>Armor_Points });
		}
	}

	public override void Use(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			player.Heal(Health_Points, Armor_Points);

			if (Pickup_Sound != "")
			{
				Audio.Play2D(Pickup_Sound);
			}
		}
	}
}