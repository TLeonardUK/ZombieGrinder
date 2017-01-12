// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Status_Duration_3 : Skill_Archetype
{
	Skill_Status_Duration_3()
	{
		Name 				= "#skill_status_duration_3_name";
		Description 		= "#skill_status_duration_3_description";
		Icon_Frame 			= "skill_status_duration_3";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Status_Duration_2";
		Unlock_Criteria_Threshold = 15;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Enemy_Status_Duration] = 1.75;
	}
}
