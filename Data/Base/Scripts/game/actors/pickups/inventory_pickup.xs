// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(false), 
	Name("Inventory Pickup"), 
	Description("Base class for inventory item pickup logic (bags, keys, etc).") 
]
public class Inventory_Pickup : Pickup
{	
	protected string Pickup_Sprite		= "actor_pickups_acid_gun_0";
	protected string Pickup_Name		= "";
	protected string Pickup_Sound		= "";

	public    int  Player_Owner_ID = -1;
	protected bool Should_Respawn   = false;

	Inventory_Pickup()
	{
		Use_On_Remote = true;
		Can_Respawn_On_Screen = true;
        AI_Attractive = true;
	}

	public void Force_Spawn()
	{
		Should_Respawn = true;
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
			return true;
		}

		return false;
	}

	public override string Get_Buy_Prompt(Pawn other)
	{
		return ""; // Shouldn't be able to buy these! pickup only plz.
	}

	public void Reset()
	{
		Player_Owner_ID = -1;
	}

	public override bool Check_Respawn()
	{
		if (Should_Respawn)
		{
			Log.Write("Respawning object "+this.Net_ID);
			Should_Respawn = false;
			return true;
		}

		if (Player_Owner_ID >= 0)
		{
			Actor player = Scene.Get_Net_ID_Actor(Player_Owner_ID);
			if (player == null)
			{
				return true;
			}
			else
			{
				// Track the player that has picked us up.
				Position = player.Collision_Center - Vec3(this.Bounding_Box.Center(), 0.0f);
			}
		}

		return false;
	}

	public static Inventory_Pickup Get_Inventory_Item_For_Net_ID(int net_id)
	{
		Actor actor = Scene.Find_Actor_By_Field(typeof(Inventory_Pickup), indexof(Inventory_Pickup.Player_Owner_ID), net_id);
		return actor as Inventory_Pickup;
	}

	public static Inventory_Pickup Get_Held_Inventory_Item_By_Type(Type item)
	{
		Actor[] pickups = Scene.Find_Actors(item);
		foreach (Inventory_Pickup pickup in pickups)
		{
			if (pickup.Player_Owner_ID >= 0)
			{
				return pickup;
			}
		}
		return null;
	}

	public override void Use(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			if (Network.Is_Server())
			{
				// If player already has an inventory item, drop it.						
				Inventory_Pickup existing = Get_Inventory_Item_For_Net_ID(player.Net_ID);
				if (existing != null)
				{
					Log.Write("Dropping pre-existing inventory item "+this.Net_ID);
					existing.Player_Owner_ID = -1;
					existing.Position = player.Collision_Center - Vec3(existing.Bounding_Box.Center(), 0.0f);
					existing.Should_Respawn = true;
				}			

				// Give player this inventory item.
				Player_Owner_ID = player.Net_ID;

				Log.Write("Giving inventory pickup "+this.Net_ID+" to player "+player.Net_ID);
			}

			if (Pickup_Sound != "")
			{
				Audio.Play2D(Pickup_Sound);
			}
		}
	}

	// Force replication as network object.
	replicate(ReplicationPriority.Low)
	{
		Player_Owner_ID
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}	
		Position
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}		
	}	
}