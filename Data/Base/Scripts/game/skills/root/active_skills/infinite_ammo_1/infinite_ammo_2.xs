// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Infinite_Ammo_2 : Skill_Archetype
{
	Skill_Infinite_Ammo_2()
	{
		Name 				= "#skill_infinite_ammo_2_name";
		Description 		= "#skill_infinite_ammo_2_description";
		Icon_Frame 			= "skill_infinite_ammo_2";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 300;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Infinite_Ammo_1";
		Is_Team_Based		= false;
		Duration			= 20.0f;
		Player_Effect		= "infinite_ammo_20s_player";
        Unlock_Criteria_Threshold = 8;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Usage] = 0.0f;
	}
}
