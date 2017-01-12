// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.components.collision;
using system.actors.actor;
using system.scene;
using system.time;

[
	Placeable(true), 
	Name("Dungeon Arena Spawn Point"), 
	Description("Position in a dungeon arena that enemies should be spawned in.") 
]
public class Dungeon_Arena_Spawn_Point : Actor
{
	Dungeon_Arena_Spawn_Point()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	default state Idle
	{
		event void On_Enter()
		{
			Hibernate();
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame 			= "actor_spawn_marker_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}

[
	Placeable(true), 
	Name("Dungeon Arena Trigger"), 
	Description("When the player enters these bounds the arena will trigger.") 
]
public class Dungeon_Arena_Trigger : Actor
{	
	Dungeon_Arena_Trigger() 
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	default state Idle
	{
		event void On_Editor_Property_Change()
		{
			collision.Area 	= Bounding_Box;
		}
		event void On_Touch(CollisionComponent component, Actor other)
		{
			if (Network.Is_Server())
			{
				if (other.Owner != null)
				{
					Human_Player human = <Human_Player>other;
					if (human != null && !human.Is_Incapacitated())
					{
						change_state(Triggered);
					}
				}
			}
		}
	}
		
	state Triggered
	{
		event void On_Enter()
		{			
			Actor[] arenas = Scene.Find_Actors_In_Area(typeof(Dungeon_Arena), this.World_Bounding_Box);
			foreach (Dungeon_Arena arena in arenas)
			{
				arena.Begin();
			}
			Hibernate();
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(255.0, 0.0, 0.0, 128.0);
			Scale = true;
			Visible = false;
			Editor_Visible = true;
		}
		serialized(50) CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 64, 64);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.Player;
		}
	}
}

[
	Placeable(true), 
	Name("Dungeon Arena"), 
	Description("Defines the bounds of a room that will act as an arena - doors seal, enemies spawn. Enemies spawn from fixed patterns defined with Dungeon_Arena_Spawn_Point if they exist in bounds or using wave mechanics otherwise.") 
]
public class Dungeon_Arena : Actor
{	
	// -------------------------------------------------------------------------------------------------
	// Spawn types.
	// -------------------------------------------------------------------------------------------------
	public serialized(1) bool Spawn_Fodder			= false;
	public serialized(1) bool Spawn_Chaser			= true;
	public serialized(1) bool Spawn_Exploder		= false;
	public serialized(1) bool Spawn_Bouncer			= false;
	public serialized(1) bool Spawn_Small_Blobby	= false;
	public serialized(1) bool Spawn_Large_Blobby	= false;
	public serialized(700) bool Spawn_Floater		= false;

	// -------------------------------------------------------------------------------------------------
	// Wave types
	// -------------------------------------------------------------------------------------------------
	public serialized(1) bool Use_Wave				= false; // Unused
	public serialized(1) int  Min_Wave_Count		= 8;
	public serialized(1) int  Max_Wave_Count		= 32;

	public serialized(10) int Max_Enemy_Limit		= 8;
	public serialized(10) float Min_Spawn_Interval	= 0.2f;
	public serialized(10) float Max_Spawn_Interval	= 0.2f;

	private float m_time_until_spawn = 0.0f;
	private Actor[] m_spawn_points;
	private int m_spawns_remaining = 0;
	private int m_spawn_cycle_index;

	private Enemy_Type[] m_enemy_types;

	public Vec4 Dungeon_Door_Border = new Vec4(-16.0f, -16.0f, 32.0f, 32.0f);

	[ Is_Link(1) ]
	public serialized(600) string Finished_Link = "";

	[ Is_Link(1) ]
	public serialized(600) string Started_Link = "";

	// -------------------------------------------------------------------------------------------------
	// State
	// -------------------------------------------------------------------------------------------------
	public serialized(1) bool Has_Triggered			= false;

	Dungeon_Arena() 
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	void Begin()
	{
		if (!Has_Triggered)
		{
			change_state(Spawning);
		}
	}

	public override void On_Trigger()
	{
		Begin();
	}

	public void Begin_Music()
	{		
		Actor[] rooms = Scene.Find_Actors_In_Area(typeof(DC_Room_Marker), this.World_Bounding_Box);
		bool bBoss = false;

		if (rooms.Length() > 0)
		{
			DC_Room_Marker marker = <DC_Room_Marker>rooms[0];
			bBoss = (marker.Room_Type == DC_Room_Type.Boss);
		}

		if (bBoss)
		{
			Audio.Push_BGM("music_bossu");
		}
		else
		{
			Audio.Push_BGM("music_fastbeat");
		}
	}

	public void Finish_Music()
	{
		Audio.Pop_BGM();
	}

	default state Idle
	{
	}

