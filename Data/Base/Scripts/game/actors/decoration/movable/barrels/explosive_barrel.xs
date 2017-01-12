// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.decoration.movable.movable;

[
	Placeable(true), 
	Name("Explosive Barrel"), 
	Description("Objects that can be picked up and moved.") 
]
public class Explosive_Barrel : Movable 
{
	override event void On_Editor_Property_Change()
	{
		Vec2 frame_size = Canvas.Get_Frame_Size(sprite.Frame);
		Bounding_Box = Vec4(0, 0.0f, frame_size.X, frame_size.Y);
		Collision_Box = Vec4(0, frame_size.Y * 0.5f, frame_size.X, frame_size.Y * 0.5f);

		base.On_Editor_Property_Change();
	}

	Explosive_Barrel()
	{
		Pickup_Sound		= "sfx_objects_destroyable_move_start";
		Pickup_Effect		= "";
		Drop_Sound			= "sfx_objects_destroyable_move_end";
		Drop_Effect			= "dust_puff";
		Destroy_Sound		= "";
		Destroy_Effect		= "explosive_destroy";

		Bounding_Box	= Vec4(0, 0, 16, 32);
		Collision_Box 	= Vec4(0, 16, 16, 16);

		sprite.Frame		= "decal_barrel_all_colors_9";
	}
}