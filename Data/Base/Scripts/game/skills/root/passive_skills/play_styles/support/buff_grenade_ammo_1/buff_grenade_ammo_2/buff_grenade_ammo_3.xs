// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public class Skill_Buff_Grenade_Ammo_3 : Skill_Archetype
{
	Skill_Buff_Grenade_Ammo_3()
	{
		Name 				= "#skill_buff_grenade_ammo_3_name";
		Description 		= "#skill_buff_grenade_ammo_3_description";
		Icon_Frame 			= "skill_buff_grenade_ammo_3";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Buff_Grenade_Ammo_2";
		Unlock_Criteria_Threshold = 22;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Stat_Multipliers[Profile_Stat.Buff_Grenade_Ammo] = 2.5;
	}
}
 