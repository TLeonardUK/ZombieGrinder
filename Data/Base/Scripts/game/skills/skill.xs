// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.skills.skill_archetype;

public enum Skill_Slot
{
	Not_Equipped = -1,
	Slot_0 = 0,
	Slot_1 = 1,
	Slot_2 = 2,

	COUNT = 3
}

[
	Name("Skill"), 
	Description("An skill is an individual unlocked skill. It essentially stores a reference to an skill-archetype as well as some custom properties.") 
]
public native("Skill") class Skill
{
	public native("Create") Skill();
	
	// Item archetype, describes generic item information.
	property Skill_Archetype Archetype
	{
		public native("Get_Archetype") Skill_Archetype Get();
		public native("Set_Archetype") void Set(Skill_Archetype value);	
	}
	
	// Item ID, used purely for serialization.
	property serialized(1) int Archetype_ID
	{
		public native("Get_Archetype_ID") int Get();
		public native("Set_Archetype_ID") void Set(int value);	
	}
 
	// Unique ID generated when item is recieved.
	property serialized(1) int Unique_ID
	{
		public native("Get_Unique_ID") int Get();
		public native("Set_Unique_ID") void Set(int value);		
	}
	
	// Time when item was recieved/purchased/etc.
	property serialized(1) int Recieve_Time
	{
		public native("Get_Recieve_Time") int Get();
		public native("Set_Recieve_Time") void Set(int value);		
	}
	 
	// Time when item was equipped.
	property serialized(1) int Equip_Time
	{
		public native("Get_Equip_Time") int Get();
		public native("Set_Equip_Time") void Set(int value);		
	}
	
	// Returns true if this skill has survived a reroll (if Cannot_Rollback is set).
	property serialized(1) bool Was_Rolled_Back
	{
		public native("Get_Was_Rolled_Back") bool Get();
		public native("Set_Was_Rolled_Back") void Set(bool value);		
	}
	
	// Slot where this skill is equipped to.
	property serialized(1) Skill_Slot Equip_Slot
	{
		public native("Get_Equip_Slot") Skill_Slot Get();
		public native("Set_Equip_Slot") void Set(Skill_Slot value);		
	}
}