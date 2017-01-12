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
	Name("Interactive Trigger"), 
	Description("Similar to a normal trigger but the user must interact with it.") 
]
public class Interactive_Trigger : Actor, Interactable
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
	
	private serialized(1) int m_trigger_count = 0;
	private serialized(1) float m_last_trigger_time = 0;

	Interactive_Trigger() 
	{
		Bounding_Box	= Vec4(0, 0, 64, 64);
	}
	
	public override float Get_Duration(Pawn other)
	{
		return 0.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#interact_prompt");
	}

	public override void Interact(Pawn other)
	{
		rpc(RPCTarget.Server, RPCTarget.None) RPC_Request_Trigger(other.Net_ID);
	}
	
	public override bool Can_Interact(Pawn other)
	{
		if (!this.Enabled)
		{
			return false;
		}

		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			if (Maximum_Triggers == 0 || m_trigger_count < Maximum_Triggers)
			{
				return true;
			}
		}

		return false;
	}

	private rpc void RPC_Request_Trigger(int id)
	{
		Pawn other = <Pawn>Scene.Get_Net_ID_Actor(id);

		if (Network.Is_Server() && other != null)
		{
			if ((<Interactable>this).Can_Interact(other)) // TODO: Fix compiler, this cast should be redundent.
			{
				if (Maximum_Triggers == 0 ||  m_trigger_count < Maximum_Triggers)
				{
					rpc(RPCTarget.All, RPCTarget.None) RPC_Start_Trigger();
				}
			}
		}
	}

	private rpc void RPC_Start_Trigger()
	{
		float elapsed = Time.Get_Ticks() - m_last_trigger_time;

		if (m_last_trigger_time == 0 || elapsed > Trigger_Interval * 1000)
		{
			Log.Write("Trigger Fired: " + Link);

			// Fire! Fire! Fire!
			Scene.Trigger_Actors(Link);

			m_last_trigger_time = Time.Get_Ticks();
			m_trigger_count++;
		}
	}
	
	// Invoked when triggers are linked to this actor and invoked.
	public override void On_Trigger()
	{
		this.Enabled = !this.Enabled;
	}

	// Force replication as network object.
	replicate(ReplicationPriority.Low)
	{
		m_trigger_count
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}		
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
	}
}
