// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_God_Mode_1 : Skill_Archetype
{
	Skill_God_Mode_1()
	{
		Name 				= "#skill_god_mode_1_name";
		Description 		= "#skill_god_mode_1_description";
		Icon_Frame 			= "skill_god_mode_1";
		Cost 				= 1;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 200;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Active_Skills";
		Is_Team_Based		= false;
		Duration			= 5.0f;
		Player_Effect		= "god_mode_5s_player";
        Unlock_Criteria_Threshold = 0;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Damage_Taken] = 0.0f;
	}
}
