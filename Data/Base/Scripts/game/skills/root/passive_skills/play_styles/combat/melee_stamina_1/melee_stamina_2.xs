// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Melee_Stamina_2 : Skill_Archetype
{
	Skill_Melee_Stamina_2()
	{
		Name 				= "#skill_melee_stamina_2_name";
		Description 		= "#skill_melee_stamina_2_description";
		Icon_Frame 			= "skill_melee_stamina_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Melee_Stamina_1";
		Unlock_Criteria_Threshold = 8;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Melee_Stamina] = 1.50;
	}
}
