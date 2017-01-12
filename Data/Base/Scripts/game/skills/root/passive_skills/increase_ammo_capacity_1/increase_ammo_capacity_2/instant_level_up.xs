// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Instant_Level_Up : Skill_Archetype
{
	Skill_Instant_Level_Up()
	{
		Name 				= "#skill_instant_level_up_name";
		Description 		= "#skill_instant_level_up_description";
		Icon_Frame 			= "skill_instant_level_up";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Cannot_Rollback		= true;
		Parent_Name 		= "Skill_Increase_Ammo_Capacity_2";
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Level_Up();
	}
}
