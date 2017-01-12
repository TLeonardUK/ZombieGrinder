// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(false), 
	Name("Player"), 
	Description("Base class for all player types. Shouldn't be instantiated directly, spawn one of the derived classes instead.") 
]
public class Player : Actor
{ 
	replicate(ReplicationPriority.High)
	{		
	}
}