// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Blind_Zombies_4 : Skill_Archetype
{
	Skill_Blind_Zombies_4()
	{
		Name 				= "#skill_blind_zombies_4_name";
		Description 		= "#skill_blind_zombies_4_description";
		Icon_Frame 			= "skill_blind_zombies_4";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Blind_Zombies_3";
		Is_Team_Based		= false;
		Duration			= 30.0f;
		Player_Effect		= "blind_zombies_30s_player";
        Unlock_Criteria_Threshold = 28;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Targetable] = 0.0f;
	}
}
