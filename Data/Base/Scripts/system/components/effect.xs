// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;

[
	Name("EffectComponent"), 
	Description("When attached to an actor it displays a particle effect relative to the actors location.") 
]
public native("EffectComponent") class EffectComponent
{ 
	// Creates a new effect component and binds it to the executing actor.
	// Shouldn't be called directly, use a Components {} block.
	public native("Create") EffectComponent();
	
	// Gets/sets if the effect is rendering.
	property serialized(1) bool Visible
	{
		public native("Set_Visible") void Set(bool val);
		public native("Get_Visible") bool Get();
	}
	
	// Gets/sets if the effect is paused.
	property serialized(1) bool Paused
	{
		public native("Set_Paused") void Set(bool val);
		public native("Get_Paused") bool Get();
	}
	
	// Gets/sets the effect resource.
	property serialized(1) string Effect_Name
	{
		public native("Set_Effect_Name") void Set(string val);
		public native("Get_Effect_Name") string Get();
	}
	
	// Gets/sets if the effect respawns after finishing.
	property serialized(1) bool One_Shot
	{
		public native("Set_One_Shot") void Set(bool val);
		public native("Get_One_Shot") bool Get();
	}	
	
	// Gets/sets the offset to place effect at.
	property serialized(1) Vec3 Offset
	{
		public native("Set_Offset") void Set(Vec3 val);
		public native("Get_Offset") Vec3 Get();
	}	

	// Gets/sets the offset to place effect at.
	property serialized(501) Vec3 Angle_Offset
	{
		public native("Set_Angle_Offset") void Set(Vec3 val);
		public native("Get_Angle_Offset") Vec3 Get();
	}	

	// Gets/sets if the effect respawns after finishing.
	property bool Is_Finished
	{
		public native("Get_Finished") bool Get();
	}	

	// Gets the instigator of this effect. eg. the one who fired the gun, who created the explosion, etc.
	property Actor Instigator
	{
		public native("Get_Instigator") Actor Get();
		public native("Set_Instigator") void  Set(Actor instigator);
	}	
	
	// Gets the meta number.
	property int Meta_Number
	{
		public native("Get_Meta_Number") int Get();
		public native("Set_Meta_Number") void Set(int value);
	}	

	// Gets the modifier.
	property string Modifier
	{
		public native("Get_Modifier") string Get();
		public native("Set_Modifier") void Set(string value);
	}	
	
	// Gets the subtype.
	property int SubType
	{
		public native("Get_Sub_Type") int Get();
		public native("Set_Sub_Type") void Set(int value);
	}

    // Gets the weapon type
    // STAT_TODO
    property Item_Archetype Weapon_Type
	{
		public native("Get_Weapon_Type") Item_Archetype Get();
		public native("Set_Weapon_Type") void Set(Item_Archetype value);
	}	

	// Gets/sets if this effect ignores spawn collision.
	property bool Ignore_Spawn_Collision
	{
		public native("Get_Ignore_Spawn_Collision") bool Get();
		public native("Set_Ignore_Spawn_Collision") void Set(bool value);
	}	

    // Modifiers that effect how the effect plays out.
    property float[] Upgrade_Modifiers
	{
		public native("Get_Upgrade_Modifiers") float[] Get();
		public native("Set_Upgrade_Modifiers") void Set(float[] value);
	}	

	// Restarts the effect.
	public native("Restart") void Restart();
}