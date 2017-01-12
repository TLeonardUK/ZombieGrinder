// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Group_Heal_1 : Skill_Archetype
{
	Skill_Group_Heal_1()
	{
		Name 				= "#skill_group_heal_1_name";
		Description 		= "#skill_group_heal_1_description";
		Icon_Frame 			= "skill_group_heal_1";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Heal_2";
		Is_Team_Based		= true;
		Duration			= 8.0f;
		Player_Effect		= "heal_8s_player";
        Unlock_Criteria_Threshold = 33;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Health_Regen] = 1.0f / 8.0f;
	}
}
