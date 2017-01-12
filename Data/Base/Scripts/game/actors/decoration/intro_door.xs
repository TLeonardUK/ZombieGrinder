// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using game.actors.interactable.interactable;

[
	Placeable(true), 
	Name("Intro Door"), 
	Description("A door that only opens for players in intros/outros.") 
]
public class Intro_Door : Actor
{
	[ 
		EditType("Animation"),
		Description("Animation to play when opening. Closing is the same in reverse.")
	]
	public serialized(1) string Open_Animation = "actor_door_laser_redh_open";
	
	[ 
		EditType("Audio"),
		Description("Sound to play when opening or closing door.")
	]
	public serialized(1) string Open_Sound = "sfx_objects_doors_place";
	
	[ 
		Description("True if this door is currently open or not.")
	]
	public serialized(1) bool Is_Open = false;
	
	[ 
		Description("Radius to use for proximity opening.")
	]
	public serialized(101) float Proximity_Radius = 30.0f;
    
	private int m_last_open_state = -1;

	Intro_Door()
	{
		Bounding_Box	= Vec4(0, 0, 64, 37);
		Store_Initial_State();
	}

	private void Store_Initial_State()
	{		
		Update_State();
	}

	private void Update_State()
	{
		sprite.Animation = Open_Animation;
		sprite.Animation_Speed =  Is_Open ? 1.0f : -1.0f;
		sprite.Frame = "";
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
			if (Network.Is_Server())
			{
                Actor[] actors = Scene.Find_Actors_In_Radius(typeof(Player_Human_Controller), Proximity_Radius * 2.0f, this.Collision_Center);

                Is_Open = false;

                foreach (Player_Human_Controller player in actors)
                {
                    if (player.Is_In_Outro())
                    {
                        Is_Open = true;
                        break;
                    }   
                }                
			}

			if (m_last_open_state != <int>Is_Open)
			{
				Audio.Play3D(Open_Sound, this.Center);
				Update_State();
				m_last_open_state = <int>Is_Open;
			}
		}
	}

	replicate(ReplicationPriority.High)
	{	
		Is_Open
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "actor_door_laser_redh_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}