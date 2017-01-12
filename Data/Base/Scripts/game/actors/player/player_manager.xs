// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.player.human_player;
using game.actors.player.player_spawn;
using game.actors.controllers.player_human_controller;
using game.actors.controllers.ai_human_controller;
using system.time;
using system.scene;

// Sorts a list of possible spawn points by order.
public class Player_Spawn_Comparer : Comparer<Player_Spawn>
{
    public override int Compare(Player_Spawn lvalue, Player_Spawn rvalue)
    {
        return lvalue.Spawn_Order > rvalue.Spawn_Order;
    }
}


[
	Name("Player_Manager"), 
	Description("Controls all aspects of spawning and despawning of players.") 
]
public static class Player_Manager
{
	// Used by Find_Spawn_Point to cycle through spawns.
	private static int s_spawn_offset = 0;
		
    // Timer to spread out spawns.
    private static float s_time_since_last_spawn = 0.0f;

    // Have we spawned any spawns yet.
    public static bool s_has_spawned_any = false;
    
	// Minimum time to wait between spawns globally.
	const float GLOBAL_SPAWN_DELAY = 0.5f;

	// Minimum time to wait between spawns for individual characters.
	const float SPAWN_DELAY = 6000;

	// Distance from enemies that a spawn point is considered "safe".
	const float SAFE_SPAWN_DISTANCE = 256;

	// Grace period where we always spawn the players even if they exit/re-enter etc.
	const float GRACE_PERIOD = 120 * 1000;

    // How far away the best spawn point has to be before we try and spawn with the pre-existing players.
    // Keep in mind this has to be withing the zoom-fit camera radius!
    const float SPAWN_WITH_OTHER_PLAYERS_DISTANCE = 160.0f;

    // Resets spawning to default.
    public static void Reset()
    {
        s_spawn_offset = 0;
        s_time_since_last_spawn = 0.0f;
        s_has_spawned_any = false;
    }
		
	// Finds a spawning point most suited to the given net-user.
	public static Player_Spawn Find_Spawn_Point(NetUser user, bool ignore_spawn_teams = false)
	{ 
		int target_team_index = user.Team_Index;
	
		// Gather all spawn points first.
		Actor[] actors = Scene.Find_Actors(typeof(Player_Spawn));
		
		// Discard those for opposing teams.
		Player_Spawn[] usable_actors = new Player_Spawn[0];
		foreach (Player_Spawn spawn in actors)
		{
			if (spawn.Active == true &&
				(spawn.Team_Index == target_team_index || ignore_spawn_teams))
			{
				usable_actors.AddLast(spawn);
			}
		}  
		 
		if (usable_actors.Length() <= 0)
		{
			Log.Write("No player spawns could be found to match client criteria!");	
			return null;			
		}

        // Sort spawns by spawn-order.
		usable_actors.Sort(new Player_Spawn_Comparer());

		// Cycle through each spawn point each invocation so we don't spawn
		// all players on the same one!
        int idx = s_spawn_offset;
        s_spawn_offset++;

		return usable_actors[idx % usable_actors.Length()];
	}

	// Similar to Find_Spawn_Point but also takes into account looking for
	// spawn points that are furthest away from players on other teams.
	public static Player_Spawn Find_Safe_Spawn_Point(NetUser user, bool ignore_spawn_teams = false)
	{ 
		int target_team_index = user.Team_Index;
	
		// Gather all spawn points first.
		Actor[] actors = Scene.Find_Actors(typeof(Player_Spawn));
		
		// Discard those for opposing teams.
		Actor[] usable_actors = new Actor[0];
		foreach (Player_Spawn spawn in actors)
		{
			if (spawn.Active == true &&
				(spawn.Team_Index == target_team_index || ignore_spawn_teams))
			{
				usable_actors.AddLast(spawn);
			}
		}  
		 
		if (usable_actors.Length() <= 0)
		{
			Log.Write("No player spawns could be found to match client criteria!");	
			return null;			
		}

		// Find all actors that are a safe distance away from other players.
		Player_Spawn[] prefered_spawns = new Player_Spawn[0];		
		float furthest_distance = 0.0f;
		Player_Spawn furthest_distance_spawn = null;

		foreach (Player_Spawn spawn in usable_actors)
		{
			float distance = Scene.Distance_To_Enemy_Actors(spawn.Position, user.Team_Index);
			if (distance > SAFE_SPAWN_DISTANCE)
			{
				prefered_spawns.AddLast(spawn);
			}
			if (furthest_distance_spawn == null || distance > furthest_distance)
			{
				furthest_distance_spawn = spawn;
				furthest_distance = distance;
			}
		}

		// If we have safe spawns, grab one of them.
		if (prefered_spawns.Length() > 0)
		{        
            int idx = s_spawn_offset;
            s_spawn_offset++;
			return <Player_Spawn>prefered_spawns[idx % prefered_spawns.Length()];
		}
		// Otherwise just use the one furthest from everyone else.
		else
		{
			return furthest_distance_spawn;
		}
	}

