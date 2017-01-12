// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Ghost_Life_Time_2 : Skill_Archetype
{
	Skill_Ghost_Life_Time_2()
	{
		Name 				= "#skill_ghost_life_time_2_name";
		Description 		= "#skill_ghost_life_time_2_description";
		Icon_Frame 			= "skill_ghost_life_time_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Ghost_Life_Time_1";
		Unlock_Criteria_Threshold = 3;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Ghost_Life_Time] = 2.0;
	}
}
