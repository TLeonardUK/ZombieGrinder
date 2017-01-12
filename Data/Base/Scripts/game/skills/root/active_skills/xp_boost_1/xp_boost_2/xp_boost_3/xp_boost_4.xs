// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_XP_Boost_4 : Skill_Archetype
{
	Skill_XP_Boost_4()
	{
		Name 				= "#skill_xp_boost_4_name";
		Description 		= "#skill_xp_boost_4_description";
		Icon_Frame 			= "skill_xp_boost_4";
		Cost 				= 5;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_XP_Boost_3";
		Is_Team_Based		= false;
		Duration			= 120.0f;
		Player_Effect		= "xp_boost_120s_player";
		Unlock_Criteria_Threshold = 33;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.XP] = 3.0;
	}
}
