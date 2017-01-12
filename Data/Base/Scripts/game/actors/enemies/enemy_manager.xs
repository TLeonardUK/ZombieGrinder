// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.enemies.bouncer_zombie;
using game.actors.enemies.chaser_zombie;
using game.actors.enemies.exploder_zombie;
using game.actors.enemies.fodder_zombie;
using game.actors.enemies.small_blobby_zombie;
using game.actors.enemies.large_blobby_zombie;
using game.actors.enemies.floater_zombie;
using game.actors.controllers.ai_bouncer_zombie_controller;
using game.actors.controllers.ai_exploder_zombie_controller;
using game.actors.controllers.ai_fodder_zombie_controller;
using game.actors.controllers.ai_chaser_zombie_controller;
using game.actors.controllers.ai_small_blobby_zombie_controller;
using game.actors.controllers.ai_large_blobby_zombie_controller;
using game.actors.controllers.ai_floater_zombie_controller;
using game.actors.enemies.enemy_spawn_zone;
using system.time;
using system.scene;

// Warning: Update Spawn_Info table in Enemy_Manager if you change this.
public enum Enemy_Type
{
	Fodder,			// Grey fodder zombies, dumb as fuck and just there for points.
	Chaser,			// Green zombies that spawn randomly and chase after your ass.
	Exploder,		// Exploding zombies, best stay clear!
	Bouncer,		// Bouncing zombies, these hurt!
	Small_Blobby,	// Small blobby enemy.
	Large_Blobby,	// Large blobby enemy.
	Floater,		// Floaty pewpew zombie.

    // Bosses
    Boss_Whipper,   // Whipping boss!

	COUNT
}

public class Enemy_Spawn_Probability_Table
{
    public serialized(8000) int Fodder;
    public serialized(8000) int Chaser;
    public serialized(8000) int Exploder;
    public serialized(8000) int Bouncer;
    public serialized(8000) int Small_Blobby;
    public serialized(8000) int Large_Blobby;
    public serialized(8000) int Floater;

    public serialized(8000) int Whipper;

    private Integer_Drop_Table m_drop_table = null;

    public Enemy_Type Get_Random_Enemy_Type()
    {
        if (m_drop_table == null)
        {
            m_drop_table = new Integer_Drop_Table();
            m_drop_table.Add(Enemy_Type.Fodder, Fodder);
            m_drop_table.Add(Enemy_Type.Chaser, Chaser);
            m_drop_table.Add(Enemy_Type.Exploder, Exploder);
            m_drop_table.Add(Enemy_Type.Bouncer, Bouncer);
            m_drop_table.Add(Enemy_Type.Small_Blobby, Small_Blobby);
            m_drop_table.Add(Enemy_Type.Large_Blobby, Large_Blobby);
            m_drop_table.Add(Enemy_Type.Floater, Floater);
            m_drop_table.Add(Enemy_Type.Boss_Whipper, Whipper);
        }
        return <Enemy_Type>m_drop_table.Get_Drop();
    }
}

public class Enemy_Spawn_State_Table
{
    public serialized(8000) bool Fodder;
    public serialized(8000) bool Chaser;
    public serialized(8000) bool Exploder;
    public serialized(8000) bool Bouncer;
    public serialized(8000) bool Small_Blobby;
    public serialized(8000) bool Large_Blobby;
    public serialized(8000) bool Floater;

    public serialized(8000) bool Whipper;
}

public class Enemy_Spawn_Info
{
	// ------------------------------------------------------------------------------
	// Configuration set in derived classes
	// ------------------------------------------------------------------------------

	// Unbreakable max limits of enemies, we will not spawn above this.
	public int Max_Enemy_Limit		= 1; 
	// Limit of zombies to be spawned, this scales with difficulty up to the Max_Enemy_Limit.
	public int Base_Spawn_Limit		= 5;