	// Spawns human players for all users on a given team who do
	// not currently possess a player.
	public static void Spawn_Humans(int team_index = -1, bool use_safest_spawn_point = false, bool ignore_spawn_teams = false, bool only_new_spawns = false, bool check_grace_period = false, bool ignore_global_spawn_delay = true)
	{  
		if (check_grace_period == true && Time.Get_Map_Time() > GRACE_PERIOD)
		{
			return;
		}

        if (Cheat_Manager.Spawn_Single_Player_Only)
        {
    		Actor[] players = Scene.Find_Actors(typeof(Human_Player));
            if (players.Length() > 0)
            {
                return;
            }
        }

		foreach (NetUser client in Network.Get_Users())
		{ 
            if (client.Last_Spawn_Time == 0)
			{
				client.First_Spawn_Timer += Time.Get_Frame_Time();  
            }
		}

        // Only spawn after a global delay.
        bool bSpawnSingleOnly = Cheat_Manager.Spawn_Single_Player_Only;      
        if (!ignore_global_spawn_delay)
        {
            s_time_since_last_spawn += Time.Get_Delta_Seconds(); 
            
            if (s_time_since_last_spawn < GLOBAL_SPAWN_DELAY)
            {
                return;
            }
            else 
            {
                s_time_since_last_spawn = 0.0f;
            }

            bSpawnSingleOnly = true;
        }
        
		foreach (NetUser client in Network.Get_Users())
		{ 
			// We delay spawns for a few seconds to prevent constant respawn-die cycles and to transition
			// from loading a bit more elegantly.
			float elapsed_spawn_time = 0.0;

			// If first spawn base spawn time on in-game time.
			if (client.Last_Spawn_Time == 0)
			{
				elapsed_spawn_time = client.First_Spawn_Timer;
                if (!ignore_global_spawn_delay || s_has_spawned_any) 
                {
                    elapsed_spawn_time = SPAWN_DELAY;
                }
            }			
			// Otherwise base on last alive time.
			else
			{
				elapsed_spawn_time = Time.Get_Ticks() - client.Last_Spawn_Time;
			} 
		 
			// Spawn if we can!
			if (client.In_Game 					== true &&
				(only_new_spawns == false || client.Last_Spawn_Time == 0.0f) &&
				client.Controller		 		== null &&
				(client.Team_Index 				== team_index || team_index == -1) &&
				elapsed_spawn_time				>= SPAWN_DELAY)
			{
				Player_Spawn point = null;
				if (use_safest_spawn_point == true)
				{
					point = Find_Safe_Spawn_Point(client, ignore_spawn_teams);
				}

				if (point == null)
				{
					point = Find_Spawn_Point(client, ignore_spawn_teams);
				}

                // If point is further away from players than reasonable, try and spawn at the players position instead.
                Human_Player other_player = null;
                if (!use_safest_spawn_point && point != null)
                {
	                Human_Player potential_player = <Human_Player>Scene.Find_Closest_Actor(typeof(Human_Player), Vec3(point.World_Bounding_Box.Center(), 0.0f));
                    if (potential_player)
                    {
                        float distance = (potential_player.World_Bounding_Box.Center() - point.World_Bounding_Box.Center()).Length();
                        if (distance > SPAWN_WITH_OTHER_PLAYERS_DISTANCE)
                        {
                            other_player = potential_player;
                        }
                    }
                }

				if (point == null)
				{
					continue;
				}

				Log.Write("Spawning client '" + client.Username + "' as human.");
							
                int players_count = Scene.Count_Actors_By_Type(typeof(Human_Player));

				// Spawn the human pawn.
				Human_Player actor = <Human_Player>Scene.Spawn(typeof(Human_Player), client);				
				actor.Team_Index = client.Team_Index;

                if (other_player != null)
                {
					actor.Position = other_player.Position;
					actor.Layer    = other_player.Layer;
                }
				else if (point == null)
				{
					actor.Position = Vec3(0, 0, 0);
					actor.Layer    = 2;
				}
				else
				{
					actor.Position = point.Position;
					actor.Layer    = point.Layer;
				}				
				
				Player_Spawn_Cutscene_Point cutscene_point = null;
				if (point != null && point.Intro_Point_Link != "")
				{
					Actor[] points = Scene.Find_Actors_By_Tag(point.Intro_Point_Link);
					if (points.Length() > 0)
					{
						cutscene_point = <Player_Spawn_Cutscene_Point>points[0];
						
						actor.Position = cutscene_point.Position;
						actor.Layer    = cutscene_point.Layer;
					}
				}

				// Spawn the controller and give it control of our new human.
				Player_Human_Controller controller = null;
				if (/*client.Is_Local &&*/ (Options.Get_Bool("enable_ai") || (Options.Get_Bool("enable_local_ai") && players_count > 0)))				
				{
					controller = <AI_Human_Controller>Scene.Spawn(typeof(AI_Human_Controller), client);
					controller.Possessed = actor;					
				}
				else
				{
					controller = <Player_Human_Controller>Scene.Spawn(typeof(Player_Human_Controller), client);
					controller.Possessed = actor;					
				}

				// Update client info.
				client.Controller = controller;
				client.Last_Spawn_Time = Time.Get_Ticks();
                client.Waiting_For_Zombie_Control = false;

                s_has_spawned_any = true;

				if (controller && cutscene_point != null)
				{
					controller.Queue_Intro(point);
				}

                if (bSpawnSingleOnly)
                {
                    return;
                }
			} 
		}
	}

	// Kills all humans!
	public static void Kill_Humans()
	{
		Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
		foreach (Human_Player other_player in potential_revivers)
		{
			other_player.Die(null, null);
		}
	}
	
	// Revives any currently incapacitated players.
	public static void Revive_Ghosts()
	{
		Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
		foreach (Human_Player other_player in potential_revivers)
		{
			if (other_player.Is_Incapacitated())
			{
				other_player.Revive(null);
			}
		}
	}
}