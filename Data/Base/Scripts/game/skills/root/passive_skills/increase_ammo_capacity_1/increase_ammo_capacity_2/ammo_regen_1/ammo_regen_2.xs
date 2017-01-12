// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Ammo_Regen_2 : Skill_Archetype
{
	Skill_Ammo_Regen_2()
	{
		Name 				= "#skill_ammo_regen_2_name";
		Description 		= "#skill_ammo_regen_2_description";
		Icon_Frame 			= "skill_ammo_regen_2";
		Cost 				= 3;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Ammo_Regen_1";
		Unlock_Criteria_Threshold = 35;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Ammo_Regen] = 0.005;
	}
}
