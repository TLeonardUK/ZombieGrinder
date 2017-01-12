// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Group_Insta_Kill_3 : Skill_Archetype
{
	Skill_Group_Insta_Kill_3()
	{
		Name 				= "#skill_group_insta_kill_3_name";
		Description 		= "#skill_group_insta_kill_3_description";
		Icon_Frame 			= "skill_group_insta_kill_3";
		Cost 				= 5;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 1000;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Group_Insta_Kill_2";
		Is_Team_Based		= true;
		Duration			= 30.0f;
		Player_Effect		= "insta_kill_30s_player";
		Unlock_Criteria_Threshold = 43;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Damage] = 9999.0;
	}
}
