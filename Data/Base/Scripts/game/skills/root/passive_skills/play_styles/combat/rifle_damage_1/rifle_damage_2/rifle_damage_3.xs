// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Rifle_Damage_3 : Skill_Archetype
{
	Skill_Rifle_Damage_3()
	{
		Name 				= "#skill_rifle_damage_3_name";
		Description 		= "#skill_rifle_damage_3_description";
		Icon_Frame 			= "skill_rifle_damage_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Rifle_Damage_2";
		Unlock_Criteria_Threshold = 19;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Rifle_Damage] = 1.5;
	}
}
