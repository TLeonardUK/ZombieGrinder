// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using system.network;

[
	Placeable(false), 
	Name("Cheat_Manager"), 
	Description("Singleton manager used for controlling in-game cheats.") 
]
public class Cheat_Manager
{ 
	// Collision dosen't effect players.
	[ Inspectable(true) ]
	static bool bNoClip = false;

	// Players take no damage.
	[ Inspectable(true) ]
	static bool bGodMode = false;

	// Players have unlimited ammo.
	[ Inspectable(true) ]
	static bool bUnlimitedAmmo = false;
	
	// Players have unlimited ammo.
	[ Inspectable(true) ]
	static float Reload_Multiplier = 1.0f;
	
	// Player speed multiplier.
	[ Inspectable(true) ]
	static float Speed_Multiplier = 1.0f;

	// Players start cash.
	[ Inspectable(true) ]
	static int StartCash = 0;

	// Wave to start wave based game modes at.
	[ Inspectable(true) ]
	static int StartWave = 0;
    
	// Maximum number of enemies.
	[ Inspectable(true) ]
	static int MaxEnemies = 0;

	// No energy requirements for active skills.
	[ Inspectable(true) ]
	static bool Infinite_Energy = false;

	// No cooldown time for active skills.
	[ Inspectable(true) ]
	static bool No_Cooldown = false;

	// Disables HUD for the purpose of trailers etc.
	[ Inspectable(true) ]
	static bool No_HUD = false;

	// Disables player visibility for trailers etc.
	[ Inspectable(true) ]
	static bool Player_Invisible = false;

	// Makes the player untargetable.
	[ Inspectable(true) ]
	static bool Player_Untargetable = false;

	// Boosts enemy spawn counts.
	[ Inspectable(true) ]
	static bool Boost_Enemy_Spawn_Counts = false;
    
	// Disables enemy spawns!
	[ Inspectable(true) ]
	static bool Disable_Enemy_Spawns = false;

	// Randomises local names.
	[ Inspectable(true) ]
	static bool Randomise_Names = false;

	// Gives first player a given weapon.
	[ Inspectable(true) ]
	static Type Initial_Weapon = null;
    
	// Forces only one local player to spawn. Used for testing
    // taking over enemies while waiting for spawn.
	[ Inspectable(true) ]
	static bool Spawn_Single_Player_Only = false;
    
    // Only spawns player controlled enemies. Used in conjunction with above.
	[ Inspectable(true) ]
	static bool Spawn_Player_Controlled_Enemies_Only = false;
    
    // Allows humans to spawn in all game modes regardless of eligability
	[ Inspectable(true) ]
	static bool Allow_Human_Zombie_Spawns_In_All_Modes = false;    
}
