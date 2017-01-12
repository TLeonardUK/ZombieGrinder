// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Blind_Zombies_3 : Skill_Archetype
{
	Skill_Blind_Zombies_3()
	{
		Name 				= "#skill_blind_zombies_3_name";
		Description 		= "#skill_blind_zombies_3_description";
		Icon_Frame 			= "skill_blind_zombies_3";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Blind_Zombies_2";
		Is_Team_Based		= false;
		Duration			= 20.0f;
		Player_Effect		= "blind_zombies_20s_player";
        Unlock_Criteria_Threshold = 23;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Targetable] = 0.0f;
	}
}
