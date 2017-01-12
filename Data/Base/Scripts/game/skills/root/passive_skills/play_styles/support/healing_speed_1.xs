// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Healing_Speed_1 : Skill_Archetype
{
	Skill_Healing_Speed_1()
	{
		Name 				= "#skill_healing_speed_1_name";
		Description 		= "#skill_healing_speed_1_description";
		Icon_Frame 			= "skill_healing_speed_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Support";
        Unlock_Criteria_Threshold = 0;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Healing_Speed] = 1.5;
	}
}
