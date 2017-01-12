// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Inventory_Increase_1 : Skill_Archetype
{
	Skill_Inventory_Increase_1()
	{
		Name 				= "#skill_inventory_increase_1_name";
		Description 		= "#skill_inventory_increase_1_description";
		Icon_Frame 			= "skill_inventory_increase_1";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Wallet_Increase_1";
		Unlock_Criteria_Threshold = 10;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
        profile.Inventory_Size = Math.Min(profile.Max_Inventory_Size, profile.Inventory_Slots_Per_Page * 2);
	}

	override void On_Loaded(Profile profile, Skill skill)
	{
        profile.Inventory_Size = Math.Clamp(profile.Inventory_Size, profile.Inventory_Slots_Per_Page * 2, profile.Max_Inventory_Size);
	} 
}
