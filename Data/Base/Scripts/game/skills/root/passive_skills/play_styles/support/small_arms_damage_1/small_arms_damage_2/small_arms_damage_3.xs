// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Small_Arms_Damage_3 : Skill_Archetype
{
	Skill_Small_Arms_Damage_3()
	{
		Name 				= "#skill_small_arms_damage_3_name";
		Description 		= "#skill_small_arms_damage_3_description";
		Icon_Frame 			= "skill_small_arms_damage_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Small_Arms_Damage_2";
		Unlock_Criteria_Threshold = 22;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Small_Arms_Damage] = 1.5;
	}
}
