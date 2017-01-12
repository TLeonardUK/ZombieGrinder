 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.time;
using system.components.collision;
using system.components.path_planner;
using system.events;
using game.actors.enemies.floater_zombie;
using game.difficulty.difficulty_manager;

// This class does exactly fuck all. All the logic is in the actual pawn. Not really a proper seperate
// of visual/control, but eh, the complex boss logic is easier to do this way!

[
	Name("AI Boss Whipper Controller"), 
	Description("Controls enemies that follow nearest enemy and shoot blood clots at them.") 
]
public class AI_Boss_Whipper_Controller : Controller
{
	AI_Boss_Whipper_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
	}

	// -------------------------------------------------------------------------
	// This state just waits for a pawn to control.
	// -------------------------------------------------------------------------
	default state Waiting_For_Possession
	{
	}
	 
	// -------------------------------------------------------------------------
	// Idle state - stand still waiting for target.
	// -------------------------------------------------------------------------
	state Idle
	{
		event void On_Enter()
		{
			Track_Pawn();
		}		
		event void On_Tick()
		{
			Track_Pawn();
		}
	}   
	
	void Track_Pawn()
	{
		Position = this.Possessed.Position;
	}
	 
	// -------------------------------------------------------------------------
	// General functions.
	// -------------------------------------------------------------------------
	override void On_Possession_Changed(Pawn other)
	{	
		if (other != null)
		{
			change_state(Idle);
		}
		else
		{
			Log.Write("Lost possession of pawn, despawning controller.");
			Scene.Despawn(this);
		}
	}	

	override void On_Possessed_Despawned()
	{
		Log.Write("Pawn despawned, despawning controller.");
		Scene.Despawn(this);
	}
	
	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{
	}
}
