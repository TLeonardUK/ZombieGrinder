// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.time;
using system.components.collision;
using system.components.path_planner;
using system.events;
using game.difficulty.difficulty_manager;
using game.actors.enemies.chaser_zombie;
using game.actors.decoration.movable.movable;

[
	Name("Player Floater Zombie Controller"), 
	Description("Allows players to control a floater zombie!") 
]
public class Player_Floater_Zombie_Controller : Player_Generic_Zombie_Controller
{
	private serialized(1) float m_walk_speed_variance = 0.0f;
    	
	const float BASE_WALK_SPEED			= 9.0;	
	const float WALK_SPEED_VARIANCE		= 2.0;
	const float HUMAN_SPEED_MULTIPLIER	= 1.25;	
	
	Player_Floater_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
		m_walk_speed_variance = Math.Rand(0.0f, WALK_SPEED_VARIANCE);
        //bLockDirectionTo2 = true;
	}

	override float Get_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return (BASE_WALK_SPEED + m_walk_speed_variance) * HUMAN_SPEED_MULTIPLIER * Difficulty_Manager.Enemy_Speed_Multiplier * speed_modifier;
	}             
}
