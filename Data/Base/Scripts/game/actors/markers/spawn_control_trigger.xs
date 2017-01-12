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
	Name("Spawn Control Trigger"), 
	Description("Manipulates spawning.") 
]
public class Spawn_Control_Trigger : Actor
{	
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
		Name("Spawning Enabled"), 
		Description("Sets spawning is enabled.") 
	]
	public serialized(1) bool Spawning_Enabled = false;

	[ 
		Name("Difficulty Scalar"), 
		Description("Sets difficulty scalar.") 
	]
	public serialized(1) float Difficulty_Scalar = 1.0f;
    
	[ 
		Name("Spawn Throttle Scalar"), 
		Description("Throttle enemy spawns by the given scalar.") 
	]
	public serialized(9000) float Spawn_Throttle_Scalar = 1.0f;

    [ 
		Name("Fodder Spawn Throttle Scalar"), 
		Description("Throttle fodder enemy spawns by the given scalar.") 
	]
	public serialized(9001) float Fodder_Spawn_Throttle_Scalar = 1.0f;

	// Enemy type modifiers.
	public serialized(160) bool Spawn_Fodder		= true;
	public serialized(160) bool Spawn_Chaser		= true;
	public serialized(160) bool Spawn_Exploder		= true;
	public serialized(160) bool Spawn_Bouncer		= false;
	public serialized(200) bool Spawn_Large_Blobby	= false;
	public serialized(200) bool Spawn_Small_Blobby	= false;
	public serialized(700) bool Spawn_Floater		= false;

	private int m_trigger_count = 0;
	private float m_last_trigger_time = 0;

	Spawn_Control_Trigger() 
	{
		Bounding_Box	= Vec4(0, 0, 64, 64);
		collision.Area 	= Bounding_Box;
	}
	
	void Try_Trigger()
	{
		if (Maximum_Triggers == 0 ||  m_trigger_count < Maximum_Triggers)
		{
			float elapsed = Time.Get_Ticks() - m_last_trigger_time;

			if (m_last_trigger_time == 0 || elapsed > Trigger_Interval * 1000)
			{
				Enemy_Manager.Spawning_Enabled = Spawning_Enabled;
                Enemy_Manager.Spawn_Throttle_Scalar = Spawn_Throttle_Scalar;
                Enemy_Manager.Fodder_Spawn_Throttle_Scalar = Fodder_Spawn_Throttle_Scalar;
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Fodder,			Spawn_Fodder);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Chaser,			Spawn_Chaser);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Exploder,		Spawn_Exploder);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Bouncer,		Spawn_Bouncer);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Large_Blobby,	Spawn_Large_Blobby);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Small_Blobby,	Spawn_Small_Blobby);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Floater,		Spawn_Floater);

				Difficulty_Manager.Set_Difficulty(Difficulty_Scalar);

				m_last_trigger_time = Time.Get_Ticks();
				m_trigger_count++;
			}
		}
	}

	default state Spawning
	{
		event void On_Tick()
		{
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
