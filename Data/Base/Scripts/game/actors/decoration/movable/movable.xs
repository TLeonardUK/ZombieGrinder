// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.audio;
using game.actors.player.human_player;
using game.actors.interactable.interactable;
using game.weapons.weapon_ammo;

[
	Placeable(false), 
	Name("Movable"), 
	Description("Objects that can be picked up and moved.") 
]
public class Movable : Actor, Interactable, EnemyAttackable
{
	const float INTERACTION_DURATION = 500.0f;
	Vec4 COLLISION_AREA_PLACEMENT_BUFFER = new Vec4(-4.0f, -4.0f, 8.0f, 8.0f);
	Vec4 DAMAGE_AREA_BUFFER = new Vec4(-2.0f, -2.0f, 4.0f, 4.0f);

	// NetID of pawn currently hodling this item.
	public serialized(1) int Pickup_Pawn_ID = -1;
	
	// NetID of the NetUser who owns this turret. If set to -1 this
	// is owned by the server. Determines who is allowed to move it.
	public serialized(1) int Owner_ID = -1;

	// ID of team this current belongs to. If set to -1 this is owned by
	// the server. Determines behaviour!
	public serialized(1) int Team_ID = -1;
	
    private bool m_setup_pending = false;

	// Collision area.
	public Vec4 Collision_Box;

	// Placement sounds / effects.
	public serialized(2001) string Pickup_Sound		= "";
	public serialized(2001) string Pickup_Effect		= "";
	public serialized(2001) string Drop_Sound		= "";
	public serialized(2001) string Drop_Effect		= "dust_puff";
	public serialized(2001) string Destroy_Sound		= "";
	public serialized(2001) string Destroy_Effect	= "metal_destroy";

	// Health state.
	public bool Is_Indestructable = false;
	public serialized(1) float Max_Health = 30.0f;
	public serialized(1) float Health = 30.0f;
	public serialized(300) bool Destroyed = false;

	protected float Depth_Offset = 0.0f;

	// If set then this movable is solid.
	public bool Is_Solid = true;

	// Auto positions to the movable object on the first frame to prevent wierd lerping
	// effects when moving to the first "picked up" state.
	public bool Auto_Position = false;

	Movable()
	{
		Bounding_Box	= Vec4(0, 0, 16, 32);
		Collision_Box 	= Vec4(0, 16, 16, 16);

		// No real point ticking movables if nobody is alround to see em.
		Tick_Offscreen	= false;
	}
	
	virtual event void On_Editor_Property_Change()
	{
		shadow_sprite.Frame = sprite.Frame;
		collision.Area = Collision_Box;
		damage_sensor.Area = Collision_Box + DAMAGE_AREA_BUFFER;
		placement_sensor.Area = Collision_Box + COLLISION_AREA_PLACEMENT_BUFFER;
	}
	
	// Gets the team this actor belongs on.
	public override int Get_Team_Index()
	{
		return Team_ID;
	}
	
	public override NetUser Get_Imposing_As()
	{
		return Network.Get_User(Owner_ID);
	}

	virtual void On_Update_State()
	{
		// Nothing to do ...
	}

	virtual void Set_Ammo_Modifier(Weapon_Ammo ammo)
	{
		// Turrets etc
	}

	virtual void Set_Upgrade_Modifiers(float[] modifiers)
	{
		// Turrets etc
	}

	default state Idle
	{
		event void On_Enter()
		{ 
            m_setup_pending = true;
		}
		event void On_Tick()
		{
            // Setup idle state. We do this here rather than in On_Enter so it happens on the next frame, thus after any collision resolving has taken place.
            if (m_setup_pending)
            { 
			    collision.Enabled = Is_Solid;
			    collision.Area = Collision_Box;
			    damage_sensor.Area = Collision_Box + DAMAGE_AREA_BUFFER;
			    placement_sensor.Enabled = false;
			    placement_sensor.Area = Collision_Box + COLLISION_AREA_PLACEMENT_BUFFER;

			    sprite.Depth_Bias = this.Collision_Box.Y + Depth_Offset;
			    sprite.Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
                Depth_Y_Offset = 0.0f;

		    	if (!Is_Solid)
			    {
				    collision.Type = CollisionType.Non_Solid;		
			    }

			    shadow_sprite.Frame = sprite.Frame;
    
                m_setup_pending = false;
            }    
        
            // Check if we have a human and need to go to picked up state.
			if (Pickup_Pawn_ID != -1)
			{
				Human_Player human = Get_Human();
				if (human != null)
				{
					Vec3 target_pos = Get_Target_Position(human);

					if (Auto_Position)
					{
						Position = target_pos;
                        Auto_Position = false;
					}

					change_state(Picked_Up);
					return;
				}
			}
            
			// Lerp to "pickup" position.			
			sprite.Offset = Math.Lerp(sprite.Offset, Vec2(0.0f, 0.0f), Time.Get_Delta() * 4.0f);
			sprite.Color = Vec4(255.0, 255.0, 255.0, 255.0) * Get_Health_Color_Scale();
			sprite.Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

			// Change shadow scale with height.
			float delta = 1.0f - ((-sprite.Offset.Y) / 16.0f);
			float scale = 0.7f + (0.3f * delta);
			shadow_sprite.Draw_Scale = Vec2(scale, scale);
			
			On_Update_State();
		}
	}
	
