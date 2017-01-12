// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Full_Ammo_2 : Skill_Archetype
{
	Skill_Full_Ammo_2()
	{
		Name 				= "#skill_full_ammo_2_name";
		Description 		= "#skill_full_ammo_2_description";
		Icon_Frame 			= "skill_full_ammo_2";
		Cost 				= 2;
        Energy_Type         = Skill_Energy_Type.Enemy_Kills;
        Energy_Required     = 300;
		Is_Passive		 	= false;
		Are_Children_Mutex 	= true;
		Parent_Name 		= "Skill_Full_Ammo_1";
		Is_Team_Based		= false;
		Duration			= 5.0f;
		Player_Effect		= "full_ammo_5s_player";
        Unlock_Criteria_Threshold = 6;
	}

	public override void Apply(NetUser user)
	{
		user.Skill_Stat_Multipliers[Profile_Stat.Ammo_Regen] = 1.0f / 5.0f;
	}
}
