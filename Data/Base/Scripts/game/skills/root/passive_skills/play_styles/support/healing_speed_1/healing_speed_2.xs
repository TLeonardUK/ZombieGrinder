// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Healing_Speed_2 : Skill_Archetype
{
	Skill_Healing_Speed_2()
	{
		Name 				= "#skill_healing_speed_2_name";
		Description 		= "#skill_healing_speed_2_description";
		Icon_Frame 			= "skill_healing_speed_2";
		Cost 				= 3;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Healing_Speed_1";
		Unlock_Criteria_Threshold = 2;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Healing_Speed] = 2.0;
	}
}
