// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Gold_Magnet_2 : Skill_Archetype
{
	Skill_Gold_Magnet_2()
	{
		Name 				= "#skill_gold_magnet_2_name";
		Description 		= "#skill_gold_magnet_2_description";
		Icon_Frame 			= "skill_gold_magnet_2";
		Cost 				= 3;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Gold_Magnet_1";
        Unlock_Criteria_Threshold = 33;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Gold_Magnet] = 1.25;
	}
}
