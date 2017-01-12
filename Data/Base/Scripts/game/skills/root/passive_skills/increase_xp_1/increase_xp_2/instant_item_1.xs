// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill_archetype;
using game.items.hats.item_bison_cap_3;

public class Skill_Instant_Item_1 : Skill_Archetype
{
	Skill_Instant_Item_1()
	{
		Name 				= "#skill_instant_item_1_name";
		Description 		= "#skill_instant_item_1_description";
		Icon_Frame 			= "skill_instant_item_1";
		Cost 				= 2;
		Is_Passive		 	= true;
		Are_Children_Mutex 	= false;
		Cannot_Rollback		= true;
		Parent_Name 		= "Skill_Increase_XP_2";
	}
	
	override void On_Recieve(Profile profile, Skill skill)
	{
		profile.Add_Item(Item_Archetype.Find_By_Name("Item_Bison_Cap_3"));
	}
}
