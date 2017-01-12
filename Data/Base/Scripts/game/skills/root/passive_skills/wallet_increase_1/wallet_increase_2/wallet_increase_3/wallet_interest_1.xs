// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Wallet_Interest_1 : Skill_Archetype
{
	Skill_Wallet_Interest_1()
	{
		Name 				= "#skill_wallet_interest_1_name";
		Description 		= "#skill_wallet_interest_1_description";
		Icon_Frame 			= "skill_wallet_interest_1";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Wallet_Increase_3";
		Unlock_Criteria_Threshold = 32;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{	
		profile.Stat_Multipliers[Profile_Stat.Wallet_Interest] = 0.01;
	}
}
