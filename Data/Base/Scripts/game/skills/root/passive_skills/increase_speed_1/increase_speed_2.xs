// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Increase_Speed_2 : Skill_Archetype
{
	Skill_Increase_Speed_2()
	{
		Name 				= "#skill_increase_speed_2_name";
		Description 		= "#skill_increase_speed_2_description";
		Icon_Frame 			= "skill_increase_speed_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Increase_Speed_1";
		Unlock_Criteria_Threshold = 4;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Speed] = 1.2;
	}
}
