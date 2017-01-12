 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.scene;

[
	Name("Controller"), 
	Description("Base class for all controllers than control the behaviour of pawns.") 
]
public class Controller : Actor
{
	private Pawn m_possessed;
	private serialized(1) int m_possessed_id = -1;

	// Called on load when possession is updated.
	event void On_Possession_Synced()
	{
		if (m_possessed_id != -1)
		{
	    	Log.Write("Controller " + this.Net_ID + " is having possession synced.");		
			Change_Possessed(m_possessed_id);
		}
	}

	// RPC to tell other user who this controller is controlling.
	private rpc void RPC_Set_Possessed(int actor_id)
	{		
		Change_Possessed(actor_id);
	}	

	private void Change_Possessed(int actor_id)
	{
        Pawn new_pawn = <Pawn>Scene.Get_Net_ID_Actor(actor_id);;

        // No point changing all the resync logic if we are already in possession.
        if (actor_id == m_possessed_id)
        {
            if (m_possessed == new_pawn)
            {
                if (m_possessed.Possessor == this)
                {
                    return;
                }
            }
        }

		Log.Write("Controller " + this.Net_ID + " is now possessing actor " + actor_id);		
		
		m_possessed_id = actor_id;
		
		m_possessed = <Pawn>Scene.Get_Net_ID_Actor(actor_id);
		if (m_possessed != null)
		{
			m_possessed.Possessor = this;
		}

		On_Possession_Changed(m_possessed);
	}
	
	// Gets/Sets the current pawn this controller is possessing.
	property Pawn Possessed
	{
		public Pawn Get()
		{
			return m_possessed;
		}
		
		public void Set(Pawn value)
		{		
			rpc(RPCTarget.All, RPCTarget.None) RPC_Set_Possessed(value == null ? -1 : value.Net_ID);
		}
	}		
	
	// Invoked when the pawn we are possessing is changed.
	virtual void On_Possession_Changed(Pawn other)
	{	
		// Do something in derived classes!
	}

	// Called by pawns to notify controller that they are going to despawn.
	virtual void On_Possessed_Despawned()
	{

	}
	
	// Current interaction states of controlled pawns.
	public virtual bool Is_Interacting()
	{
		return false;
	}
	
	public virtual float Get_Interact_Progress()
	{
		return 0.0f;
	}

	public virtual string Get_Interact_Prompt()
	{
		return "";
	}

    public virtual bool Is_Safe_To_Switch_Controller()
    {
        return true;
    }

    // Force controllers to be replicated.
    replicate(ReplicationPriority.High)
	{
	}
}