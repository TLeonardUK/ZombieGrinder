// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Speed_Gunner_3 : Skill_Archetype
{
	Skill_Speed_Gunner_3()
	{
		Name 				= "#skill_speed_gunner_3_name";
		Description 		= "#skill_speed_gunner_3_description";
		Icon_Frame 			= "skill_speed_gunner_3";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Speed_Gunner_2";
		Is_Team_Based		= false;
		Duration			= 30.0f;
		Player_Effect		= "speed_gunner_30s_player";
		Unlock_Criteria_Threshold = 31;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Rate_Of_Fire] = 0.6;
		user.Skill_Stat_Multipliers[Profile_Stat.Reload_Speed] = 0.6;
	}
}