	state Picked_Up
	{
		event void On_Enter()
		{
			collision.Enabled = false;
			placement_sensor.Enabled = true;
		}
		event void On_Tick()
		{
			Human_Player human = Get_Human();

			// No human, back to idle state.
			if (human == null)
			{
				if (Network.Is_Server())
				{
					// If we got here then we were "dropped" without an RPC_Drop, either player
					// died or left. In which case, destroy this object if its not in a valid place.
					if (placement_sensor.Is_Colliding())
					{
						rpc(RPCTarget.All, RPCTarget.None) RPC_Destroy();
					}
					else
					{
						rpc(RPCTarget.All, RPCTarget.None) RPC_Drop(Position);
					}
				}

				change_state(Idle);
				return;
			}

			// Place in front of the user who picked us up.
			if (human != null)
			{
				// If human is local, check if they are press the drop button.
				if (human.Owner != null && human.Owner.Is_Local)
				{
					if (!placement_sensor.Is_Colliding())
					{
						if (Input.Was_Pressed(human.Owner, OutputBindings.Interact))
						{
							rpc(RPCTarget.All, RPCTarget.None) RPC_Drop(Position);
                            return;
						}
					}
				}

				// Figure out position.
				Vec3 target_pos = Get_Target_Position(human);
				Position = Math.Lerp(this.Position, target_pos, Time.Get_Delta() * 4.0f);
			}
			
			// Can we place it?
			Vec4 target_color = Vec4(255.0, 255.0, 255.0, 255.0) * Get_Health_Color_Scale();
			if (placement_sensor.Is_Colliding())
			{
				target_color = Vec4(255.0, 0.0, 0.0, 70.0);
			}
			sprite.Color = Math.Lerp(sprite.Color, target_color, Time.Get_Delta() * 4.0f);

			// Lerp to "pickup" position.			
			sprite.Offset = Math.Lerp(sprite.Offset, Vec2(0.0f, -16.0f), Time.Get_Delta() * 4.0f);

			// Change shadow scale with height.
			float delta = 1.0f - ((-sprite.Offset.Y) / 16.0f);
			float scale = 0.7f + (0.3f * delta);
			shadow_sprite.Draw_Scale = Vec2(scale, scale);
			
			// Update collision as derived classes may modify it.
			placement_sensor.Area = Collision_Box + COLLISION_AREA_PLACEMENT_BUFFER;
			collision.Area = Collision_Box;
			damage_sensor.Area = Collision_Box + DAMAGE_AREA_BUFFER;
			sprite.Depth_Bias = this.Collision_Box.Y + Depth_Offset;

			On_Update_State();
		}
	}

    void Reduce_Health(float amount)
    {
        On_Damage(damage_sensor, null, this.Position, 0.0f, amount, CollisionDamageType.Melee, 0, 0, Weapon_SubType.Melee, null);
    }

	event bool On_Damage(
		CollisionComponent component, 
		Actor instigator, 
		Vec3 position, 
		float dir, 
		float damage, 
		CollisionDamageType type,
		int ricochet_count,
		int penetration_count,
		Weapon_SubType weapon_subtype,
        Item_Archetype weapon_item)
	{
		// Still alive?
		if (Destroyed)
		{
			return false;
		}
		
		// Make sure turrets etc don't shot themselves.
		if (instigator == this)
		{
			return false;
		}

		if (instigator != null && Team_Index <= 0 && Team_Index == instigator.Get_Team_Index())
		{
			return false;
		}
			
		// Scale by instigators damage multiplier.
		//damage = Calculate_Damage_Multiplier(instigator, damage);

		if (Is_Indestructable == false)
		{
			Health = Math.Min(Max_Health, Math.Max(0.0f, Health - damage));
			//Log.Write("Object was damaged, health is now "+Health+".");
			
			bool bResponsibleForDeath = (((instigator == null || instigator.Owner == null) && Network.Is_Server()) || (instigator != null && instigator.Owner != null && instigator.Owner.Is_Local));
			if (bResponsibleForDeath)
			{
				if (Health <= 0.0f)
				{
					Event_Movable_Destroyed evt = new Event_Movable_Destroyed();
					evt.Destroyed = this;
					evt.Destroyer = instigator;
					evt.Fire_Global();

					rpc(RPCTarget.All, RPCTarget.None) RPC_Destroy();
				}
			}

			return true;
		}

		return true;
	}

