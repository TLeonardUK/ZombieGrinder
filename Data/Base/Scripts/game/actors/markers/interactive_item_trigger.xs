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
	Name("Interactive Item Trigger"), 
	Description("Same as interactive trigger except it requires a given interactive item, and triggers different events depending on if the user has it or not.") 
]
public class Interactive_Item_Trigger : Actor, Interactable
{	
	[ 
		Name("No Item Link"), 
		Description("Event invoked if user dosen't have the item required.") 
	]
	public serialized(1) string No_Item_Link = "";	
	
	[ 
		Name("Item Type"), 
		Description("Type of item the user must have to trigger.") 
	]
	public serialized(1) string Item_Type = "Key_Pickup";	

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
	private serialized(1) bool m_no_item_link_triggered = false;

	Interactive_Item_Trigger() 
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
					float elapsed = Time.Get_Ticks() - m_last_trigger_time;

					if (m_last_trigger_time == 0 || elapsed > Trigger_Interval * 1000)
					{
						// Does user have the item required?
						Inventory_Pickup pickup = Inventory_Pickup.Get_Inventory_Item_For_Net_ID(other.Net_ID);
				 
						if (pickup == null || pickup.GetType().Name != Item_Type)
						{
							rpc(RPCTarget.All, RPCTarget.None) RPC_Start_No_Pickup();
						}
						else
						{
							rpc(RPCTarget.All, RPCTarget.None) RPC_Start_Have_Pickup(id);
						}
					}
				}
			}
		}
	}
	
	private rpc void RPC_Start_No_Pickup()
	{
		if (!m_no_item_link_triggered)
		{
			Log.Write("No Item Trigger Fired: " + No_Item_Link);
			Scene.Trigger_Actors(No_Item_Link);

			m_no_item_link_triggered = true;
		}		
	}

	private rpc void RPC_Start_Have_Pickup(int id)
	{
		Pawn other = <Pawn>Scene.Get_Net_ID_Actor(id);

		// Does user have the item required?
		Inventory_Pickup pickup = Inventory_Pickup.Get_Inventory_Item_For_Net_ID(other.Net_ID);
		
		Log.Write("Trigger Fired: " + Link);
		Scene.Trigger_Actors(Link);

		// Despawn the pickup.
		if (Network.Is_Server() && pickup != null)
		{
			pickup.Reset();
			pickup.Despawn();
		}

		m_last_trigger_time = Time.Get_Ticks();
		m_trigger_count++;
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
