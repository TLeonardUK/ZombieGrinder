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
	Name("Player Large Blobby Zombie Controller"), 
	Description("Allows players to control a large blobby zombie!") 
]
public class Player_Chaser_Zombie_Controller : Player_Generic_Zombie_Controller
{
	private serialized(1) float m_walk_speed_variance = 0.0f;
    	
	const float BASE_WALK_SPEED			= 10.0;	
	const float WALK_SPEED_VARIANCE		= 5.0;	
	const float FAST_ZOMBIE_VARIANCE	= 10.0f;
	const float FAST_ZOMBIE_PROBABILITY	= 0.1;	
	const float HUMAN_SPEED_MULTIPLIER	= 1.25;	
	
	Player_Chaser_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
		m_walk_speed_variance = Math.Rand(0.0f, WALK_SPEED_VARIANCE);

		if (Math.Rand(0.0f, 1.0f) <= FAST_ZOMBIE_PROBABILITY)
		{
			m_walk_speed_variance = FAST_ZOMBIE_VARIANCE;
		}
	}

	override float Get_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return (BASE_WALK_SPEED + m_walk_speed_variance) * HUMAN_SPEED_MULTIPLIER * Difficulty_Manager.Enemy_Speed_Multiplier * speed_modifier;
	}

    override bool Can_Shoot()
    {
		Pawn pawn = Possessed;
		Chaser_Zombie zombie = <Chaser_Zombie>pawn;

        return !zombie.Is_Decapitated;
    }

    override bool Can_Move()
    {
		Pawn pawn = Possessed;
		Chaser_Zombie zombie = <Chaser_Zombie>pawn;

        // Cannot move while crawling from the earth.
        if (!zombie.Has_Crawled_From_Earth)
        {
            return false;
        }

        return true;
    }    
}
