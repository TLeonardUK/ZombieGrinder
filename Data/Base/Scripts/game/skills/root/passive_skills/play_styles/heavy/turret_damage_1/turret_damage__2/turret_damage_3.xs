// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Turret_Damage_3 : Skill_Archetype
{
	Skill_Turret_Damage_3()
	{
		Name 				= "#skill_turret_damage_3_name";
		Description 		= "#skill_turret_damage_3_description";
		Icon_Frame 			= "skill_turret_damage_3";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Turret_Damage__2";
		Unlock_Criteria_Threshold = 14;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Turret_Damage] = 3.0;
	}
}
