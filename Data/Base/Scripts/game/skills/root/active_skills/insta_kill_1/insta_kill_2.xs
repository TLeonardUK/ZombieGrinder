// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Insta_Kill_2 : Skill_Archetype
{
	Skill_Insta_Kill_2()
	{
		Name 				= "#skill_insta_kill_2_name";
		Description 		= "#skill_insta_kill_2_description";
		Icon_Frame 			= "skill_insta_kill_2";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 300;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Insta_Kill_1";
		Is_Team_Based		= false;
		Duration			= 15.0f;	
		Player_Effect		= "insta_kill_15s_player";
        Unlock_Criteria_Threshold = 14;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Damage] = 9999.0;
	}
}
