// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.profile.profile;
using game.skills.skill;

public enum Skill_Unlock_Criteria
{
	Required_Rank = 0,
	Kills_With_Weapon = 1,
	Damage_With_Weapon = 2,
	Ailments_From_Weapon = 3
}

public enum Skill_Energy_Type
{
    Enemy_Kills = 0,
}

[
	Name("Skill_Archetype"), 
	Description("A skill archetype defines all the shared properties of an skill.") 
]
public native("Skill_Archetype") class Skill_Archetype
{
	property string Name
	{
		public native("Get_Name") string Get();
		public native("Set_Name") void Set(string value);	
	}
	
	property string Description
	{
		public native("Get_Description") string Get();
		public native("Set_Description") void Set(string value);	
	}
	
	property string Icon_Frame
	{
		public native("Get_Icon_Frame") string Get();
		public native("Set_Icon_Frame") void Set(string value);	
	}
	
	property int Cost
	{
		public native("Get_Cost") int Get();
		public native("Set_Cost") void Set(int value);	
	}
	

    property float Energy_Required
	{
		public native("Get_Energy_Required") float Get();
		public native("Set_Energy_Required") void Set(float value);	
	}
	
    property Skill_Energy_Type Energy_Type
	{
		public native("Get_Energy_Type") Skill_Energy_Type Get();
		public native("Set_Energy_Type") void Set(Skill_Energy_Type value);	
	}
	


	property bool Is_Passive
	{
		public native("Get_Is_Passive") bool Get();
		public native("Set_Is_Passive") void Set(bool value);	
	}
	
	property bool Cannot_Rollback
	{
		public native("Get_Cannot_Rollback") bool Get();
		public native("Set_Cannot_Rollback") void Set(bool value);	
	}
	
	property bool Are_Children_Mutex
	{
		public native("Get_Are_Children_Mutex") bool Get();
		public native("Set_Are_Children_Mutex") void Set(bool value);	
	}

	property string Parent_Name
	{
		public native("Get_Parent_Name") string Get();
		public native("Set_Parent_Name") void Set(string value);	
	}
	
	property string Player_Effect
	{
		public native("Get_Player_Effect") string Get();
		public native("Set_Player_Effect") void Set(string value);	
	}
	
	property bool Is_Team_Based
	{
		public native("Get_Is_Team_Based") bool Get();
		public native("Set_Is_Team_Based") void Set(bool value);	
	}

	property float Duration
	{
		public native("Get_Duration") float Get();
		public native("Set_Duration") void Set(float value);	
	}
	
	// Unlock criterial.
	property Skill_Unlock_Criteria Unlock_Criteria
	{
		public native("Get_Unlock_Criteria") Item_Unlock_Criteria Get();
		public native("Set_Unlock_Criteria") void Set(Item_Unlock_Criteria value);	
	}
	
	property Type Unlock_Criteria_Item
	{
		public native("Get_Unlock_Criteria_Item") Type Get();
		public native("Set_Unlock_Criteria_Item") void Set(Type value);	
	}
	
	property int Unlock_Criteria_Threshold
	{
		public native("Get_Unlock_Criteria_Threshold") int Get();
		public native("Set_Unlock_Criteria_Threshold") void Set(int value);	
	}

	public virtual void On_Recieve(Profile profile, Skill skill)
	{	
		// Do stuff here if we have special-requirements. Giving user item etc.
		//Log.Write("Skill.On_Recieve = " + skill.Archetype.Name);
	} 
    
	public virtual void On_Loaded(Profile profile, Skill skill)
	{	
        // Used for adjusting any changes made in subsequent versions.
	//	Log.Write("Skill.On_Loaded = " + skill.Archetype.Name);
	} 

	public virtual void Apply(NetUser user)
	{
		Log.Write("No override for - "+GetType().Name+".Apply(" + user.Net_ID+")");
	}
	
	public static native("Find_By_Name") Skill_Archetype Find_By_Name(string name);
	public static native("Find_By_ID") Skill_Archetype Find_By_ID(int id);
}
