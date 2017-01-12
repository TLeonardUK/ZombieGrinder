// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Enemy Manual Spawmer"), 
	Description("Spawns a set amount of enemies of the given types when triggered.") 
]
public class Enemy_Manual_Spawner : Actor
{
	[ 
		Name("Team Index"), 
		Description("If spawning a pawn it will be assigned to this team.") 
	]
	public serialized(1) int Team_Index = 0xFF;  

	[ 
		Name("Active"), 
		Description("If set this zone will begin spawning immediately, otherwise it will wait for a triggering event.") 
	]
	public serialized(1) bool Active = false;
	
	[ 
		Name("Server Only"), 
		Description("If set then this actor will only run on the server. Do this if you are spawning replicated actors, otherwise don't.") 
	]
	public serialized(1) bool Server_Only = true;

	[ 
		Name("Must Be Out Of Sight"), 
		Description("If checked spawn zone must be out of sight before anything will spawn.") 
	]
	public serialized(1) bool Must_Be_Out_Of_Sight = true;
    
	[ 
		Name("Minimum Interval"), 
		Description("Minimum time in seconds to wait between spawns. 0 means all will spawn at once.") 
	]
	public serialized(1) float Minimum_Interval = 0.0f;
	
	[ 
		Name("Maximum Interval"), 
		Description("Maximum time in seconds to wait between spawns. 0 means all will spawn at once.") 
	]
	public serialized(1) float Maximum_Interval = 0.0f;
    
	[ 
		Name("Minimum Count"), 
		Description("Minimum number of enemies to spawn.") 
	]
	public serialized(8000) int Minimum_Count = 1;
	
	[ 
		Name("Maximum Count"), 
		Description("Maximum number of enemies to spawn.") 
	]
	public serialized(8000) int Maximum_Count = 1;

    [ 
		Name("Spawn Probability"), 
		Description("Probability of each enemy type spawning.") ,
        IndexDecoratorEnum("Enemy_Type")
	]
	public serialized(8000) Enemy_Spawn_Probability_Table Spawn_Probability = new Enemy_Spawn_Probability_Table();
        
	// Spawn tracking.
	private int m_spawn_counter = 0;
	private float m_spawn_interval = 0.0f;
	private float m_spawn_timer = 0.0f;

	Enemy_Manual_Spawner()
	{
		Bounding_Box = Vec4(0, 0, 64, 64);
	}
	
	default state Awaiting_Spawning
	{
		event void On_Enter()
		{
			if (Active == true)
			{
				change_state(Spawning);
			}
		}
		event void On_Tick()
		{
            if (Active)
            {
            	change_state(Spawning);			
            }
            else
            {
                Hibernate();
            }
		}
	} 

	state Spawning
	{
		event void On_Tick()
		{
            Log.Write("Spawning: "+m_spawn_timer+"/"+m_spawn_interval);

			// No server, no dice.
			if (!Network.Is_Server() && Server_Only == true)
			{
				Active = false;
				change_state(Awaiting_Spawning);
				return;
			}

			// Out of sight and left enough time since last spawn, lets go!
			if (Must_Be_Out_Of_Sight == false || !Network.Is_Visible_To_Users(World_Bounding_Box))
			{
				if (Time.Get_Ticks() - m_spawn_timer >= m_spawn_interval)
				{
					Spawn();
				}
			}
		}
	}

	private void Spawn()
	{
        // Spawn enemy!
        Enemy_Manager.Spawn_Enemy_Manual(Spawn_Probability.Get_Random_Enemy_Type(), World_Bounding_Box, Layer, Enemy_Manager.Default_Team_Index);

		// Update spawn tracking.
		m_spawn_counter--;
		m_spawn_interval = Math.Rand(Minimum_Interval, Maximum_Interval) * Difficulty_Manager.Enemy_Spawn_Interval_Multiplier;
		m_spawn_timer = Time.Get_Ticks();

		// No more spawns left? May as well go hibernate till we are needed again.
		if (m_spawn_counter <= 0)
		{
            Log.Write("No spawns remaining");
			Active = false;
			change_state(Awaiting_Spawning);
		}
	}
	
	private void Start_Spawning()
	{		
		m_spawn_counter = Math.Rand(Minimum_Count, Maximum_Count) * Difficulty_Manager.Enemy_Spawn_Count_Multiplier;
		Active = true;
    
		Wake_Up();
		//change_state(Spawning);
	}

	public override void On_Trigger()
	{
		Start_Spawning();
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
	}
}
