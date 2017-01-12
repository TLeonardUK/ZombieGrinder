// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Group_Blind_Zombies_2 : Skill_Archetype
{
	Skill_Group_Blind_Zombies_2()
	{
		Name 				= "#skill_group_blind_zombies_2_name";
		Description 		= "#skill_group_blind_zombies_2_description";
		Icon_Frame 			= "skill_group_blind_zombies_2";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Group_Blind_Zombies_1";
		Is_Team_Based		= true;
		Duration			= 20.0f;
		Player_Effect		= "blind_zombies_20s_player";
        Unlock_Criteria_Threshold = 31;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Targetable] = 0.0f;
	}
}