	// Absolute limit to interval between spawns.
	public float Min_Spawn_Interval			= 0.1f;
	// Interval between spawning each enemy type, this scales with difficulty down to Min_Spawn_Interval.
	public float Base_Min_Spawn_Interval	= 5;
	// Interval between spawning each enemy type, this scales with difficulty down to Max_Spawn_Interval.
	public float Base_Max_Spawn_Interval	= 10.0f;

	// Distance at which these enemies despawn. 0 for never.
	public float Despawn_Distance			= 0;

	// If set this enemy type is active and good for spawning.
	public bool Active						= false;

	// Type of pawn to spawn.
	public Type Pawn_Type					= null;

	// Type of controller to spawn.
	public Type Controller_Type				= null;

	// Type of controller to spawn for human control.
	public Type Player_Controller_Type				= null;

	// If set the enemies are spawned based on free path-nodes, not based on
	// spawn-zones.
	public bool Force_Path_Based_Spawns		= false;
	
	// If true the enemies can spawn in view of players.
	public bool Can_Spawn_On_Screen			= false;

	// Counts towards wave counter.
	public bool Count_Towards_Waves			= true;

	// Area required to spawn this enemy.
	public Vec4 Spawn_Area_Required			= new Vec4(0.0f, 0.0f, 16.0f, 16.0f);
	
	// Min/Max areas used when choosing path based spawns.
	public float Spawn_Min_Distance			= 128.0f;
	public float Spawn_Max_Distance			= 600.0f;

	// Minimum difficulty scalar before these enemies start to show up.
	public float Min_Difficulty_Scalar		= 0.0f;

    // Scales up based on fear, this should basically be set for everything but fodder/boss zombies.
    public bool Responds_To_Fear            = false;

	// ------------------------------------------------------------------------------
	// Runtime control.
	// ------------------------------------------------------------------------------
	public float m_time_till_next_spawn		= 0.0f;
}
 
public class Fodder_Spawn_Info : Enemy_Spawn_Info
{
	Fodder_Spawn_Info()
	{
		Max_Enemy_Limit			= 20;
		Base_Spawn_Limit		= 6;

		Min_Spawn_Interval		= 1.0f;
		Base_Min_Spawn_Interval	= 0.5f;
		Base_Max_Spawn_Interval	= 1.0f;
		
		Despawn_Distance		= 900.0f;

		Active					= true; 
		Force_Path_Based_Spawns	= true;
		Count_Towards_Waves		= false;

		Min_Difficulty_Scalar	= 0.0f;

		Pawn_Type				= typeof(Fodder_Zombie);
		Controller_Type			= typeof(AI_Fodder_Zombie_Controller);
        Player_Controller_Type  = null;

        Responds_To_Fear        = false;
	}
}

public class Chaser_Spawn_Info : Enemy_Spawn_Info
{
	Chaser_Spawn_Info()
	{
		Max_Enemy_Limit			= 20;
		Base_Spawn_Limit		= 3;

		Min_Spawn_Interval		= 2.0f;
		Base_Min_Spawn_Interval	= 10.0f;
		Base_Max_Spawn_Interval	= 15.0f;

		Spawn_Min_Distance		= 72.0f;

		Can_Spawn_On_Screen		= true;
		
		Despawn_Distance		= 900.0f;

		Active					= true;
		
		Min_Difficulty_Scalar	= 0.0f;

		Pawn_Type				= typeof(Chaser_Zombie);
		Controller_Type			= typeof(AI_Chaser_Zombie_Controller);
        Player_Controller_Type  = typeof(Player_Chaser_Zombie_Controller);

        Responds_To_Fear        = true;
	}
}

public class Exploder_Spawn_Info : Enemy_Spawn_Info
{
	Exploder_Spawn_Info()
	{
		Max_Enemy_Limit			= 10;
		Base_Spawn_Limit		= 1;

		Min_Spawn_Interval		= 2.0f;
		Base_Min_Spawn_Interval	= 15.0f;
		Base_Max_Spawn_Interval	= 20.0f;
		
		Despawn_Distance		= 900.0f;

		Active					= true;
		
		Min_Difficulty_Scalar	= 4.0f;

		Pawn_Type				= typeof(Exploder_Zombie);
		Controller_Type			= typeof(AI_Exploder_Zombie_Controller);
        Player_Controller_Type  = typeof(Player_Exploder_Zombie_Controller);

        Responds_To_Fear        = true;
	}
}

