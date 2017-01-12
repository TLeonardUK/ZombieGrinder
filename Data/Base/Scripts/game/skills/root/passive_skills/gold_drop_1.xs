// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Gold_Drop_1 : Skill_Archetype
{ 
	Skill_Gold_Drop_1()
	{
		Name 				= "#skill_gold_drop_1_name";
		Description 		= "#skill_gold_drop_1_description";
		Icon_Frame 			= "skill_gold_drop_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Passive_Skills";
        Unlock_Criteria_Threshold = 11;
	}
}
