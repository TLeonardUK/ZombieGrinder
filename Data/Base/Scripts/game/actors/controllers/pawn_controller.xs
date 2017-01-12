 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.scene;

[
	Name("Pawn Controller"), 
	Description("Base class for all pawn controllers in game. Includes additional functionality.") 
]
public class Pawn_Controller : Controller
{
	private Vec3 m_movement_delta;
	
    // General behaviour
	bool Move_Towards(Vec3 target, float speed, bool checkDirection, Direction oldDirection, bool limitTo4Dir = false)
	{ 
		Pawn pawn = Possessed;
		
		Vec3 start_position = pawn.Position;
		Vec3 final_position = start_position;
		float delta = Time.Get_Delta();
		bool at_target = false;

		// Apply movement.
		float vector_to_target_magnitude = (target - final_position).Length();
		Vec3 vector_to_target = (target - final_position).Unit();
		Vec3 movement = vector_to_target * Math.Min(vector_to_target_magnitude, speed * delta);
        
		final_position = final_position + movement;
		Vec3 vector_to_target_after = (target - final_position).Unit();
        
		// If this movement will take us onto or past the target, then we are at the end of our movement.
		at_target = vector_to_target.Dot(vector_to_target_after) < 0.0 || vector_to_target_after == Vec3(0.0f) || vector_to_target_after.Length() < 0.01;

        if (checkDirection)
        {
            // Turn towards direction of movement.
            Vec3 position_delta = final_position - pawn.Position;
            // Pervent flickering if we move one direction then the other in quick succession
            m_movement_delta = Math.Lerp(m_movement_delta, position_delta, 0.2f);
            Direction dir = Direction_Helper.Calculate_Direction(m_movement_delta.X, m_movement_delta.Y, oldDirection, limitTo4Dir);
            pawn.Turn_Towards(dir);
        }

		// Onwards!
		pawn.Move_Towards(final_position);

		return at_target;
	}
}