public class Bouncer_Spawn_Info : Enemy_Spawn_Info
{
	Bouncer_Spawn_Info()
	{
		Max_Enemy_Limit			= 5;
		Base_Spawn_Limit		= 1;

		Min_Spawn_Interval		= 5.0f;
		Base_Min_Spawn_Interval	= 20.0f;
		Base_Max_Spawn_Interval	= 30.0f;
		
		Despawn_Distance		= 900.0f;

		Active					= true;
		
		Min_Difficulty_Scalar	= 20.0f;

		Pawn_Type				= typeof(Bouncer_Zombie);
		Controller_Type			= typeof(AI_Bouncer_Zombie_Controller);
        Player_Controller_Type  = typeof(Player_Bouncer_Zombie_Controller);

        Responds_To_Fear        = true;
	}
}

public class Small_Blobby_Spawn_Info : Enemy_Spawn_Info
{
	Small_Blobby_Spawn_Info()
	{
		Max_Enemy_Limit			= 10;
		Base_Spawn_Limit		= 1;
		
		Min_Spawn_Interval		= 2.0f;
		Base_Min_Spawn_Interval	= 10.0f;
		Base_Max_Spawn_Interval	= 15.0f;
		
		Despawn_Distance		= 900.0f;

		Active					= true;
		
		Min_Difficulty_Scalar	= 7.0f;

		Pawn_Type				= typeof(Small_Blobby_Zombie);
		Controller_Type			= typeof(AI_Small_Blobby_Zombie_Controller);
        Player_Controller_Type  = typeof(Player_Small_Blobby_Zombie_Controller);

        Responds_To_Fear        = true;
	}
}

public class Large_Blobby_Spawn_Info : Enemy_Spawn_Info
{
	Large_Blobby_Spawn_Info()
	{
		Max_Enemy_Limit			= 5;
		Base_Spawn_Limit		= 1;
		
		Min_Spawn_Interval		= 5.0f;
		Base_Min_Spawn_Interval	= 15.0f;
		Base_Max_Spawn_Interval	= 20.0f;
		
		Despawn_Distance		= 900.0f;

		Active					= true;
		
		Min_Difficulty_Scalar	= 15.0f;

		Pawn_Type				= typeof(Large_Blobby_Zombie);
		Controller_Type			= typeof(AI_Large_Blobby_Zombie_Controller);
        Player_Controller_Type  = typeof(Player_Large_Blobby_Zombie_Controller);

        Responds_To_Fear        = true;
	}
}

public class Floater_Spawn_Info : Enemy_Spawn_Info
{
	Floater_Spawn_Info()
	{
		Max_Enemy_Limit			= 5;
		Base_Spawn_Limit		= 1;

		Min_Spawn_Interval		= 2.0f;
		Base_Min_Spawn_Interval	= 15.0f;
		Base_Max_Spawn_Interval	= 20.0f;
		
		Despawn_Distance		= 900.0f;

		Active					= true;
		
		Min_Difficulty_Scalar	= 17.0f;

		Pawn_Type				= typeof(Floater_Zombie);
		Controller_Type			= typeof(AI_Floater_Zombie_Controller);
        Player_Controller_Type  = typeof(Player_Floater_Zombie_Controller);

        Responds_To_Fear        = true;
	}
}

public class Boss_Whipper_Spawn_Info : Enemy_Spawn_Info
{
	Boss_Whipper_Spawn_Info()
	{
		Max_Enemy_Limit			= 1;
		Base_Spawn_Limit		= 1;

		Min_Spawn_Interval		= 0.0f;
		Base_Min_Spawn_Interval	= 0.0f;
		Base_Max_Spawn_Interval	= 0.0f;
		
		Despawn_Distance		= 0.0f;

		Active					= false;
		
		Min_Difficulty_Scalar	= 0.0f;

		Pawn_Type				= typeof(Boss_Whipper);
		Controller_Type			= typeof(AI_Boss_Whipper_Controller);
        Player_Controller_Type  = null;

        Responds_To_Fear        = false;
	}
}

