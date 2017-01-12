// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Speed_Gunner_4 : Skill_Archetype
{
	Skill_Speed_Gunner_4()
	{
		Name 				= "#skill_speed_gunner_4_name";
		Description 		= "#skill_speed_gunner_4_description";
		Icon_Frame 			= "skill_speed_gunner_4";
		Cost 				= 5;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Speed_Gunner_3";
		Is_Team_Based		= false;
		Duration			= 30.0f;
		Player_Effect		= "speed_gunner_30s_player";
		Unlock_Criteria_Threshold = 34;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Rate_Of_Fire] = 0.5;
		user.Skill_Stat_Multipliers[Profile_Stat.Reload_Speed] = 0.5;
	}
}
