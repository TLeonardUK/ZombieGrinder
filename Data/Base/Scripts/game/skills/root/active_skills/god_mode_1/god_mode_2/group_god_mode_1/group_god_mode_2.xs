// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Group_God_Mode_2 : Skill_Archetype
{
	Skill_Group_God_Mode_2()
	{
		Name 				= "#skill_group_god_mode_2_name";
		Description 		= "#skill_group_god_mode_2_description";
		Icon_Frame 			= "skill_group_god_mode_2";
		Cost 				= 4;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Group_God_Mode_1";
		Is_Team_Based		= true;
		Duration			= 10.0f;
		Player_Effect		= "god_mode_10s_player";
        Unlock_Criteria_Threshold = 42;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Damage_Taken] = 0.0f;
	}
}
