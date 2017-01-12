// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Incendiary_Bomb_2 : Skill_Archetype
{
	Skill_Incendiary_Bomb_2()
	{
		Name 				= "#skill_incendiary_bomb_2_name";
		Description 		= "#skill_incendiary_bomb_2_description";
		Icon_Frame 			= "skill_incendiary_bomb_2";
		Cost 				= 4;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Incendiary_Bomb_1";
		Is_Team_Based		= false;
		Duration			= 0.0f;
        Unlock_Criteria_Threshold = 34;
	}

	public override void Apply(NetUser user)
	{
		if (user.Controller != null)
		{
			Pawn pawn = user.Controller.Possessed;
			if (pawn != null)
			{
				FX.Spawn_Untracked("skill_incendiary_bomb_2", pawn.Center, 0, pawn.Layer, pawn);
			}
		}
	}
}
