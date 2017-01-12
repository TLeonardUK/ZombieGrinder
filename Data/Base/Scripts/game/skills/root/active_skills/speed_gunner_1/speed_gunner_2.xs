// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Speed_Gunner_2 : Skill_Archetype
{
	Skill_Speed_Gunner_2()
	{
		Name 				= "#skill_speed_gunner_2_name";
		Description 		= "#skill_speed_gunner_2_description";
		Icon_Frame 			= "skill_speed_gunner_2";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 200;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Speed_Gunner_1";
		Is_Team_Based		= false;
		Duration			= 30.0f;
		Player_Effect		= "speed_gunner_30s_player";
		Unlock_Criteria_Threshold = 21;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Rate_Of_Fire] = 0.75;
		user.Skill_Stat_Multipliers[Profile_Stat.Reload_Speed] = 0.75;
	}
}
