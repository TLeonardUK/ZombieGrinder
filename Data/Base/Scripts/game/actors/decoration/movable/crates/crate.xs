// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.decoration.movable.movable;

[
	Placeable(true), 
	Name("Crate"), 
	Description("Objects that can be picked up and moved.") 
]
public class Crate : Movable 
{
    private bool bFixedEffects = false;

	override event void On_Editor_Property_Change()
	{
		Vec2 frame_size = Canvas.Get_Frame_Size(sprite.Frame);
		Bounding_Box = Vec4(0, 0.0f, frame_size.X, frame_size.Y);
		Collision_Box = Vec4(0, frame_size.Y * 0.5f, frame_size.X, frame_size.Y * 0.5f);

		base.On_Editor_Property_Change();
	}

	override void On_Update_State()
	{
        if (!bFixedEffects)
        {
            if (sprite.Frame.Contains("decal_crate_crate_1_") ||
                sprite.Frame.Contains("decal_crate_crate_2_") ||
                sprite.Frame.Contains("decal_crate_crate_3_"))
            {
        		Destroy_Effect = "cardboard_destroy";
                bFixedEffects = true;
            }
        }
	}

	Crate()
	{
		Pickup_Sound		= "sfx_objects_destroyable_move_start";
		Pickup_Effect		= "";
		Drop_Sound			= "sfx_objects_destroyable_move_end";
		Drop_Effect			= "dust_puff";
		Destroy_Sound		= "sfx_objects_destroyable_poof";
		Destroy_Effect		= "wood_destroy";

		Bounding_Box		= Vec4(0, 0, 32, 32);
		Collision_Box 		= Vec4(0, 16, 32, 16);

		sprite.Frame		= "decal_crate_crate_1_0";
	}
}