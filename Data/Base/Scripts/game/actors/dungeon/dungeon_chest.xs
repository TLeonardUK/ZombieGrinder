// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using game.modes.dungeon;

[
	Placeable(true), 
	Name("Dungeon Chest"), 
	Description("Chest that can be opened for coins/rewards. Fires event when opened, can be used to make trap chests.") 
]
public class Dungeon_Chest : Actor, Interactable
{
	public serialized(1) bool bSpecial = false;
	public serialized(1) bool bOpened = false;

	// All players have to be inside this range to interact with stairs.
	const float MINIMUM_INTERACT_DISTANCE = 72.0f;

	Dungeon_Chest()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
	}
	
	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#buy_open_prompt");
	}
	
	public override bool Can_Interact(Pawn other)
	{
		if (!bOpened)
		{
			Human_Player player = <Human_Player>other;
			if (player != null) 
			{
				return true;
			}
		}
		return false;
	}

	public override void Interact(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_Open(other.Owner.Net_ID);
		}
	}

	public rpc void RPC_Open(int opener)
	{
		bOpened = true;

		Update_Sprite();
		Audio.Play3D("sfx_objects_chests_open", this.Center);
	
		Scene.Trigger_Actors(Link);

		// Give rewards to local users.
		NetUser user = Network.Get_User(opener);
		if (user != null && user.Is_Local)
		{
			// Spam money!
			if (bSpecial)
			{
                if (!Engine.In_Restricted_Mode())
                { 
				    Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
				    Game_HUD hud = <Game_HUD>mode.Get_HUD();
				
				    Chest_Drop_Table drop_table = new Chest_Drop_Table();
				    Item_Archetype arch = Item_Archetype.Find_By_Type(drop_table.Get_Drop());

				    // Give user a new item!
				    if (user.Local_Profile.Has_Space_For(arch))
				    {
					    hud.Show_Message
					    (
						    Locale.Get("#message_pickup_gem").Format(new object[] { Locale.Get(arch.Name) }),
						    Locale.Get("#message_pickup_gem_title"),	
						    "tutorial_help"
					    );

					    Item item = user.Local_Profile.Add_Item(arch);
                        item.Is_Indestructable = true; // Chest rewards are forever :3
				    }
				    else
				    {
					    int sell_price = arch.Cost * user.Stat_Multipliers[Profile_Stat.Sell_Price];

					    hud.Show_Message
					    (
						    Locale.Get("#message_pickup_gem_sold").Format(new object[] { Locale.Get(arch.Name), <string>sell_price }),
						    Locale.Get("#message_pickup_gem_sold_title"),	
						    "tutorial_help"
					    );

					    user.Local_Profile.Coins += sell_price;
				    }
                }

				// And some coins!
				FX.Spawn_Untracked("coin_splash_long_large", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, 1000);
			}
			else
			{
				FX.Spawn_Untracked("coin_splash_long_large", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, 1000);
			}
		}
	}

	public void Update_Sprite()
	{
		if (bSpecial)
		{
			sprite.Animation = "actor_chest_special_open";
		}
		else
		{
			sprite.Animation = "actor_chest_standard_open";
		}

		sprite.Frame = "";		
		sprite.Animation_Speed = bOpened ? 1.0f : -1.0f;
	}

	default state Idle
	{
		event void On_Enter()
		{		
            collision.Blocks_Path_Spawns	= true;
			collision.Blocks_Paths	        = true; 

			Update_Sprite();
		}
		event void On_Tick()
		{
			Update_Sprite();
		}
		event void On_Editor_Property_Change()
		{
			Update_Sprite();
		}
	}
	
	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "actor_chest_standard_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		serialized(450) CollisionComponent collision
		{
			Shape 				= CollisionShape.Rectangle;
			Area 				= Vec4(6, 6, 24, 24);
			Type				= CollisionType.Solid;		
			Group				= CollisionGroup.Environment;
			Collides_With		= CollisionGroup.Player | CollisionGroup.Enemy;
			Blocks_Path_Spawns	= true;
			Blocks_Paths	    = true; 
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
	}
}