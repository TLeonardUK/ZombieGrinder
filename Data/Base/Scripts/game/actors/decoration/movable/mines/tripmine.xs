// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.decoration.movable.movable;
using game.actors.player.human_player;
using system.fx;

[
	Placeable(true), 
	Name("Tripmine"), 
	Description("Objects that can be picked up and moved.") 
]
public class Tripmine : Movable 
{
	private Direction Current_Direction = Direction.N;
	public Effect Beam_Effect;

	public event void On_Beam_Trigger(Event_Particle evt)
	{
		if (Network.Is_Server())
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_Destroy();
		}
	}

	public override void On_Update_State()
	{
		// Grab human.
		Human_Player human = Get_Human();
		if (human != null)
		{
			Current_Direction = human.Get_Direction();
		}
		
		// Update direction frame.
		string frame = sprite.Frame;
		Vec3 muzzle_position;
		float muzzle_angle = Direction_Helper.Angles[Current_Direction] - Math.PI;
		switch (Current_Direction)
		{
			case Direction.N: 
				{
					frame = "particle_tripmine_projectile_0";
					Collision_Box = Vec4(0, 0, 16, 8);
					muzzle_position = this.Position + Vec3(8.0f, 5.0f, 0.0f);  
					break;
				}
			case Direction.E: 
				{
					frame = "particle_tripmine_projectile_2";	
					Collision_Box = Vec4(8, 0, 8, 16);
					muzzle_position = this.Position + Vec3(14.0f, 9.0f, 0.0f);  
					break;
				}
			case Direction.S: 
				{
					frame = "particle_tripmine_projectile_3";	
					Collision_Box = Vec4(0, 8, 16, 8);
					muzzle_position = this.Position + Vec3(8.0f, 15.0f, 0.0f);  
					break;
				}
			case Direction.W: 
				{
					frame = "particle_tripmine_projectile_1";	
					Collision_Box = Vec4(0, 0, 8, 16);
					muzzle_position = this.Position + Vec3(3.0f, 9.0f, 0.0f);  
					break;
				}
		}

		// Spawn the beam effect.		
		if (Beam_Effect != null)
		{
			if (!Scene.Is_Spawned(Beam_Effect) || Beam_Effect.Get_Component().Is_Finished)
			{
				Beam_Effect = null;
			}
			else
			{		
				if (Pickup_Pawn_ID >= 0)
				{
					Scene.Despawn(Beam_Effect);
					Beam_Effect = null;
				}
				else
				{
					Beam_Effect.Position = muzzle_position;
					Beam_Effect.Rotation = muzzle_angle;
				}
			}
		}

		if (Beam_Effect == null && Pickup_Pawn_ID < 0)
		{
			Beam_Effect = FX.Spawn("tripmine_fire", muzzle_position, muzzle_angle, Layer, this);
		}

		// If beam has collided, explode.

		// Update sprite frame.
		sprite.Frame = frame;
		shadow_sprite.Frame = frame;
	}

	Tripmine()
	{
		Pickup_Sound		= "sfx_objects_destroyable_move_start";
		Pickup_Effect		= "";
		Drop_Sound			= "sfx_objects_destroyable_move_end";
		Drop_Effect			= "dust_puff";
		Destroy_Sound		= "";
		Destroy_Effect		= "explosive_destroy";
		Is_Solid			= false;

		Bounding_Box		= Vec4(0, 0, 16, 16);
		Collision_Box 		= Vec4(0, 0, 16, 16);

		//0=down
		//1=left
		//2=right
		//3=down

		sprite.Frame		= "particle_tripmine_projectile_0";
	}
}