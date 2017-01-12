// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Instant_Skill_1 : Skill_Archetype
{
	Skill_Instant_Skill_1()
	{
		Name 				= "#skill_instant_skill_1_name";
		Description 		= "#skill_instant_skill_1_description";
		Icon_Frame 			= "skill_instant_skill_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Cannot_Rollback		= true;
		Parent_Name 		= "Skill_Increase_HP_2";
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Skill_Points += 5;
	}
}
