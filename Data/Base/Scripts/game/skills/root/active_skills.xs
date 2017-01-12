// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Active_Skills : Skill_Archetype
{
	Skill_Active_Skills()
	{
		Name 				= "#skill_active_skills_name";
		Description 		= "#skill_active_skills_description";
		Icon_Frame 			= "skill_active_skills";
		Cost 				= 0;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Root";
	}
}
