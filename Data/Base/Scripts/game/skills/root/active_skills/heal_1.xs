// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Heal_1 : Skill_Archetype
{
	Skill_Heal_1()
	{
		Name 				= "#skill_heal_1_name";
		Description 		= "#skill_heal_1_description";
		Icon_Frame 			= "skill_heal_1";
		Cost 				= 1;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 200;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Active_Skills";
		Is_Team_Based		= false;
		Duration			= 8.0f;
		Player_Effect		= "heal_8s_player";
        Unlock_Criteria_Threshold = 0;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Health_Regen] = 1.0f / 8.0f;
	}
}