[
	Name("Enemy Manager"), 
	Description("Controls all aspects of spawning and despawning of enemies.") 
]
public static class Enemy_Manager
{		
	// Warning: Update Enemy_Type above if you modify this.
	public static Enemy_Spawn_Info[] Spawn_Info = new Enemy_Spawn_Info[] 
	{
		<Enemy_Spawn_Info>(new Fodder_Spawn_Info()),
		<Enemy_Spawn_Info>(new Chaser_Spawn_Info()),
		<Enemy_Spawn_Info>(new Exploder_Spawn_Info()),
		<Enemy_Spawn_Info>(new Bouncer_Spawn_Info()),
		<Enemy_Spawn_Info>(new Small_Blobby_Spawn_Info()),
		<Enemy_Spawn_Info>(new Large_Blobby_Spawn_Info()),
		<Enemy_Spawn_Info>(new Floater_Spawn_Info()),
		<Enemy_Spawn_Info>(new Boss_Whipper_Spawn_Info())
	};

	private static bool[] m_enemy_types_enabled = 
	{
		true,
		true,
		true,
		true,
		true,
		true,
		true,
        true
	};

	// Number of spawns allow before next reset, negative for infinite.
	private static int m_spawns_remaining = -1;

	// Default team index that enemies are placed into.
	public const int Default_Team_Index = 99;

	// If set enemies are allowed to spawn on path nodes.
	public static bool Allow_Path_Spawns = true;
	
	// Returns true if spawning is permitted
	public static bool Spawning_Enabled = true;

    // Returns true if spawning is permitted
    public static float Spawn_Throttle_Scalar = 1.0f;

    // Fodder spawn throttle scalar.
    public static float Fodder_Spawn_Throttle_Scalar = 1.0f;

	// If true enemy minimum difficulty scalars will be used.
	public static bool Use_Min_Difficulty_Scalars = false;

	// Total max enemies of all kinds.
	public static int Maximum_Enemies = 30; // Does not include fodder enemies.
	
	public static void Reset()
	{
		for (int i = 0; i < m_enemy_types_enabled.Length(); i++)
		{
			m_enemy_types_enabled[i] = true;

			Enemy_Spawn_Info type = Spawn_Info[i];
			type.m_time_till_next_spawn = 0.0f;			
		}
	
		m_spawns_remaining = -1;
		Allow_Path_Spawns = true;
		Spawning_Enabled = true;
        Spawn_Throttle_Scalar = 1.0f;
        Fodder_Spawn_Throttle_Scalar = 1.0f;
		Use_Min_Difficulty_Scalars = false;
		Maximum_Enemies = 30; // Does not include fodder enemies.
	}

	// Finds the best spawn zone to spawn the next enemy in.
	private static Enemy_Spawn_Zone Find_Spawn_Zone(Actor[] zones)
	{
		// Gather all spawn points first.
		Actor[] preferred_actors = new Actor[0];
		Actor[] potential_actors = new Actor[0];

		// Discard those we can't spawn at.
		foreach (Enemy_Spawn_Zone spawn in zones)
		{
			// Got to be enabled.
			if (spawn.Enabled == true)
			{
				// Spawns left?
				if (spawn.Max_Spawns == 0 || spawn.Spawns < spawn.Max_Spawns)
				{
					// Make sure we are offscreen if required.
					if (spawn.Must_Be_Out_Of_Sight == false || !Network.Is_Visible_To_Users(spawn.World_Bounding_Box))
					{
						Path_Pylon pylon = <Path_Pylon>Pathing.Get_Pylon_By_Position(spawn.Center);

						// Make sure we are in a path area thats been touched by the players.
						if (pylon == null || pylon.Activated_By_Player == true)
						{				
							// Preferential treatment if within a reasonable radius to humans.
							if (Scene.Distance_To_Closest_Actor_Of_Type(spawn.Center, typeof(Human_Player)) < spawn.Preferrential_Distance)
							{
								preferred_actors.AddLast(spawn);
							}

							potential_actors.AddLast(spawn);
						}
					}
				}
			}
		}  
		 
		// Nothing available :(
		if (potential_actors.Length() <= 0)
		{
			//Log.Write("No enemy spawns could be found to match criteria!");	
			return null;			
		}
		
		// Grab a random preferred actor.
		if (preferred_actors.Length() > 0)
		{
			return <Enemy_Spawn_Zone>preferred_actors[Math.Rand(0, preferred_actors.Length())];
		}
		// No preferred actors, just choose one of any available.
		else
		{
			return <Enemy_Spawn_Zone>potential_actors[Math.Rand(0, potential_actors.Length())];
		}
	}

