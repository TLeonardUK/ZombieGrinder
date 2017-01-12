// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.time;

[
	Placeable(true), 
	Name("Actor Spawner"), 
	Description("When triggered it spawns different types of actors inside it.") 
]
public class Actor_Spawner : Actor
{
	[ 
		Name("Team Index"), 
		Description("If spawning a pawn it will be assigned to this team.") 
	]
	public serialized(1) int Team_Index = 0xFF;

	/*
	[ 
		Name("As Oval"), 
		Description("If set spawns will be spawned in an oval shape.") 
	] 
	private bool m_as_oval = false;
	property serialized(1) bool As_Oval
	{
		bool Get()
		{
			return m_as_oval;
		}
		void Set(bool val)
		{
			m_as_oval = val;			
			sprite.Frame = m_as_oval ? "actor_shape_oval_0" : "actor_shape_rect_0";		
		}
	}
	*/

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
		Name("Minimum Count"), 
		Description("Minimum number of enemies to spawn.") 
	]
	public serialized(1) int Minimum_Count = 1;
	
	[ 
		Name("Maximum Count"), 
		Description("Maximum number of enemies to spawn.") 
	]
	public serialized(1) int Maximum_Count = 1;

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
		Name("Actor Type"), 
		Description("Class name of actor to spawn.") 
	]
	public serialized(1) string Actor_Type = "Zombie";
	
	[ 
		Name("Controller Type"), 
		Description("If a pawn is spawned a controller of this type will also be spawned to control them.") 
	]
	public serialized(1) string Controller_Type = "AI_Zombie_Controller";

	// Spawn tracking.
	private int m_spawn_counter = 0;
	private float m_spawn_interval = 0.0f;
	private float m_spawn_timer = 0.0f;

	Actor_Spawner()
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
			Hibernate();
		}
	} 

	state Spawning
	{
		event void On_Tick()
		{
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

	// Gets a random position to spawn the given bounding box in within our bounds.
	private Vec3 Get_Random_Spawn_Position(Vec4 bbox)
	{
		Vec4 own_bb = World_Bounding_Box;
		float range_x = Math.Max(0.0f, own_bb.Width - bbox.Width);
		float range_y = Math.Max(0.0f, own_bb.Height - bbox.Height);
		return Vec3
		(
			(own_bb.X + Math.Rand(0.0f, range_x)) - bbox.X,
			(own_bb.Y + Math.Rand(0.0f, range_y)) - bbox.X,
			0.0f
		);
	}

	private void Spawn()
	{
		Type type = Type.Find(Actor_Type);
		if (type == null)
		{
			Log.Write("Failed to find actor type '" + Actor_Type + "' for actor spawner.");
			return;
		}

		// Spawn plz.
		Log.Write("Spawning actor: " + Actor_Type);
		Actor actor = Scene.Spawn(type, null);
		if (actor != null)
		{
			actor.Position = Get_Random_Spawn_Position(actor.Bounding_Box);
			actor.Layer    = Layer;

			// Assign team index.
			Pawn pawn = <Pawn>actor;
			if (pawn != null)
			{
				pawn.Team_Index = Team_Index;
				Log.Write("Assigning spawned pawn to team '"+Team_Index+"'.");
			}

			// Assign controller.
			if (Controller_Type != "")
			{			
				Type controller_type = Type.Find(Controller_Type);
				
				if (controller_type == null)
				{
					Log.Write("Failed to find controller type '" + Actor_Type + "' for actor spawner.");
					return;
				}
				else
				{
					if (pawn != null)
					{
						Log.Write("Spawning controller: " + Controller_Type);
				
						Controller controller = <Controller>Scene.Spawn(controller_type, null);
						if (controller != null)
						{	
							controller.Possessed = pawn;
						}
						else 
						{
							Log.Write("Failed to spawn controller '" + Controller_Type + "'.");
						}
					}
				}
			}
		}
		else
		{
			Log.Write("Failed to spawn actor '" + Actor_Type + "'.");
		}

		// Update spawn tracking.
		m_spawn_counter--;
		m_spawn_interval = Math.Rand(Minimum_Interval, Maximum_Interval);
		m_spawn_timer = Time.Get_Ticks();

		// No more spawns left? May as well go hibernate till we are needed again.
		if (m_spawn_counter <= 0)
		{
			Active = false;
			change_state(Awaiting_Spawning);
		}
	}
	
	private void Start_Spawning()
	{		
		m_spawn_counter = Math.Rand(Minimum_Count, Maximum_Count);
		Active = true;

		Wake_Up();
		change_state(Spawning);
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
