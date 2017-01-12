// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Insta_Kill_3 : Skill_Archetype
{
	Skill_Insta_Kill_3()
	{
		Name 				= "#skill_insta_kill_3_name";
		Description 		= "#skill_insta_kill_3_description";
		Icon_Frame 			= "skill_insta_kill_3";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Insta_Kill_2";
		Is_Team_Based		= false;
		Duration			= 20.0f;
		Player_Effect		= "insta_kill_20s_player";
        Unlock_Criteria_Threshold = 16;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Damage] = 9999.0;
	}
}