    // Can we spawn any enemies that would scale with fear.
    public static bool Can_Spawn_Fear_Enemies()
    {
		if (!Spawning_Enabled)
		{
			return false;
		}

		for (int i = 0; i < Enemy_Type.COUNT; i++)
		{
			Enemy_Spawn_Info type = Spawn_Info[i];
            if (type.Active && m_enemy_types_enabled[i] == true && type.Responds_To_Fear == true)
            {
                return true;
            }
        }

        return false;
    }

	// General spawning of enemies throughout the map. Returns number of zombies spawned.
	public static int Spawn_Enemies()
	{
		float delta_t = Time.Get_Delta();

		int spawns = 0;

        // Find out of any humans want control of zombies.
		Actor[] zones = Scene.Find_Enabled_Actors(typeof(Enemy_Spawn_Zone));
        NetUser[] users = Network.Get_Users();
		float difficulty_scalar = Difficulty_Manager.Base_Difficulty;

        bool bHumansWaitingForControl = false;
        foreach (NetUser user in users)
        {
            if (user.Waiting_For_Zombie_Control && user.Controller == null)
            {
                bHumansWaitingForControl = true;
                break;
            }
        }

        // Try and take over pre-existing enemies.
        if (bHumansWaitingForControl)
        {    
            Pawn[] potential_takeovers = new Pawn[0];
            int[] potential_takeover_types = new int[0];

            // Make a list of all potential enemies to take over.
		    for (int i = 0; i < Enemy_Type.COUNT; i++)
		    {
			    Enemy_Spawn_Info type = Spawn_Info[i];    
                if (type.Pawn_Type != null && type.Player_Controller_Type != null)
                {
				    Actor[] preexisting = Scene.Find_Actors(type.Pawn_Type);
                
                    for (int j = 0; j < preexisting.Length(); j++)
                    {
                        Pawn other = <Pawn>preexisting[j];
                        if (other)
                        {
                            Player_Generic_Zombie_Controller human_controller = <Player_Generic_Zombie_Controller>other.Possessor;
                            if (human_controller == null)
                            {
                                if (other.Possessor && other.Possessor.Is_Safe_To_Switch_Controller())
                                {
                                    potential_takeovers.AddLast(other);
                                    potential_takeover_types.AddLast(i);
                                }
                            }
                        }
                    }
                }
            }

            // Take em over!
            if (potential_takeovers.Length() > 0)
            {
                int index = Math.Rand(0, potential_takeovers.Length() - 1);
                Pawn other = <Pawn>potential_takeovers[index];
                int type_index = potential_takeover_types[index];
                if (other)
                {
                    foreach (NetUser user in users)
                    {
                        if (user.Waiting_For_Zombie_Control && user.Controller == null)
                        {
                            Spawn_Human_Enemy(user, type_index, Vec4(0.0f, 0.0f, 0.0f, 0.0f), 2, Default_Team_Index, other);
                            break;
                        }
                    }
                }
            }
        }
        
		if (!Spawning_Enabled)
		{
			return false;
		}

        if (Cheat_Manager.Disable_Enemy_Spawns)
        {
            return false;
        }

		// No spawns remaining :(.
		if (m_spawns_remaining == 0)
		{
			return 0;
		}

		// To many enemies alive!
		int initial_alive_count = Count_Wave_Alive();
		int allowed_spawns = Maximum_Enemies - initial_alive_count;

		if (allowed_spawns <= 0)
		{
			return 0;
		}

        // Only allow one spawn per frame as the pathing stuff is kinda expensive, fix plz.
        allowed_spawns = 1;

		// Spawn all waiting enemy types.
		for (int i = 0; i < Enemy_Type.COUNT; i++)
		{
			Enemy_Spawn_Info type = Spawn_Info[i];
			type.m_time_till_next_spawn -= delta_t;
            
            // Keep spawning.
            if (Cheat_Manager.Boost_Enemy_Spawn_Counts)
            {
                type.m_time_till_next_spawn = 0.0f;
                if (i != Enemy_Type.Fodder)
                {
                    i++;
                    continue;
                }
            }

			bool bCanSpawn = (type.Active &&
							  m_enemy_types_enabled[i] == true && 
							  type.m_time_till_next_spawn <= 0.0f &&
							  (difficulty_scalar > type.Min_Difficulty_Scalar || !Use_Min_Difficulty_Scalars));

			// Can we spawn?
			if (bCanSpawn)
			{				
				// Enough space for spawns?
				int active_zombies_of_type = Scene.Count_Actors_By_Type(type.Pawn_Type);
				int max_active_zombies_of_type = type.Base_Spawn_Limit * Difficulty_Manager.Enemy_Spawn_Count_Multiplier;

                int max_limit = type.Max_Enemy_Limit;

                if (i == Enemy_Type.Fodder)
                {
                    max_limit *= (1.0f / Fodder_Spawn_Throttle_Scalar);
                    max_active_zombies_of_type *= (1.0f / Fodder_Spawn_Throttle_Scalar);
                }

                // MOAR enemies.
                if (Cheat_Manager.Boost_Enemy_Spawn_Counts)
                {
                    max_limit *= 5;
                    max_active_zombies_of_type *= 5;
                }

                if (Cheat_Manager.MaxEnemies != 0)
                {
                    max_limit = Cheat_Manager.MaxEnemies;
                    max_active_zombies_of_type = Cheat_Manager.MaxEnemies;
                }

				if (active_zombies_of_type < max_limit &&
					active_zombies_of_type < max_active_zombies_of_type)
				{
					bool should_spawn = false;
					bool throttle_spawns = true;

					Vec4 spawn_bbox;
					int spawn_layer;

					// If no zones exist, then we attempt to get a random pathing position (or if we force pathing positions).
					if ((zones.Length() == 0 && Allow_Path_Spawns == true) || type.Force_Path_Based_Spawns == true) 
					{ 
						Path_Spawn_Point point = Pathing.Find_Random_Spawn_Area(type.Spawn_Area_Required, type.Spawn_Min_Distance, type.Spawn_Max_Distance, type.Can_Spawn_On_Screen);
						if (point.Is_Valid == true)
						{
							should_spawn = true;

							spawn_bbox = point.Area;
							spawn_layer = 2; // TODO: Don't hardcode this shit.
						}
					}
					else
					{
						// Find next valid spawn position.
						Enemy_Spawn_Zone zone = Find_Spawn_Zone(zones);
						if (zone != null)
						{
							should_spawn = true;
							spawn_bbox = zone.World_Bounding_Box;
							spawn_layer = zone.Layer;
							throttle_spawns = zone.Throttles_Spawns;

							zone.Spawns++;
							if (zone.Max_Spawns > 0 && zone.Spawns > zone.Max_Spawns)
							{
								zone.Enabled = false;
							}
						}
					}

					// Spawn away.
					if (should_spawn)
					{
                        // Spawn user-controled zombie.
                        bool bSpawned = false;
                        if (bHumansWaitingForControl && type.Player_Controller_Type != null)
                        {
                            foreach (NetUser user in users)
                            {
                                if (user.Waiting_For_Zombie_Control && user.Controller == null)
                                {
						            Spawn_Human_Enemy(user, i, spawn_bbox, spawn_layer, Default_Team_Index, null);
                                    user.Waiting_For_Zombie_Control = false;
                                    break;
                                }
                            }
                        }

                        if (!bSpawned)
                        {
                            if (Cheat_Manager.Spawn_Player_Controlled_Enemies_Only)
                            { 
                                 i++;
                                continue;
                            }
						    Spawn_Enemy(i, spawn_bbox, spawn_layer, Default_Team_Index);
                        }                   

						if (throttle_spawns == true)
						{
                            float scalar = 1.0f;
                            if (i != Enemy_Type.Fodder)
                            {
                                scalar = Spawn_Throttle_Scalar;
                            }
                            else
                            {
                                scalar = Fodder_Spawn_Throttle_Scalar;
                            }

							type.m_time_till_next_spawn = Math.Max(type.Min_Spawn_Interval, Math.Rand(type.Base_Min_Spawn_Interval, type.Base_Max_Spawn_Interval) * Difficulty_Manager.Enemy_Spawn_Interval_Multiplier) * scalar;
						}

						if (type.Count_Towards_Waves)
						{
							m_spawns_remaining--;
							spawns++;

			                // Reduce number of spawns allowed.
			                allowed_spawns--;

			                // Out of spawns!
			                if (m_spawns_remaining == 0 ||
				                allowed_spawns <= 0)
			                {
				                break;
			                }
						}
					}
				}
			}
		}

		return spawns;
	}
	
