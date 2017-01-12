// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Group_Heal_2 : Skill_Archetype
{
	Skill_Group_Heal_2()
	{
		Name 				= "#skill_group_heal_2_name";
		Description 		= "#skill_group_heal_2_description";
		Icon_Frame 			= "skill_group_heal_2";
		Cost 				= 4;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Group_Heal_1";
		Is_Team_Based		= true;
		Duration			= 5.0f;
		Player_Effect		= "heal_5s_player";
        Unlock_Criteria_Threshold = 42;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Health_Regen] = 1.0f / 5.0f;
	}
}
