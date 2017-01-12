// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Full_Ammo_4 : Skill_Archetype
{
	Skill_Full_Ammo_4()
	{
		Name 				= "#skill_full_ammo_4_name";
		Description 		= "#skill_full_ammo_4_description";
		Icon_Frame 			= "skill_full_ammo_4";
		Cost 				= 5;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 700;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Full_Ammo_3";
		Is_Team_Based		= false;
		Duration			= 0.0f;
		Player_Effect		= "full_ammo_0s_player";
        Unlock_Criteria_Threshold = 31;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Regen] = 999.0f;
	}
}
