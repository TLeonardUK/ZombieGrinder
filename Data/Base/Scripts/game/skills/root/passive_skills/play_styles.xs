// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Play_Styles : Skill_Archetype
{
	Skill_Play_Styles()
	{
		Name 				= "#skill_play_styles_name";
		Description 		= "#skill_play_styles_description";
		Icon_Frame 			= "skill_play_styles";
		Cost 				= 0;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Passive_Skills";
	}
}
