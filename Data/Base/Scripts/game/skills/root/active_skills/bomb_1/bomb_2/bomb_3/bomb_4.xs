// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;
using system.fx;

public class Skill_Bomb_4 : Skill_Archetype
{
	Skill_Bomb_4()
	{
		Name 				= "#skill_bomb_4_name";
		Description 		= "#skill_bomb_4_description";
		Icon_Frame 			= "skill_bomb_4";
		Cost 				= 4;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Bomb_3";
		Is_Team_Based		= false;
		Duration			= 0.0f;
        Unlock_Criteria_Threshold = 37;
	}

	public override void Apply(NetUser user)
	{
		if (user.Controller != null)
		{
			Pawn pawn = user.Controller.Possessed;
			if (pawn != null)
			{
				FX.Spawn_Untracked("skill_bomb_4", pawn.Center, 0, pawn.Layer, pawn);
			}
		}		
	}
}
