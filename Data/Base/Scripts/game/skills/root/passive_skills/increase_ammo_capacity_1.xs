// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Increase_Ammo_Capacity_1 : Skill_Archetype
{
	Skill_Increase_Ammo_Capacity_1()
	{
		Name 				= "#skill_increase_ammo_capacity_1_name";
		Description 		= "#skill_increase_ammo_capacity_1_description";
		Icon_Frame 			= "skill_increase_ammo_capacity_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Passive_Skills";
        Unlock_Criteria_Threshold = 0;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Ammo_Capacity] = 1.1;
	}
}
