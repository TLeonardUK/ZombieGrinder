// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Turret_Damage__2 : Skill_Archetype
{
	Skill_Turret_Damage__2()
	{
		Name 				= "#skill_turret_damage__2_name";
		Description 		= "#skill_turret_damage__2_description";
		Icon_Frame 			= "skill_turret_damage__2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Turret_Damage_1";
		Unlock_Criteria_Threshold = 5;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Turret_Damage] = 2.0;
	}
}
