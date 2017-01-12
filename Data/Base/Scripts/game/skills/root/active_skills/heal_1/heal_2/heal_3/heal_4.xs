// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Heal_4 : Skill_Archetype
{
	Skill_Heal_4()
	{
		Name 				= "#skill_heal_4_name";
		Description 		= "#skill_heal_4_description";
		Icon_Frame 			= "skill_heal_4";
		Cost 				= 5;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Heal_3";
		Is_Team_Based		= false;
		Duration			= 0.0f;
		Player_Effect		= "heal_0s_player";
        Unlock_Criteria_Threshold = 27;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Health_Regen] = 999.0f;
	}
}
