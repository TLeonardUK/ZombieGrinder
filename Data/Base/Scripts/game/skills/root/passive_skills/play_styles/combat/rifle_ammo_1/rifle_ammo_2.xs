// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Rifle_Ammo_2 : Skill_Archetype
{
	Skill_Rifle_Ammo_2()
	{
		Name 				= "#skill_rifle_ammo_2_name";
		Description 		= "#skill_rifle_ammo_2_description";
		Icon_Frame 			= "skill_rifle_ammo_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Rifle_Ammo_1";
		Unlock_Criteria_Threshold = 8;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Rifle_Ammo] = 1.5;
	}
}
