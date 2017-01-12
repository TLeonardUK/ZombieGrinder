// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Ammo_Regen_3 : Skill_Archetype
{
	Skill_Ammo_Regen_3()
	{
		Name 				= "#skill_ammo_regen_3_name";
		Description 		= "#skill_ammo_regen_3_description";
		Icon_Frame 			= "skill_ammo_regen_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Ammo_Regen_2";
		Unlock_Criteria_Threshold = 39;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Ammo_Regen] = 0.01;
	}
}
