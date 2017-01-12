// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_XP_Boost_2 : Skill_Archetype
{
	Skill_XP_Boost_2()
	{
		Name 				= "#skill_xp_boost_2_name";
		Description 		= "#skill_xp_boost_2_description";
		Icon_Frame 			= "skill_xp_boost_2";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 300;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_XP_Boost_1";
		Is_Team_Based		= false;
		Duration			= 60.0f;
		Player_Effect		= "xp_boost_60s_player";
		Unlock_Criteria_Threshold = 18;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.XP] = 2.0;
	}
}
