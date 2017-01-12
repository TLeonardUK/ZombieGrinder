// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Ammo_Regen_1 : Skill_Archetype
{
	Skill_Ammo_Regen_1()
	{
		Name 				= "#skill_ammo_regen_1_name";
		Description 		= "#skill_ammo_regen_1_description";
		Icon_Frame 			= "skill_ammo_regen_1";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Increase_Ammo_Capacity_2";
		Unlock_Criteria_Threshold = 27;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Ammo_Regen] = 0.003;
	}
}
