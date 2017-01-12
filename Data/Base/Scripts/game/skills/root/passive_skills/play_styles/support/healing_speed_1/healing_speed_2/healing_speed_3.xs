// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Healing_Speed_3 : Skill_Archetype
{
	Skill_Healing_Speed_3()
	{
		Name 				= "#skill_healing_speed_3_name";
		Description 		= "#skill_healing_speed_3_description";
		Icon_Frame 			= "skill_healing_speed_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Healing_Speed_2";
		Unlock_Criteria_Threshold = 20;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Healing_Speed] = 3.0;
	}
}
