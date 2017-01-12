// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;

[
	Placeable(false), 
	Name("Interactable"), 
	Description("Defines the methods required to make an object interactable by players.") 
]
public interface Interactable
{
	// Returns true if the given pawn can interact with us.
	public abstract bool Can_Interact(Pawn other);

	// Gets prompt shown to the user to interact "Press X To Herpaderp"
	public abstract string Get_Prompt(Pawn other);

	// Gets time in seconds that interaction requires. Progress bar will
	// be drawn if set. Return 0 for instant.
	public abstract float Get_Duration(Pawn other);
	
	// Gets the price in coins to interact with this object. Or 0 for free.
	public abstract int Get_Cost(Pawn other);

	// Causes interaction!
	public abstract void Interact(Pawn other);
}
