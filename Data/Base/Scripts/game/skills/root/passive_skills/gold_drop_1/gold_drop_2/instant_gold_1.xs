// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Instant_Gold_1 : Skill_Archetype
{
	Skill_Instant_Gold_1()
	{
		Name 				= "#skill_instant_gold_1_name";
		Description 		= "#skill_instant_gold_1_description";
		Icon_Frame 			= "skill_instant_gold_1";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Cannot_Rollback		= true;
		Parent_Name 		= "Skill_Gold_Drop_2";
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Coins += 100000;
	}
}
