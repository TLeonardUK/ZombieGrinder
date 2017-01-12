// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Health_Regen_3 : Skill_Archetype
{
	Skill_Health_Regen_3()
	{
		Name 				= "#skill_health_regen_3_name";
		Description 		= "#skill_health_regen_3_description";
		Icon_Frame 			= "skill_health_regen_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Health_Regen_2";
		Unlock_Criteria_Threshold = 40;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Health_Regen] = 0.01;
	}
}
