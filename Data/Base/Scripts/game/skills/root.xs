// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Root : Skill_Archetype
{
	Skill_Root()
	{
		Name 				= "#skill_root_name";
		Description 		= "#skill_root_description";
		Icon_Frame 			= "skill_root";
		Cost 				= 0;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "";
	}
}
