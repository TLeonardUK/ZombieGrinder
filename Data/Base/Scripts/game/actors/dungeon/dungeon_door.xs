// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using game.actors.interactable.interactable;

[
	Placeable(true), 
	Name("Dungeon Door"), 
	Description("Dungeon doors support locking/sealing/etc. They should be placed in all levels and set to open by default.") 
]
public class Dungeon_Door : Actor, Interactable
{
	[ 
		EditType("Animation"),
		Description("Animation to play when opening. Closing is the same in reverse.")
	]
	public serialized(1) string Open_Animation = "actor_door_dungeon_up_open";
	
	[ 
		EditType("Audio"),
		Description("Sound to play when opening or closing door.")
	]
	public serialized(1) string Open_Sound = "sfx_objects_doors_place";
	
	[ 
		Description("True if this door is currently open or not.")
	]
	public serialized(1) bool Is_Open = true;
		
	[ 
		Description("True if this door is locked and can be opened by the player.")
	]
	public serialized(1) bool Is_Locked = false;
		
	[ 
		Name("Key Type"), 
		Description("Type of key required to unlock this door.") 
	]
	public serialized(1) string Key_Type = "Key_Pickup";	

	[ 
		Description("Fraction of total height to use for collision height")
	]
	public serialized(101) float Collision_Height_Fraction = 0.25;
    
	[ 
		Description("If set the door referenced by this string is considered the 'partner' of this door, and use when deciding how to push player out of locked coridoors.")
	]
	public serialized(2500) string Partner_Door = "";

    private bool bSealed = false;
     
	private int m_last_open_state = -1;

	Dungeon_Door()
	{
		Bounding_Box	= Vec4(0, 0, 48, 48);
		collision.Area 	= Vec4(0, 0, 48, 48);
		Store_Initial_State();
	}

	private void Store_Initial_State()
	{		
		bool bOpen = (Is_Open && !Is_Locked);
		//m_last_open_state = bOpen;

		Update_State();
	}

	private void Update_State()
	{
		bool bOpen = (Is_Open && !Is_Locked);

		if (bOpen)
		{
			collision.Type	= CollisionType.Non_Solid;		
			collision.Group	= CollisionGroup.Volume;
		}
		else
		{
			collision.Type	= CollisionType.Solid;		
			collision.Group	= CollisionGroup.Environment;
			collision.Collides_With	= CollisionGroup.Player | CollisionGroup.Enemy;
		}				
		
		//	collision.Area = Vec4(16.0f, 16.0f, 16.0f, 16.0f);
		if (Open_Animation.Contains("_up_") || Open_Animation.Contains("_down_"))
		{
			collision.Area = Vec4(0.0f, 16.0f, 48.0f, 16.0f);
		//    sprite.Depth_Bias = this.Bounding_Box.Height;//collision.Area.Y + (collision.Area.Height * 0.5f);
		}
		else
		{
			collision.Area = Vec4(16.0f, 0.0f, 16.0f, 48.0f);		
		}
		sprite.Depth_Bias = -64.0f;
		
		sprite.Animation = Open_Animation;
		sprite.Animation_Speed = (bOpen) ? 1.0f : -1.0f;
		sprite.Frame = "";

		Log.Write("Playing animation (Is_Open="+Is_Open+") "+sprite.Animation+" with speed "+sprite.Animation_Speed);
	}

	default state Idle
	{
		event void On_Enter()
		{
			Store_Initial_State();
		}
		event void On_Editor_Property_Change()
		{
			Store_Initial_State();
		}
		event void On_Tick()
		{
		    bool bOpen = (Is_Open && !Is_Locked);

          //  if (Net_ID == 242)
           // {
            //    Log.Write("Ticking invisible dungeon door! bOpen="+bOpen+" Last="+m_last_open_state+" Anim="+Open_Animation);
            //}

			if (m_last_open_state != <int>bOpen)
			{
				Audio.Play3D(Open_Sound, this.Center);
				Update_State();
				m_last_open_state = <int>bOpen;
            }

            if (!bOpen && bSealed && Partner_Door != "")
            {
                Force_Players_Out_Of_Coridoor();
            }
		}
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
		if (!Is_Locked)
		{
			return false;
		}

		Inventory_Pickup pickup = Inventory_Pickup.Get_Inventory_Item_For_Net_ID(other.Net_ID);
		return (pickup != null && pickup.GetType().Name == Key_Type);
	}

