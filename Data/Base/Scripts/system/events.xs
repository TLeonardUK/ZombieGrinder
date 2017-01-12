// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Placeable(false), 
	Name("Events"), 
	Description("Global class, used to pass events between nearby actors. Derive classes from this one fire them!") 
]
public native("Event") class Event
{
	public native("Fire_Global") void Fire_Global();
	public native("Fire_Global_By_Type") void Fire_Global(Type type_of_actor);

	public native("Fire_In_Radius") void Fire_In_Radius(Vec3 position, float radius);
	public native("Fire_In_Radius_By_Type") void Fire_In_Radius(Type type_of_actor, Vec3 position, float radius);
}

// Used to propogate audio events between characters for AI events.
public class Event_Hear_Sound : Event
{
	Actor Instigator;
}

// Used to propogate pawn damage events between character for AI events.
public class Event_Pawn_Hurt_AI : Event
{
	Actor Instigator;
}

// Fired globally when pawn is killed.
public class Event_Pawn_Killed : Event
{
	Pawn	        Dead;
	Actor	        Killer;
	int		        Score_Increase;
	string	        Coin_Reward;
	int		        XP_Reward;
    Item_Archetype  Weapon_Type;
}

// Fired globally when pawn is hurt.
public class Event_Pawn_Hurt : Event
{
	Pawn	Hurt;
	Actor	Harmer;			// This seems a dumb name.
	float	Damage;
	int		Score_Increase;

	CollisionDamageType Damage_Type;
	Weapon_SubType      Weapon_Sub_Type;
}

// Fired globally when pawn is revived.
public class Event_Pawn_Revived : Event
{
	Pawn	Revived;
	Actor	Reviver;
}

// Fired globally when pawn is decapitated.
public class Event_Pawn_Decapitated : Event
{
	Pawn	Victim;
	Actor	Decapitator;
}

// Particle event. 
// DO NOT MODIFY, ORDER IS ASSUME BY CODE in Game::Particle_Script_Event
public class Event_Particle : Event
{
	Vec3	Position;
}

// Fired when a movable is destroyed.
public class Event_Movable_Destroyed : Event
{
	Actor	Destroyer;
	Actor	Destroyed;
}

// Coins collected
public class Event_Coins_Collected : Event
{
	NetUser	User;
	int		Collected;
}

// Pawn afflicted with ailment.
public class Event_Pawn_Status_Ailment : Event
{
	Pawn				Effected;
	Actor				Instigator;
	StatusAilmentType	Last_Ailment;
	StatusAilmentType	Ailment;
}

// Pawn has manually reloaded.
public class Event_Pawn_Manual_Reload : Event
{
	Pawn				Reloader;
}

// Pawn has alt-fired
public class Event_Pawn_Alt_Fire : Event
{
	Pawn				Instigator;
}

// Pawn has sprinted.
public class Event_Pawn_Sprint : Event
{
	Pawn				Instigator;
}

// Pawn has strafed.
public class Event_Pawn_Strafe : Event
{
	Pawn				Instigator;
}

// Pawn has cycled weapon.
public class Event_Pawn_Cycle_Weapon : Event
{
	Pawn				Instigator;
}

// An item was dropped locally!
public class Event_Item_Dropped : Event
{
	bool IsGem;
	bool IsCoin;
}

// Candy corn was picked up.
public class Event_Candy_Corn_Picked_Up : Event
{
}
