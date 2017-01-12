// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Melee_Damage_2 : Skill_Archetype
{
	Skill_Melee_Damage_2()
	{
		Name 				= "#skill_melee_damage_2_name";
		Description 		= "#skill_melee_damage_2_description";
		Icon_Frame 			= "skill_melee_damage_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Melee_Damage_1";
		Unlock_Criteria_Threshold = 9;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Melee_Damage] = 1.2;
	}
}
