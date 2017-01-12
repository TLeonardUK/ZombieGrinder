// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Wallet_Increase_1 : Skill_Archetype
{
	Skill_Wallet_Increase_1()
	{
		Name 				= "#skill_wallet_increase_1_name";
		Description 		= "#skill_wallet_increase_1_description";
		Icon_Frame 			= "skill_wallet_increase_1";
		Cost 				= 1;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Passive_Skills";
        Unlock_Criteria_Threshold = 0;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Wallet_Size = profile.Start_Wallet_Size * 1.5;
	}
}
