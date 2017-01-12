// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Melee_Damage_3 : Skill_Archetype
{
	Skill_Melee_Damage_3()
	{
		Name 				= "#skill_melee_damage_3_name";
		Description 		= "#skill_melee_damage_3_description";
		Icon_Frame 			= "skill_melee_damage_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Melee_Damage_2";
		Unlock_Criteria_Threshold = 20;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Melee_Damage] = 1.5;
	}
}
