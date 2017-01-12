// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Status_Duration_2 : Skill_Archetype
{
	Skill_Status_Duration_2()
	{
		Name 				= "#skill_status_duration_2_name";
		Description 		= "#skill_status_duration_2_description";
		Icon_Frame 			= "skill_status_duration_2";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Status_Duration_1";
		Unlock_Criteria_Threshold = 6;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Enemy_Status_Duration] = 1.5;
	}
}