	// Despawns any enemies out of range. Returns number of zombies despawned.
	public static int Despawn_Enemies(bool bNonWaveOnly = false)
	{
		for (int i = 0; i < Enemy_Type.COUNT; i++)
		{
			Enemy_Spawn_Info type = Spawn_Info[i];
			if (type.Active == true && type.Despawn_Distance > 0.0f && (bNonWaveOnly == false || type.Count_Towards_Waves == false))
			{                
                // Keep spawning.
                float dist = type.Despawn_Distance;
                if (Cheat_Manager.Boost_Enemy_Spawn_Counts)
                {
                    dist /= 2.0f;
                }

				Actor[] actors = Scene.Find_Actors_Distance_From_Cameras(type.Pawn_Type, dist, 9999999.0f);
				foreach (Pawn p in actors)
				{
                    // Ignore owned enemies that players are controlling.
                    if (p.Owner == null)
                    {
					    Log.Write("Despawning out of range enemy: "+p.Net_ID+" position="+p.Position.X+","+p.Position.Y);
					    p.Despawn();
                    }
				}
			}
		}

		return 0;
	}

	// Despawns all enemies.
	public static int Despawn_All_Enemies()
	{
		Actor[] actors = Scene.Find_Actors(typeof(Enemy));
		foreach (Enemy p in actors)
		{
			Log.Write("Despawning enemy: "+p.Net_ID);
			p.Magic_Despawn();
		}
		return 0;
	}
	
