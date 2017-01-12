// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Gold_Drop_3 : Skill_Archetype
{
	Skill_Gold_Drop_3()
	{
		Name 				= "#skill_gold_drop_3_name";
		Description 		= "#skill_gold_drop_3_description";
		Icon_Frame 			= "skill_gold_drop_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Gold_Drop_2";
        Unlock_Criteria_Threshold = 21;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Gold_Drop] = 1.5;
	}
}
