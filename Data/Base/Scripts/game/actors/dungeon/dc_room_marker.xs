// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.components.collision;
using system.actors.actor;
using system.scene;
using system.time;

// Must match native enum MapRandGenRoomType.
public enum DC_Room_Type
{
	Arena			= 0,			// Doors lock, enemies spawn, no continue until all dead.
	Boss			= 1,			// Doors lock, epic boss spawns, no continue until dead. Next room is usually end.
	Corridoor		= 2,			// Just a bit of walking, maybe a trap or two.
	Start			= 3,			// Starting room.
	End				= 4,			// Ending room.
	Resupply		= 5,			// Weapon/ammo resupply room.
	Treasure		= 6,			// Random treasure room.
	Boss_Treasure	= 7				// Big boss treasure room.
}

// This is a special interface. If an object derives from this it will be hibernated/woken up only when
// the player is near the dungeon room the object is contained within.
public interface IDungeonRoomHibernatedObject
{
	public abstract void On_Hibernate();
	public abstract void On_Wakeup(); 
}

[
	Placeable(true), 
	Name("DC Room Marker"), 
	Description("Defines the bounds of a room template for the dungeon-crawler rand-gen.") 
]
public class DC_Room_Marker : Actor
{	
	[ 
		Name("Room Type"),
		Description("Room type identifier, needs to match native enum MapRandGenRoomType.")
	]
	public serialized(150) DC_Room_Type Room_Type = DC_Room_Type.Corridoor;

	[ 
		Name("Maximum Room Instances"),
		Description("Maximum number of instances of this room. 0 for unlimited.")
	]
	public serialized(700) int Maximum_Instances = 0;
	
	private bool m_initial_update = true;
	private bool m_last_players_close = false;

	public bool Has_Been_Visited = false;

	DC_Room_Marker() 
	{
		Bounding_Box = Vec4(0, 0, 64, 64);
	}

	default state Idle
	{
		public event void On_Tick()
		{	
			// Track if users have visited us.
			if (!Has_Been_Visited)
			{
				if (Scene.Are_Actors_Colliding(typeof(Human_Player), this.World_Bounding_Box))
				{
					Has_Been_Visited = true;
				}
			}

			// Wakeup/hibernate objects if players are getting close to us.
			Vec4 bbox_area = this.World_Bounding_Box + Vec4(-128.0f, -128.0f, 256.0f, 256.0f);
			Actor[] arenas = Scene.Find_Actors_In_Area(typeof(Human_Player), bbox_area);
			bool bPlayersClose = arenas.Length() > 0;

			this.Tick_Area = bbox_area;

			if (bPlayersClose != m_last_players_close || m_initial_update)
			{
				Actor[] objs = Scene.Find_Actors_In_Area(typeof(IDungeonRoomHibernatedObject), this.World_Bounding_Box);
				if (objs.Length() > 0)
				{
					if (bPlayersClose)
					{
						Log.Write("Waking up "+objs.Length()+" room objects.");
					}
					else
					{
						Log.Write("Hibernating "+objs.Length()+" room objects.");
					}

					foreach (Actor obj in objs)
					{
						IDungeonRoomHibernatedObject hiber_object = <IDungeonRoomHibernatedObject>obj;

						if (bPlayersClose)
						{
							obj.Wake_Up();
							hiber_object.On_Wakeup();
						}
						else
						{
							hiber_object.On_Hibernate();
							obj.Hibernate();
						}
					} 
				}

				m_initial_update = false;
				m_last_players_close = bPlayersClose;
			}
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(128.0, 0.0, 128.0, 128.0);
			Scale = true;
			Visible = false;
			Editor_Visible = true;
		}
	}
}