	// Sets if the given enemy type is enable or disabled.
	public static void Set_Enemy_Type_Enabled(Enemy_Type type, bool enabled)
	{
		m_enemy_types_enabled[type] = enabled;
	}

	// Sets how many enemies can be spawn during the next spawn round.
	public static void Set_Spawn_Count_Allowed(int counter)
	{
		m_spawns_remaining = counter;
	}

	// Counts how many living enemies there are - this ignores fodder enemies.
	public static int Count_Alive()
	{
		int count = 0;

		for (int i = 0; i < Enemy_Type.COUNT; i++)
		{
			Enemy_Spawn_Info type = Spawn_Info[i];
			if (type.Active == true)
			{
				count += Scene.Count_Actors_By_Type(type.Pawn_Type);
			}
		}

		return count;
	}
	
	// Counts how many living enemies there are - this ignores fodder enemies.
	public static int Count_Wave_Alive()
	{
		int count = 0;

		for (int i = 0; i < Enemy_Type.COUNT; i++)
		{
			Enemy_Spawn_Info type = Spawn_Info[i];
			if (type.Active == true && type.Count_Towards_Waves == true)
			{
				count += Scene.Count_Actors_By_Type(type.Pawn_Type);
			}
		}

		return count;
	}
	
	// Gets a random position to spawn the given bounding box in within our bounds.
	private static Vec3 Random_Spawn_Position_In_Bounds(Vec4 own_bb, Vec4 bbox)
	{
		float range_x = Math.Max(0.0f, own_bb.Width - bbox.Width);
		float range_y = Math.Max(0.0f, own_bb.Height - bbox.Height);
		return Vec3
		(
			(own_bb.X + Math.Rand(0.0f, range_x)) - bbox.X,
			(own_bb.Y + Math.Rand(0.0f, range_y)) - bbox.Y,
			0.0f
		);
	}

