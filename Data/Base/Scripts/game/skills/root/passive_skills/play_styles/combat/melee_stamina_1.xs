// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Melee_Stamina_1 : Skill_Archetype
{
	Skill_Melee_Stamina_1()
	{
		Name 				= "#skill_melee_stamina_1_name";
		Description 		= "#skill_melee_stamina_1_description";
		Icon_Frame 			= "skill_melee_stamina_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Combat";
        Unlock_Criteria_Threshold = 0;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Melee_Stamina] = 1.25;
	}
}
