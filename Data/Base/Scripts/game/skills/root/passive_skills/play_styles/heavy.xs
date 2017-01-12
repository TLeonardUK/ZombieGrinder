// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Heavy : Skill_Archetype
{
	Skill_Heavy()
	{
		Name 				= "#skill_heavy_name";
		Description 		= "#skill_heavy_description";
		Icon_Frame 			= "skill_heavy";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Play_Styles";
	}
}
