// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Gold_Drop_2 : Skill_Archetype
{
	Skill_Gold_Drop_2()
	{
		Name 				= "#skill_gold_drop_2_name";
		Description 		= "#skill_gold_drop_2_description";
		Icon_Frame 			= "skill_gold_drop_2";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Gold_Drop_1";
		Unlock_Criteria_Threshold = 12;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Gold_Drop] = 1.25;
	}
}
