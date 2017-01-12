// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Full_Ammo_3 : Skill_Archetype
{
	Skill_Full_Ammo_3()
	{
		Name 				= "#skill_full_ammo_3_name";
		Description 		= "#skill_full_ammo_3_description";
		Icon_Frame 			= "skill_full_ammo_3";
		Cost 				= 3;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 500;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Full_Ammo_2";
		Is_Team_Based		= false;
		Duration			= 3.0f;
		Player_Effect		= "full_ammo_3s_player";
        Unlock_Criteria_Threshold = 23;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Regen] = 1.0f / 3.0f;
	}
}