    // Spawns a human or normal enemy based on current player states within the given position.
    public static Pawn Spawn_Enemy_Manual(Enemy_Type type_index, Vec4 bounding_box, int layer, int team_index)
    {
		Enemy_Spawn_Info type = Spawn_Info[<int>type_index];  

        // Spawn user-controled zombie.
        if (type.Player_Controller_Type != null)
        {
            NetUser[] users = Network.Get_Users();

            foreach (NetUser user in users)
            {
                if (user.Waiting_For_Zombie_Control && user.Controller == null)
                {
					Pawn pwn = Spawn_Human_Enemy(user, type_index, bounding_box, layer, team_index, null);
                    user.Waiting_For_Zombie_Control = false;
                    return pwn;
                }
            }
        }
        
        if (Cheat_Manager.Spawn_Player_Controlled_Enemies_Only)
        { 
            return null;
        }
		return Spawn_Enemy(type_index, bounding_box, layer, team_index);          
    }

	// Spawns an enemy at the given position with the given controller.
	public static Pawn Spawn_Enemy(Enemy_Type type_index, Vec4 bounding_box, int layer, int team_index)
	{  
		Enemy_Spawn_Info type = Spawn_Info[type_index];

		Pawn actor			= <Pawn>Scene.Spawn(type.Pawn_Type, null);				
		actor.Team_Index	= team_index;
		actor.Layer			= layer;

		Vec4 collision_box  = actor.Collision_Box;
		actor.Position		= Random_Spawn_Position_In_Bounds(bounding_box, collision_box);
				
		// Spawn the controller and give it control of our new enemy.
		Controller controller = <Controller>Scene.Spawn(type.Controller_Type, null);
		controller.Possessed = actor;

		return actor;
	}	
    
	// Spawns an enemy at the given position with human-control.
	public static Pawn Spawn_Human_Enemy(NetUser client, Enemy_Type type_index, Vec4 bounding_box, int layer, int team_index, Pawn actor)
	{  
		Enemy_Spawn_Info type = Spawn_Info[type_index];

	    if (actor == null)
        {   
            actor = <Pawn>Scene.Spawn(type.Pawn_Type, client);				
		    actor.Team_Index	= team_index;
		    actor.Layer			= layer;

		    Vec4 collision_box  = actor.Collision_Box;
		    actor.Position		= Random_Spawn_Position_In_Bounds(bounding_box, collision_box);
        }
        else
        {
            // Cause existing controller to gtfo.
            actor.Possessor.Possessed = null;

            // Swap ownership (for the purposes of actor interpolation etc).
            actor.Change_Owner(client);
        }

		// Spawn the controller and give it control of our new enemy.
		Controller controller = <Controller>Scene.Spawn(type.Player_Controller_Type, client);
		controller.Possessed = actor;

		client.Controller = controller;
		client.Last_Spawn_Time = Time.Get_Ticks();
        client.Waiting_For_Zombie_Control = false;  

		return actor;
	}	
}