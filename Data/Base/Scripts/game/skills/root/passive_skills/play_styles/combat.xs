// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Combat : Skill_Archetype
{
	Skill_Combat()
	{
		Name 				= "#skill_combat_name";
		Description 		= "#skill_combat_description";
		Icon_Frame 			= "skill_combat";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Play_Styles";
	}
}
