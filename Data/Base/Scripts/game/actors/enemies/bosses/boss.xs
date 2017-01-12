// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.enemies.enemy;
using system.network;
using system.components.sprite;
using system.components.collision;
using system.components.boid;
using system.actors.actor;
using system.time;
using system.fx;
using system.actors.pawn;
using system.events;
using game.profile.profile;
using game.weapons.enemies.weapon_chaser_zombie;
using game.difficulty.difficulty_manager;
 
[
	Placeable(false), 
	Name("Boss"), 
	Description("Base class for all boss enemies.") 
]
public class Boss : Enemy
{
    public virtual string Get_Boss_Intro_Text()
    {
        return "Untitled Boss";
    }
}