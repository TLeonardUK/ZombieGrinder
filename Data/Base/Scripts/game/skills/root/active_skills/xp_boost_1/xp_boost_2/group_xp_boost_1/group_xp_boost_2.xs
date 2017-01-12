// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Group_XP_Boost_2 : Skill_Archetype
{
	Skill_Group_XP_Boost_2()
	{
		Name 				= "#skill_group_xp_boost_2_name";
		Description 		= "#skill_group_xp_boost_2_description";
		Icon_Frame 			= "skill_group_xp_boost_2";
		Cost 				= 4;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Group_XP_Boost_1";
		Is_Team_Based		= true;
		Duration			= 90.0f;
		Player_Effect		= "xp_boost_90s_player";
		Unlock_Criteria_Threshold = 36;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.XP] = 2.5;
	}
}