	state Spawning
	{
		event void On_Enter()
		{
			Log.Write("Arena mode triggered, starting spawning!");
			 
			// Lock all the doors in the arena.
			Actor[] doors = Scene.Find_Actors_In_Area(typeof(Dungeon_Door), this.World_Bounding_Box + Dungeon_Door_Border);
			Log.Write("Sealing "+doors.Length()+" doors.");
			foreach (Dungeon_Door door in doors)
			{
                door.Seal_Self_And_Neighbors();
			}

			m_spawns_remaining = Math.Rand(Min_Wave_Count, Max_Wave_Count);					
			m_spawn_points = Scene.Find_Actors_In_Area(typeof(Dungeon_Arena_Spawn_Point), this.World_Bounding_Box);

			m_enemy_types = new Enemy_Type[0];
			if (Spawn_Fodder)			m_enemy_types.AddLast(Enemy_Type.Fodder);	
			if (Spawn_Chaser)			m_enemy_types.AddLast(Enemy_Type.Chaser);
			if (Spawn_Exploder)			m_enemy_types.AddLast(Enemy_Type.Exploder);
			if (Spawn_Bouncer)			m_enemy_types.AddLast(Enemy_Type.Bouncer);
			if (Spawn_Small_Blobby)		m_enemy_types.AddLast(Enemy_Type.Small_Blobby);
			if (Spawn_Large_Blobby)		m_enemy_types.AddLast(Enemy_Type.Large_Blobby);
			if (Spawn_Floater)			m_enemy_types.AddLast(Enemy_Type.Floater);
									
			Scene.Trigger_Actors(Started_Link);

			Begin_Music();

			Has_Triggered = true;
		}
		event void On_Tick()
		{
			m_time_until_spawn -= Time.Get_Delta_Seconds();

			Actor[] enemies = Scene.Find_Actors_With_Collision_In_Area(typeof(Enemy), this.World_Bounding_Box);			
			Actor[] humans = Scene.Find_Actors_With_Collision_In_Area(typeof(Human_Player), this.World_Bounding_Box);

			bool bValidHumans = false;

			foreach (Human_Player human in humans)
			{
				if (!human.Is_Incapacitated())
				{
					bValidHumans = true;
					break;
				}
			}
    
			//Log.Write("[Arena] Enemies="+enemies.Length()+" Humans="+humans.Length());

			if (!bValidHumans)
			{
				Log.Write("Player died during arena mode, resetting.");
						
				Scene.Trigger_Actors(Finished_Link);

				// Despawn all enemies.
				foreach (Enemy e in enemies)
				{
					e.Magic_Despawn();
				}

				// Umlock all the doors in the arena.
				Actor[] doors = Scene.Find_Actors_In_Area(typeof(Dungeon_Door), this.World_Bounding_Box + Dungeon_Door_Border);
				Log.Write("Unlocking "+doors.Length()+" doors.");
				foreach (Dungeon_Door door in doors)
				{
                    door.Unseal_Self_And_Neighbors();
				}

				Has_Triggered = false;
				
				Finish_Music();

				change_state(Idle);
			}
			else
			{
				if (m_time_until_spawn < 0.0f && m_spawns_remaining > 0)
				{
					if (enemies.Length() < Max_Enemy_Limit)
					{	
						if (m_spawn_points.Length() > 0)
						{
							// Select next spawn point.
							Dungeon_Arena_Spawn_Point point = null;

							int counter = 0;
							while (true)
							{
								point = <Dungeon_Arena_Spawn_Point>m_spawn_points[(m_spawn_cycle_index + counter) % m_spawn_points.Length()];
								float dist = Scene.Distance_To_Closest_Actor_Of_Type(point.Center, typeof(Human_Player));
								if (dist > 64.0f)
								{
									break;
								}
								if (counter >= m_spawn_points.Length())
								{
									break;
								}
								counter++;
							}

							if (point != null)
							{
								// Spawn a random enemy!
								Enemy_Type enemy_type = m_enemy_types[Math.Rand(0, m_enemy_types.Length())];
								Pawn pwn = Enemy_Manager.Spawn_Enemy(enemy_type, point.World_Bounding_Box, 2, Enemy_Manager.Default_Team_Index);
								if (pwn != null)
								{
									Log.Write("Setting pawn as magic spawn.");
									(<Enemy>pwn).Parent_Arena = this;
									(<Enemy>pwn).Spawned_By_Magic = true;
								}

								// Do some tracking.
								m_spawn_cycle_index = (m_spawn_cycle_index + 1) % m_spawn_points.Length();
								m_time_until_spawn = Math.Rand(Min_Spawn_Interval, Max_Spawn_Interval);
								m_spawns_remaining--;

								Log.Write("Spawning random enemy on point "+point.Position.X+","+point.Position.Y+" next spawn in "+m_time_until_spawn);
							}
						}
					}
				}
				else
				{
					if (m_spawns_remaining <= 0 && enemies.Length() <= 0)
					{
						Log.Write("Arena mode completed.");
								
						Scene.Trigger_Actors(Finished_Link);

						// Umlock all the doors in the arena.
						Actor[] doors = Scene.Find_Actors_In_Area(typeof(Dungeon_Door), this.World_Bounding_Box + Dungeon_Door_Border);
						Log.Write("Unlocking "+doors.Length()+" doors.");
						foreach (Dungeon_Door door in doors)
						{
                            door.Unseal_Self_And_Neighbors();
						}
			
						Finish_Music();

						change_state(Idle);
					}
				}
			}
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 128.0, 0.0, 128.0);
			Scale = true;
			Visible = false;
			Editor_Visible = true;
		}
	}
}
