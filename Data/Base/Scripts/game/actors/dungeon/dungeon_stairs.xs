// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using game.modes.dungeon;

[
	Placeable(true), 
	Name("Dungeon Stairs"), 
	Description("Lets move on to the next level!") 
]
public class Dungeon_Stairs : Actor, Interactable
{
	// All players have to be inside this range to interact with stairs.
	const float MINIMUM_INTERACT_DISTANCE = 72.0f;

	Dungeon_Stairs()
	{
		Bounding_Box = Vec4(0, 0, 48, 48);
	}
	
	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#descend_prompt");
	}
	
	public override bool Can_Interact(Pawn other)
	{
		// Can only interact if all players are in range.
		float dist = Scene.Distance_To_Furthest_Actor_Of_Type(this.Center, typeof(Human_Player));
		if (dist < MINIMUM_INTERACT_DISTANCE)
		{
			return true;
		}
		return false;
	}

	public override void Interact(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_GameOver();
		}
	}

	public rpc void RPC_GameOver()
	{		
		if (Network.Is_Server())
		{
			Dungeon_Crawler_Mode mode = <Dungeon_Crawler_Mode>Scene.Active_Game_Mode;
			if (mode != null)
			{
				mode.Game_Over(true, "next_dungeon_level");
			}
		}
	}
	
	default state Idle
	{
		event void On_Enter()
		{
			Hibernate();
		}
	}
	
	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "decal_dungeon_stairs";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
	}
}