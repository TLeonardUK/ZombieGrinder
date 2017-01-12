// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_XP_Boost_3 : Skill_Archetype
{
	Skill_XP_Boost_3()
	{
		Name 				= "#skill_xp_boost_3_name";
		Description 		= "#skill_xp_boost_3_description";
		Icon_Frame 			= "skill_xp_boost_3";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_XP_Boost_2";
		Is_Team_Based		= false;
		Duration			= 90.0f;
		Player_Effect		= "xp_boost_90s_player";
		Unlock_Criteria_Threshold = 26;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.XP] = 2.5;
	}
}
