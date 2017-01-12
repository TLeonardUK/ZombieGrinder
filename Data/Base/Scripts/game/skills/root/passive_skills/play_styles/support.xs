// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Support : Skill_Archetype
{
	Skill_Support()
	{
		Name 				= "#skill_support_name";
		Description 		= "#skill_support_description";
		Icon_Frame 			= "skill_support";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Play_Styles";
	}
}
