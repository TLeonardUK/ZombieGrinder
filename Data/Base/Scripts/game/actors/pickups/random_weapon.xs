// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(true), 
	Name("Random Weapon Pickup"), 
	Description("When spawned this creates a random purchasable weapon.") 
]
public class Random_Weapon_Pickup : Actor
{	
	private serialized(800) float Spawn_Probability = 1.0f;
	private serialized(800) bool Purchasable = true;

	Random_Weapon_Pickup()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	default state Idle
	{
		event void On_Enter()
		{
			this.Enabled = true;
		}
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Weapon_Drop_Table weapon_table = new Weapon_Drop_Table();
				
				// Despawn any pre-existing weapons.
				Actor[] pre = Scene.Find_Actors_In_Area(typeof(Pickup), this.World_Bounding_Box);
				foreach (Actor a in pre)
				{
					Scene.Despawn(a);
				}

				if (Math.Rand(0.0f, 1.0f) <= Spawn_Probability)
				{
					Type type = weapon_table.Get_Drop();

					Actor drop = Scene.Spawn(type, null);
					drop.Layer = this.Layer;
					drop.Position = Vec3(this.World_Bounding_Box.Center(), 0.0f) - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f);
					(<Pickup>drop).Force_Bias_Offset = true;

					Pickup pickup = <Pickup>drop;
					if (pickup != null)
					{
						pickup.Purchasable = Purchasable;
					}

					Log.Write("Spawning random weapon "+type.Name);
				}

				Hibernate();
			}
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame 			= "decal_ai_breadcrumb_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}