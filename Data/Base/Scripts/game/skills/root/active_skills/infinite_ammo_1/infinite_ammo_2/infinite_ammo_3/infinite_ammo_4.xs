// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Infinite_Ammo_4 : Skill_Archetype
{
	Skill_Infinite_Ammo_4()
	{
		Name 				= "#skill_infinite_ammo_4_name";
		Description 		= "#skill_infinite_ammo_4_description";
		Icon_Frame 			= "skill_infinite_ammo_4";
		Cost 				= 5;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Infinite_Ammo_3";
		Is_Team_Based		= false;
		Duration			= 60.0f;
		Player_Effect		= "infinite_ammo_60s_player";
        Unlock_Criteria_Threshold = 35;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Usage] = 0.0f;
	}
}
