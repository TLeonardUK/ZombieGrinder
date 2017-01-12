// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_purple_gem_1;

[
	Placeable(false), 
	Name("Candy Corn Pickup"), 
	Description("Sweet.") 
]
public class Candy_Corn_Pickup : Local_Pickup
{
	Candy_Corn_Pickup()
	{
		Cost = 9999999;
	}
	
	public override string Get_Sprite()
	{
		return "actor_pickups_candy_corn_0";
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
		Event_Candy_Corn_Picked_Up evt = new Event_Candy_Corn_Picked_Up();
		evt.Fire_Global();
	
		Audio.Play2D("sfx_objects_pickups_candy_corn_pickup");
	}
}