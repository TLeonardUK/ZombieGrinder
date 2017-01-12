// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Infinite_Ammo_3 : Skill_Archetype
{
	Skill_Infinite_Ammo_3()
	{
		Name 				= "#skill_infinite_ammo_3_name";
		Description 		= "#skill_infinite_ammo_3_description";
		Icon_Frame 			= "skill_infinite_ammo_3";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Infinite_Ammo_2";
		Is_Team_Based		= false;
		Duration			= 30.0f;
		Player_Effect		= "infinite_ammo_30s_player";
        Unlock_Criteria_Threshold = 18;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Usage] = 0.0f;
	}
}
