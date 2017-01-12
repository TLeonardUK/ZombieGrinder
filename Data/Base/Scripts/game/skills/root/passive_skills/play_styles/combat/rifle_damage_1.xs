// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Rifle_Damage_1 : Skill_Archetype
{
	Skill_Rifle_Damage_1()
	{
		Name 				= "#skill_rifle_damage_1_name";
		Description 		= "#skill_rifle_damage_1_description";
		Icon_Frame 			= "skill_rifle_damage_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Combat";
        Unlock_Criteria_Threshold = 0;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Rifle_Damage] = 1.1;
	}
}
