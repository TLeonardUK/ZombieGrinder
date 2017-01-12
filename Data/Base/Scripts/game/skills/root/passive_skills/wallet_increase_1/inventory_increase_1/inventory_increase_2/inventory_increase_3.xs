// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;

public class Skill_Inventory_Increase_3 : Skill_Archetype
{
	Skill_Inventory_Increase_3()
	{
		Name 				= "#skill_inventory_increase_3_name";
		Description 		= "#skill_inventory_increase_3_description";
		Icon_Frame 			= "skill_inventory_increase_3";
		Cost 				= 4;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Parent_Name 		= "Skill_Inventory_Increase_2";
		Unlock_Criteria_Threshold = 36;
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
        profile.Inventory_Size = Math.Min(profile.Max_Inventory_Size, profile.Inventory_Slots_Per_Page * 4);
	}

	override void On_Loaded(Profile profile, Skill skill)
	{	
        profile.Inventory_Size = Math.Clamp(profile.Inventory_Size, profile.Inventory_Slots_Per_Page * 4, profile.Max_Inventory_Size);
	} 
}
