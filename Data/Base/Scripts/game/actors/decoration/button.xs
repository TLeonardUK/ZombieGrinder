// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.audio;
using game.actors.interactable.interactable;

[
    Placeable(true),
    Name("Button"),
    Description("When interacted with it switches the button frame and fires its link event.")
]
public class Button : Actor, Interactable
{
    const float INTERACTION_DURATION = 500.0f;

    private serialized(20) bool m_activated = false;
    private bool m_old_activated = false;

    [
        Name("Press Sound"),
        Description("Sound to play when pressed."),
        EditType("Audio")
    ]
    public serialized(30) string Audio_Name = "sfx_objects_switchs_use";

    [
        Name("Pressed Frame"),
        Description("Frame to use when button is pressed."),
        EditType("Image")
    ]
    public serialized(1) string Pressed_Frame = "actor_switch_default_1";

    [
        Name("Unpressed Frame"),
        Description("Frame to use when button is unpressed."),
        EditType("Image")
    ]
    public serialized(1) string Unpressed_Frame = "actor_switch_default_0";

    [
        Name("Require Touch"),
        Description("If set to true the user needs to touch this button to set it off.")
    ]
    public serialized(7000) bool bRequireTouch = false;

    [
        Name("Reset Interval"),
        Description("How long before button is reset. 0 for never reset.")
    ]
    public serialized(7000) float Reset_Interval = 0.0;

    private serialized(7000) float Reset_Timer = 0.0f;

    Button()
    {
        Bounding_Box = Vec4(0, 0, 16, 16);
    }
	
	default state Idle
    {
        event void On_Tick()
        {
            if (m_activated)
            {
                sprite.Frame = Pressed_Frame;

                if (Network.Is_Server())
                {
                    if (Reset_Timer > 0.0f)
                    {
                        Reset_Timer -= Time.Get_Delta_Seconds();
                        if (Reset_Timer <= 0.0f)
                        {
                            Log.Write("Resetting activated button.");
                            m_activated = false;
                        }
                    }
                }
            }
            else
            {
	    		sprite.Frame = Unpressed_Frame;
            }

            if (m_activated != m_old_activated)
            {    
        		Audio.Play3D(Audio_Name, Position);
            }

            m_old_activated = m_activated;
        }
	}
	
	public override bool Can_Interact(Pawn other)
	{
		return !m_activated && !bRequireTouch;
	}

	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#press_button_prompt");
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}

	public override float Get_Duration(Pawn other)
	{
		return INTERACTION_DURATION;
	}

	public override void Interact(Pawn other)
	{
		rpc(RPCTarget.Server, RPCTarget.None) RPC_Trigger();
	}
	
	private rpc void RPC_Trigger()
	{
        if (!m_activated)
        {
    		m_activated = true;
    		rpc(RPCTarget.All, RPCTarget.None) RPC_Trigger_Logic();
        }
	}
    
	private rpc void RPC_Trigger_Logic()
	{
		Log.Write("Button was pressed, firing event '" + Link + "'!");	
        Reset_Timer = Reset_Interval;		
		Scene.Trigger_Actors(Link);
    }

	// Force replication as network object.
	replicate(ReplicationPriority.Low)
	{
		m_activated
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}	    
	}
	
	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_switch_default_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}