	public float Get_Height_Offset()
	{
		return sprite.Offset.Y;
	}

	public Vec4 Get_Health_Color_Scale()
	{
		float delta = 0.5f + ((Health / Max_Health) * 0.5f);
		return Vec4(delta, delta, delta, 1.0f);
	}

	public Vec3 Get_Target_Position(Human_Player human)
	{
		return (human.Collision_Center) - 
			Vec3(this.Collision_Box.X, this.Collision_Box.Y, 0.0f) - 			
			Vec3(this.Collision_Box.Width * 0.5f, this.Collision_Box.Height * 0.5f, 0.0f) +
			(Direction_Helper.Vectors[human.Current_Direction] * Vec3(Math.Max(this.Collision_Box.Width * 1.0f, 32.0f), Math.Max(this.Collision_Box.Height * 1.50f, 32.0f), 0.0f));
	}

	public Human_Player Get_Human()
	{
		Human_Player human = <Human_Player>Scene.Get_Net_ID_Actor(Pickup_Pawn_ID);
		return human;
	}

	public override bool Can_Interact(Pawn other)
	{
		Human_Player human = <Human_Player>other;
		if (Is_Pawn_Moving(other))
		{
			return false;
		}

		// Same owner?
		if (Owner_ID >= 0)
		{
			if (other.Owner != null)
			{
				if (other.Owner.Net_ID != Owner_ID)
				{
					return false;
				}
			}
		}

		return human != null && Get_Human() == null;
	}

	public static bool Is_Pawn_Moving(Actor other)
	{
		Actor actor = Scene.Find_Actor_By_Field(typeof(Movable), indexof(Movable.Pickup_Pawn_ID), other.Net_ID);
		return actor != null;
	}

	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#move_prompt");
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}

	public override float Get_Duration(Pawn other)
	{
		return INTERACTION_DURATION;
	}

	public override void Interact(Pawn other)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Pickup(other.Net_ID);
	}
	
	protected rpc void RPC_Pickup(int id)
	{
		Log.Write("Object was picked up by pawn id '" + id + "'");
		Pickup_Pawn_ID = id;

		if (Pickup_Effect != "")
		{
			 FX.Spawn_Untracked(Pickup_Effect, Collision_Center, 0, Layer, null, 0);
		}
		if (Pickup_Sound != "")
		{
			Audio.Play3D(Pickup_Sound, Center);
		}

		change_state(Picked_Up);
	}

	protected rpc void RPC_Drop(Vec3 pos)
	{
		Log.Write("Object was dropped by '" + Pickup_Pawn_ID + "' at "+pos.X+","+pos.Y+","+pos.Z);

        Pickup_Pawn_ID = -1;
		Position = pos;
    
		if (Drop_Effect != "")
		{
			 FX.Spawn_Untracked(Drop_Effect, Collision_Center, 0, Layer, null, 0);
		}
		if (Drop_Sound != "")
		{
			Audio.Play3D(Drop_Sound, Center);
		}

		change_state(Idle);
	}
	
	protected virtual void On_Destroy()
	{
	}

    public void Force_Destroy()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Destroy();
    }

	protected rpc void RPC_Destroy()
	{
		if (Destroyed == true)
		{
			return;
		}

		Destroyed = true;

		Log.Write("Object was destroyed.");
		
		if (Destroy_Effect != "")
		{
			 FX.Spawn_Untracked(Destroy_Effect, Collision_Center, 0, Layer, null, 0);
		}
		if (Destroy_Sound != "")
		{
			Audio.Play3D(Destroy_Sound, Center);
		}

		On_Destroy();

		Scene.Despawn(this);
	}

    public void Set_Sprite_Frame(string frame)
    {
        sprite.Frame = frame;
    }

	// Force replication as network object.
	replicate(ReplicationPriority.Low)
	{
		Health
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Interpolate_Linear;
		}
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "decal_barrel_all_colors_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
		SpriteComponent shadow_sprite
		{
			Frame			= "decal_barrel_all_colors_0";
			Color			= Vec4(0.0, 0.0, 0.0, 200.0);
			Depth_Bias 		= -128.0;
			Draw_Scale		= Vec2(0.8f, 0.8f);
			Sort_By_Transparency = true;
		}
		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 16, 24);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Environment;
			Collides_With	= CollisionGroup.All;
			Blocks_Paths	= true; 
			Blocks_Path_Spawns	= true;
		}
		CollisionComponent damage_sensor
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 16, 24);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.Damage;
		}
		CollisionComponent placement_sensor
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(-4, -4, 16 + 8, 24 + 8);					// Always expand slightly to take into account latent collision-detection.
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.All & (~CollisionGroup.Particle);
		}
	}
}