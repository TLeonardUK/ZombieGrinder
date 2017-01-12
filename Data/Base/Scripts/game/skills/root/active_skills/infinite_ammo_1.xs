// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Infinite_Ammo_1 : Skill_Archetype
{
	Skill_Infinite_Ammo_1()
	{
		Name 				= "#skill_infinite_ammo_1_name";
		Description 		= "#skill_infinite_ammo_1_description";
		Icon_Frame 			= "skill_infinite_ammo_1";
		Cost 				= 1;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 200;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Active_Skills";
		Is_Team_Based		= false;
		Duration			= 10.0f;
		Player_Effect		= "infinite_ammo_10s_player";
        Unlock_Criteria_Threshold = 0;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Usage] = 0.0f;
	}
}
