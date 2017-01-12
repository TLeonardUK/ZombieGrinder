// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Ghost_Revive_Time_3 : Skill_Archetype
{
	Skill_Ghost_Revive_Time_3()
	{
		Name 				= "#skill_ghost_revive_time_3_name";
		Description 		= "#skill_ghost_revive_time_3_description";
		Icon_Frame 			= "skill_ghost_revive_time_3";
		Cost 				= 3;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Ghost_Revive_Time_2";
		Unlock_Criteria_Threshold = 17;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Ghost_Revive_Time] = 0.3;
	}
}
