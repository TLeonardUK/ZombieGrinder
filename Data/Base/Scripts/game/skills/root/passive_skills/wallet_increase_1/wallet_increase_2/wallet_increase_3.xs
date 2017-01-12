// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Wallet_Increase_3 : Skill_Archetype
{
	Skill_Wallet_Increase_3()
	{
		Name 				= "#skill_wallet_increase_3_name";
		Description 		= "#skill_wallet_increase_3_description";
		Icon_Frame 			= "skill_wallet_increase_3";
		Cost 				= 5;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Wallet_Increase_2";
		Unlock_Criteria_Threshold = 24;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Wallet_Size = profile.Start_Wallet_Size * 2.0;
	}
}
