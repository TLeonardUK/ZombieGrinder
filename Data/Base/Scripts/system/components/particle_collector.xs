// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;

// Types of collectible particles. Keep en sync with code in ParticleManager.h
public enum ParticleCollectionType
{
	Coins
}

[
	Name("ParticleCollectorComponent"), 
	Description("Used as a global attractor to pickup collectable particles (coins etc).") 
]
public native("ParticleCollectorComponent") class ParticleCollectorComponent
{ 
	// Creates a new effect component and binds it to the executing actor.
	// Shouldn't be called directly, use a Components {} block.
	public native("Create") ParticleCollectorComponent();
	
	// Gets/sets if the effect is paused.
	property serialized(1) bool Is_Paused
	{
		public native("Set_Is_Paused") void Set(bool val);
		public native("Get_Is_Paused") bool Get();
	}
	
	// Gets/sets the offset to place effect at.
	property serialized(1) Vec3 Offset
	{
		public native("Set_Offset") void Set(Vec3 val);
		public native("Get_Offset") Vec3 Get();
	}	
	
	// Gets/sets radius to collect within.
	property serialized(1) float Radius
	{
		public native("Set_Radius") void Set(float val);
		public native("Get_Radius") float Get();
	}
	
	// Gets/sets radius to collect within.
	property serialized(1) float Strength
	{
		public native("Set_Strength") void Set(float val);
		public native("Get_Strength") float Get();
	}

	public native("Get_Collected") int Get_Collectod(int type);
}