	public override void Interact(Pawn other)
	{		
		rpc(RPCTarget.Server, RPCTarget.None) RPC_Toggle(other.Net_ID);
	}

	public rpc void RPC_Toggle(int id)
	{
		Pawn other = <Pawn>Scene.Get_Net_ID_Actor(id);
		if (other != null && Is_Locked)
		{
			// Does user have the item required?
			Inventory_Pickup pickup = Inventory_Pickup.Get_Inventory_Item_For_Net_ID(other.Net_ID);
		
			if (pickup != null)
			{
				pickup.Reset();
				pickup.Despawn();

				Log.Write(other.Net_ID +  " unlocked door " + this.Net_ID + ".");
		
				Is_Open = true;
				Is_Locked = false;
			}
		}
	}

	private override void On_Trigger()
	{
		Is_Open = !Is_Open;
	}

    public void Unseal_Self_And_Neighbors()
    {
        Is_Open = true;
        bSealed = false;

        Actor[] doors = Scene.Find_Actors_By_Tag(Partner_Door);
        if (doors.Length() <= 0)
        {
            return;
        }

        Dungeon_Door partner = <Dungeon_Door>doors[0];
        if (partner)
        {
            partner.Is_Open = true;
            partner.bSealed = false;
        } 
    }

    public void Seal_Self_And_Neighbors()
    {
        Is_Open = false;
        bSealed = true;

        Actor[] doors = Scene.Find_Actors_By_Tag(Partner_Door);
        if (doors.Length() <= 0)
        {
            return;
        }

        Dungeon_Door partner = <Dungeon_Door>doors[0];
        if (partner)
        {
            partner.Is_Open = false;
            partner.bSealed = true;
        }
    }

    private void Force_Players_Out_Of_Coridoor()
    {
		Vec4 out_of_bounds_area = this.World_Collision_Box;

        Actor[] doors = Scene.Find_Actors_By_Tag(Partner_Door);
        if (doors.Length() > 0)
        {
            Dungeon_Door partner = <Dungeon_Door>doors[0];
            if (partner)
            {
                out_of_bounds_area = out_of_bounds_area.RectUnion(partner.World_Collision_Box);
            }
        } 
        
		Actor[] pawns = Scene.Find_Actors_With_Collision_In_Area(typeof(Pawn), out_of_bounds_area);
        if (pawns.Length() > 0)
        {       
		    Log.Write("Forcing "+pawns.Length()+" out of door coridoor.");
         
			// Work out if we are horizontal or vertical to work out which direction to push.
			bool bIsVertical = (sprite.Animation.Contains("_up_") || sprite.Animation.Contains("_down_"));
            
			foreach (Pawn pawn in pawns)
			{
                if ((pawn.Owner != null && pawn.Owner.Is_Local) ||
                    (pawn.Owner == null && Network.Is_Server()))
                {
					Log.Write("Local pawn "+pawn.Net_ID+" is inside door coridoor that has just closed, pushing pawn out.");

                    Vec4 pawn_collision_box = pawn.World_Collision_Box;
					Vec2 offset = Vec2(0.0f, 0.0f);

					// Work out how far to push to get pawn out of our collision.
					if (bIsVertical)
					{
						// Pawn lower than door, push out downwards.
						if (pawn_collision_box.Center().Y > out_of_bounds_area.Center().Y)
						{
							float pawn_top = pawn_collision_box.Y;
							float our_bottom = out_of_bounds_area.Y + out_of_bounds_area.Height;
							offset = Vec2(0.0f, (our_bottom - pawn_top) + 1);
						}
						// Push pawn out upwards.
						else
						{
							float pawn_bottom = pawn_collision_box.Y + pawn_collision_box.Height;
							float our_top = out_of_bounds_area.Y;
							offset = Vec2(0.0f, (our_top - pawn_bottom) - 1);
						}
					}
					else 
					{
						// Pawn lower than door, push out right.
						if (pawn_collision_box.Center().X > out_of_bounds_area.Center().X)
						{
							float pawn_left = pawn_collision_box.X;
							float our_right = out_of_bounds_area.X + out_of_bounds_area.Width;
							offset = Vec2((our_right - pawn_left) + 1, 0.0f);
						}
						// Push pawn out to the left.
						else
						{		
							float pawn_right = pawn_collision_box.X + pawn_collision_box.Width;
							float our_left = out_of_bounds_area.X;
							offset = Vec2((our_left - pawn_right) - 1, 0.0f);
						}
					}

					pawn.Teleport(pawn.Position + Vec3(offset, 0.0f));
                }
            }
        }
    }

