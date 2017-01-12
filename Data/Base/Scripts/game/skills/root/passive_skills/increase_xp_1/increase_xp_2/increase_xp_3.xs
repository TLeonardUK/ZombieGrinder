// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Increase_XP_3 : Skill_Archetype
{
	Skill_Increase_XP_3()
	{
		Name 				= "#skill_increase_xp_3_name";
		Description 		= "#skill_increase_xp_3_description";
		Icon_Frame 			= "skill_increase_xp_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Increase_XP_2";
		Unlock_Criteria_Threshold = 22;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.XP] = 1.5;
	}
}
