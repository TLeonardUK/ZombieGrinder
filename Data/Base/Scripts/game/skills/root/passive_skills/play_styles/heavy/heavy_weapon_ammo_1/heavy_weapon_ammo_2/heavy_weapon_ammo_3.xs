// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Heavy_Weapon_Ammo_3 : Skill_Archetype
{
	Skill_Heavy_Weapon_Ammo_3()
	{
		Name 				= "#skill_heavy_weapon_ammo_3_name";
		Description 		= "#skill_heavy_weapon_ammo_3_description";
		Icon_Frame 			= "skill_heavy_weapon_ammo_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Heavy_Weapon_Ammo_2";
		Unlock_Criteria_Threshold = 13;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Heavy_Weapon_Ammo] = 2.0;
	}
}