    /*
	// If any locally-controlled pawns are inside our bounds, this function will force them out in the appropriate direction.
	private void Force_Pawns_Out_Of_Bounds()
	{		
		Vec4 collision_box = this.World_Collision_Box;

		Actor[] pawns = Scene.Find_Actors_With_Collision_In_Area(typeof(Pawn), collision_box);
		Log.Write("Forcing "+pawns.Length()+" out of door collision.");

		if (pawns.Length() > 0)
		{
			// Work out if we are horizontal or vertical to work out which direction to push.
			bool bIsVertical = (sprite.Animation.Contains("_up_") || sprite.Animation.Contains("_down_"));

			foreach (Pawn pawn in pawns)
			{
				if ((pawn.Owner != null && pawn.Owner.Is_Local) || 
					(pawn.Owner == null && Network.Is_Server()))
				{				
					Log.Write("Local pawn "+pawn.Net_ID+" is inside door that has just closed, pushing pawn out.");

					Vec4 pawn_collision_box = pawn.World_Collision_Box;
					Vec2 offset = Vec2(0.0f, 0.0f);

					Log.Write("door="+collision_box.X+","+collision_box.Y+","+collision_box.Width+","+collision_box.Height+" pawn="+pawn_collision_box.X+","+pawn_collision_box.Y+","+pawn_collision_box.Width+","+pawn_collision_box.Height);

					// Work out how far to push to get pawn out of our collision.
					if (bIsVertical)
					{
						// Pawn lower than door, push out downwards.
						if (pawn_collision_box.Center().Y > collision_box.Center().Y)
						{
							float pawn_top = pawn_collision_box.Y;
							float our_bottom = collision_box.Y + collision_box.Height;
							offset = Vec2(0.0f, (our_bottom - pawn_top) + 1);
						}
						// Push pawn out upwards.
						else
						{
							float pawn_bottom = pawn_collision_box.Y + pawn_collision_box.Height;
							float our_top = collision_box.Y;
							offset = Vec2(0.0f, (our_top - pawn_bottom) - 1);
						}
					}
					else 
					{
						// Pawn lower than door, push out right.
						if (pawn_collision_box.Center().X > collision_box.Center().X)
						{
							float pawn_left = pawn_collision_box.X;
							float our_right = collision_box.X + collision_box.Width;
							offset = Vec2((our_right - pawn_left) + 1, 0.0f);
						}
						// Push pawn out to the left.
						else
						{		
							float pawn_right = pawn_collision_box.X + pawn_collision_box.Width;
							float our_left = collision_box.X;
							offset = Vec2((our_left - pawn_right) - 1, 0.0f);
						}
					}

					pawn.Teleport(pawn.Position + Vec3(offset, 0.0f));
				}
			}
		}
	}
    */

	replicate(ReplicationPriority.High)
	{	
		Is_Open
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		Is_Locked
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		bSealed
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "actor_door_dungeon_up_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
		serialized(50) CollisionComponent collision
		{
			Shape 				= CollisionShape.Rectangle;
			Area 				= Vec4(0, 0, 64, 37);
			Type				= CollisionType.Solid;		
			Group				= CollisionGroup.Environment;
			Collides_With		= CollisionGroup.Player | CollisionGroup.Enemy;
			Blocks_Path_Spawns	= true;
		}
	}
}