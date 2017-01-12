// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Freeze_Bomb_1 : Skill_Archetype
{
	Skill_Freeze_Bomb_1()
	{
		Name 				= "#skill_freeze_bomb_1_name";
		Description 		= "#skill_freeze_bomb_1_description";
		Icon_Frame 			= "skill_freeze_bomb_1";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Bomb_2";
		Is_Team_Based		= false;
		Duration			= 0.0f;
        Unlock_Criteria_Threshold = 28;
	}

	public override void Apply(NetUser user)
	{
		if (user.Controller != null)
		{
			Pawn pawn = user.Controller.Possessed;
			if (pawn != null)
			{
				FX.Spawn_Untracked("skill_freeze_bomb_1", pawn.Center, 0, pawn.Layer, pawn);
			}
		}
	}
}
