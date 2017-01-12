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
	Name("Trigger"), 
	Description("When a given actor walks into this trigger it throws an event at the given actors.") 
]
public class Trigger : Actor
{	
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

			collision.Shape = val ? CollisionShape.Oval : CollisionShape.Rectangle;
			sprite.Frame = m_as_oval ? "actor_shape_oval_0" : "actor_shape_rect_0";		
		}
	}
	*/

	[ 
		Name("Maximum Triggers"), 
		Description("Maximum number of times this can be triggered.") 
	]
	public serialized(1) int Maximum_Triggers = 0;
	
	[ 
		Name("Trigger Interval"), 
		Description("Minimum time between each trigger in seconds.") 
	]
	public serialized(1) float Trigger_Interval = 0.0;
	
	[ 
		Name("Team Index"), 
		Description("Team index of pawns that can trigger this. -1 = all teams.") 
	]
	public serialized(1) int Team_Index = -1;
	
	[ 
		Name("Require All Players"), 
		Description("Requires all players to be within the trigger bounds to trigger.") 
	]
	public serialized(800) bool Require_All_Players = false;
	
	private int m_trigger_count = 0;
	private float m_last_trigger_time = 0;

	Trigger() 
	{
		Bounding_Box	= Vec4(0, 0, 64, 64);
		collision.Area 	= Bounding_Box;
	}
	
    virtual void Trigger_Logic()
    {    
		// Fire! Fire! Fire!
		Scene.Trigger_Actors(Link);
    }

	void Try_Trigger()
	{
		if (Maximum_Triggers == 0 ||  m_trigger_count < Maximum_Triggers)
		{
			float elapsed = Time.Get_Ticks() - m_last_trigger_time;

			if (m_last_trigger_time == 0 || elapsed > Trigger_Interval * 1000)
			{
				Log.Write("Trigger Fired: " + Link);
                Trigger_Logic();
				m_last_trigger_time = Time.Get_Ticks();
				m_trigger_count++;
			}
		}
	}

    virtual void Base_Update()
    {

    }

	default state Spawning
	{
		event void On_Tick()
		{
            Base_Update();
		//	Hibernate();
		}
		event void On_Editor_Property_Change()
		{
			collision.Area 	= Bounding_Box;
		}
		event void On_Touch(CollisionComponent component, Actor other)
		{
			if (other != null)
			{
				if (Team_Index == -1 || (other.Owner != null && other.Owner.Team_Index == Team_Index)) // Owned by a player.
				{
					if (Require_All_Players)
					{
						Actor[] players = Scene.Find_Actors(typeof(Human_Player));
						Vec4 this_area = this.World_Bounding_Box;
						bool bAllInside = true;

						foreach (Actor player in players)
						{
							Vec4 area = player.World_Bounding_Box;
							if (!this_area.Intersects(area))
							{
								return;
							}
						}
					}

					Try_Trigger();
				}
			}
		}
	} 
	
	// Invoked when triggers are linked to this actor and invoked.
	public override void On_Trigger()
	{
		this.Enabled = !this.Enabled;
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 255.0, 0.0, 128.0);
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
