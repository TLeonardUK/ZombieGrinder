// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.audio;
using game.actors.interactable.interactable;

[
	Placeable(false), 
	Name("Pickup"), 
	Description("Base class for all items that can either be picked up or bought off walls. Locally and non-replicated.") 
]
public class Local_Pickup : Actor, Interactable
{
	[  
		Name("Purchasable"),
		Description("If set this pickup will be mounted on a wall and be purchasable, otherwise it just bobs up and down and can be picked up.")
	]
 	public serialized(1) bool Purchasable = false;
	
	[ 
		Name("Cost"),
		Description("Coin cost if purchasable.")
	]
	public serialized(1) int Cost = 1000;
	
	[ 
		Name("Respawn Interval"),
		Description("Time between respawns, or 0 if it never despawns.")
	]
	public serialized(1) int Respawn_Interval = 0;

	// Currently spawned?
	public serialized(1) bool Is_Spawned = true;

	public serialized(705) bool Force_Bias_Offset = false;

	// How long before respawn.
	private float Respawn_Timer = 0.0f;

	// If true this pickup will not be disposed of when despawned.
	public bool Do_Not_Dispose_On_Despawn = false;

	// if true this pickup can respawn on screen.
	public bool Can_Respawn_On_Screen = false;
	
	// Bobing effect.
	const float BOB_MIN_HEIGHT = 3.0f;
	const float BOB_HEIGHT = 10.0f;
	const float BOB_INTERVAL = 1.5f;
	const float BOB_SCALE_REDUCTION = 0.2f;
	private float m_bob_time = 0.0f;
	private int m_bob_direction = 0;

	// If set then this pickup is attractive to AI, will attempt to pickup.
	public bool AI_Attractive = false;

	// If set then Use will be invoked on remote clients as well as the one actually picking up the pickup.
	public bool Use_On_Remote = false;
	
	Local_Pickup()
	{
		Bounding_Box = Vec4(0, 0, 24, 24);

		// No real point ticking movables if nobody is alround to see em.
		Tick_Offscreen	= false;
	}

	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return Purchasable ? Cost : 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		if (!Is_Spawned)
		{
			return "";
		}

