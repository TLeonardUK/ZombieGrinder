// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Bomb_3 : Skill_Archetype
{
	Skill_Bomb_3()
	{
		Name 				= "#skill_bomb_3_name";
		Description 		= "#skill_bomb_3_description";
		Icon_Frame 			= "skill_bomb_3";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Bomb_2";
		Is_Team_Based		= false;
		Duration			= 0.0f;
        Unlock_Criteria_Threshold = 25;
	}

	public override void Apply(NetUser user)
	{
		if (user.Controller != null)
		{
			Pawn pawn = user.Controller.Possessed;
			if (pawn != null)
			{
				FX.Spawn_Untracked("skill_bomb_3", pawn.Center, 0, pawn.Layer, pawn);
			}
		}
	}
}
