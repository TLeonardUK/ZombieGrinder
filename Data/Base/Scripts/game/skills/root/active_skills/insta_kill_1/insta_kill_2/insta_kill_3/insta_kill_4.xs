// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Insta_Kill_4 : Skill_Archetype
{
	Skill_Insta_Kill_4()
	{
		Name 				= "#skill_insta_kill_4_name";
		Description 		= "#skill_insta_kill_4_description";
		Icon_Frame 			= "skill_insta_kill_4";
		Cost 				= 4;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Insta_Kill_3";
		Is_Team_Based		= false;
		Duration			= 30.0f;
		Player_Effect		= "insta_kill_30s_player";
        Unlock_Criteria_Threshold = 26;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Damage] = 9999.0;
	}
}
