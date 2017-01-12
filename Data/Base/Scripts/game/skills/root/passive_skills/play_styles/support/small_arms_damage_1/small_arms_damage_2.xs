// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Small_Arms_Damage_2 : Skill_Archetype
{
	Skill_Small_Arms_Damage_2()
	{
		Name 				= "#skill_small_arms_damage_2_name";
		Description 		= "#skill_small_arms_damage_2_description";
		Icon_Frame 			= "skill_small_arms_damage_2";
		Cost 				= 3;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Small_Arms_Damage_1";
		Unlock_Criteria_Threshold = 7;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Small_Arms_Damage] = 1.25;
	}
}
