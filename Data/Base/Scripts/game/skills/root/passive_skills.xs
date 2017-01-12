// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Passive_Skills : Skill_Archetype
{
	Skill_Passive_Skills()
	{
		Name 				= "#skill_passive_skills_name";
		Description 		= "#skill_passive_skills_description";
		Icon_Frame 			= "skill_passive_skills";
		Cost 				= 0;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Root";
	}
}
