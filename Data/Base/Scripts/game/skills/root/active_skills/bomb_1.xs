// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Bomb_1 : Skill_Archetype
{
	Skill_Bomb_1()
	{
		Name 				= "#skill_bomb_1_name";
		Description 		= "#skill_bomb_1_description";
		Icon_Frame 			= "skill_bomb_1";
		Cost 				= 1;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 200;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Active_Skills";
		Is_Team_Based		= false;
		Duration			= 0.0f;
        Unlock_Criteria_Threshold = 0;
	}

	public override void Apply(NetUser user)
	{
		if (user.Controller != null)
		{
			Pawn pawn = user.Controller.Possessed;
			if (pawn != null)
			{
				FX.Spawn_Untracked("skill_bomb_1", pawn.Center, 0, pawn.Layer, pawn);
			}
		}
	}
}