		if (Purchasable == false)
		{
			if (Network.Is_Client() && Options.Get_Bool("require_input_to_pickup_items"))
			{
				return Locale.Get("#pickup_prompt");
			}

			return "";
		}
		else
		{
			return Get_Buy_Prompt(other);
		}
	}

	public override void Interact(Pawn other)
	{
		if (!Is_Spawned)
		{
			return;
		}

		// If multi-use, then just straight up use it.
		if (Purchasable == true)// || Respawn_Interval == 0)
		{
			Use(other);
		}
		else
		{
			Trigger(other.Net_ID);
		}
	}
	
	private void Trigger(int id)
	{
		Pawn other = <Pawn>Scene.Get_Net_ID_Actor(id);

		if (!Is_Spawned)
		{
			return;
		}

		if (other != null)
		{
			if ((<Interactable>this).Can_Interact(other)) // TODO: Fix compiler, this cast should be redundent.
			{
				Log.Write("Invoking pickup use for object "+this.Net_ID);

				Use(id);

				if (Respawn_Interval != 0 || !Purchasable)
				{
					Is_Spawned = false;
					Real_Despawn();
				}
			}
		}
	}
	
	public void Despawn()
	{		
		Is_Spawned = false;
		Real_Despawn();
	}

	private void Use(int id)
	{
		Pawn other = <Pawn>Scene.Get_Net_ID_Actor(id);
		if (other != null)
		{
			// Do we ever really care when other users pick stuff up?
			if ((other.Owner != null && other.Owner.Is_Local) || Use_On_Remote == true)
			{
				Use(other);
			}
		}
	}
	
	private void Real_Despawn()
	{
		if (Respawn_Interval == 0 && !Purchasable && !Do_Not_Dispose_On_Despawn)
		{			
			Scene.Despawn(this);
		}
		else
		{
			Is_Spawned = false;
			Respawn_Timer = Respawn_Interval;
		}

		change_state(Despawned);
	}

	private void Respawn()
	{
		Is_Spawned = true;
		change_state(Spawned);
	}
	
	public virtual string Get_Sprite()
	{
		return "";
	}

	public virtual void Use(Pawn other)
	{
	}
	
	public virtual string Get_Buy_Prompt(Pawn other)
	{
		return "";
	}

	public virtual bool Check_Respawn()
	{
		Respawn_Timer -= Time.Get_Delta_Seconds();

		// Respawn if its time and we are offscreen, don't want magic reappearing stuff, looks wierd.
		if (Respawn_Timer <= 0.0f && (!Network.Is_Visible_To_Users(World_Bounding_Box) || Can_Respawn_On_Screen == true))
		{
			return true;
		}

		return false;
	}

	state Despawned
	{
		event void On_Enter()
		{
			sprite.Visible = false;
			shadow.Visible = false;
		}
		event void On_Tick()
		{
			if (Is_Spawned)
			{
				change_state(Spawned);
			}

			if (Network.Is_Server())
			{
				if (Check_Respawn())
				{
					Respawn();
				}
			}
		}
	}

	default state Spawned
	{
		event void On_Enter()
		{
			sprite.Visible = true;
			shadow.Visible = true;			
			collision.Enabled = !Purchasable;
	
			sprite.Frame = Get_Sprite();
			shadow.Frame = sprite.Frame;

			// Offset pickups so they stay above gibs.
			if (sprite.Depth_Bias < 1.0f)
			{
				sprite.Depth_Bias += 8;
				shadow.Depth_Bias += 8;
			}
		}
		event void On_Tick()
		{
			if (!Is_Spawned)
			{
				change_state(Despawned);
			}

			// Bob up and down a bit.
			if (!Purchasable)
			{
				m_bob_time += Time.Get_Delta_Seconds();

				float bob_delta = Math.Clamp(m_bob_time / BOB_INTERVAL, 0.0f, 1.0f);
				
				// Bob up.
				if (m_bob_direction == 0)
				{
					sprite.Offset = Math.SmoothStep(Vec2(0.0f, -BOB_MIN_HEIGHT), Vec2(0.0f, -BOB_HEIGHT), bob_delta);
					shadow.Draw_Scale = Math.SmoothStep(Vec2(1.0f, 1.0f), Vec2(1.0f - BOB_SCALE_REDUCTION, 1.0f - BOB_SCALE_REDUCTION), bob_delta);
				}
				// Bob down.
				else
				{
					sprite.Offset = Math.SmoothStep(Vec2(0.0f, -BOB_HEIGHT), Vec2(0.0f, -BOB_MIN_HEIGHT), bob_delta);
					shadow.Draw_Scale = Math.SmoothStep(Vec2(1.0f - BOB_SCALE_REDUCTION, 1.0f - BOB_SCALE_REDUCTION), Vec2(1.0f, 1.0f), bob_delta);
				}

				// Reverse direction.
				if (bob_delta == 1.0f)
				{
					m_bob_direction = 1 - m_bob_direction;
					m_bob_time = 0.0f;
				}
			}
			else if (Force_Bias_Offset)
			{
				if (Network.Is_Server())
				{
					// Shit fix, do this properly! Find out why the depth biases are fucked.
					sprite.Depth_Bias = 16;
				}
				else
				{
					sprite.Depth_Bias = 32;
				}
			}
		}
		event void On_Touch(CollisionComponent component, Actor other)
		{
			if (Network.Is_Client())
			{
				if (!Purchasable)
				{
					if ((other.Owner != null && other.Owner.Is_Local == true)) // Local players deal with collision.
					{
						if (Options.Get_Bool("require_input_to_pickup_items"))
						{
							return;
						}

						Human_Player player = <Human_Player>other;
						if (player != null && !player.Is_Incapacitated() && !player.Is_Interact_Locked())
						{
							Trigger(other.Net_ID);
						}
					}
				}
			}
		}
	}
	
	components
	{
		SpriteComponent shadow
		{
			Frame = "actor_ammo_crate_default_0";
			Color = Vec4(0.0, 0.0, 0.0, 170.0);
			Depth_Bias = -0.01f;
			Sort_By_Transparency = true;
		}
		SpriteComponent sprite
		{
			Frame = "actor_ammo_crate_default_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(8, 8, 8, 8);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.Player;
		}
	}
}
