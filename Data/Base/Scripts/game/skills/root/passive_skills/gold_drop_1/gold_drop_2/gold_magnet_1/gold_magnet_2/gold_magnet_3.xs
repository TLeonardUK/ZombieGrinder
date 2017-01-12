// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Gold_Magnet_3 : Skill_Archetype
{
	Skill_Gold_Magnet_3()
	{
		Name 				= "#skill_gold_magnet_3_name";
		Description 		= "#skill_gold_magnet_3_description";
		Icon_Frame 			= "skill_gold_magnet_3";
		Cost 				= 4;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Gold_Magnet_2";
		Unlock_Criteria_Threshold = 38;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Gold_Magnet] = 1.5;
	}
}
