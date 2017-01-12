// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.canvas;

[
	Placeable(false), 
	Name("Minimap"), 
	Description("Dungeon minimap rendering.") 
]
public class Minimap
{
	private static bool bVisible = false;
	private static float Alpha = 0.0f;

	private static Vec2 Offset;

	private static int Downscale_Factor = 3;
	private static Vec2 Map_Bounds;
	private static Vec2 Minimap_Bounds;

	private static NetUser Instigator;

	private static float Move_Speed = 256.0f;

	public static void Show(NetUser user)
	{
		Instigator = user;

		if (!bVisible)
		{
			bVisible = true;
			Map_Bounds = Scene.Get_Bounds().ZW;
			Minimap_Bounds = Vec2(Map_Bounds.X / Downscale_Factor, Map_Bounds.Y / Downscale_Factor);
		
			Actor[] local_users = Scene.Get_Local_Controlled_Pawns();
			if (local_users.Length() > 0)
			{
				Actor center_actor = local_users[0];
				Offset = center_actor.Center.XY / Vec2(Downscale_Factor, Downscale_Factor);
			}
			else
			{		
				Offset = Vec2(Minimap_Bounds.X / 2, Minimap_Bounds.Y / 2);
			}
		}
	}
	
	public static void Hide()
	{
		bVisible = false;
	}

	public static bool Is_Open()
	{
		return bVisible;
	}

	public static void Draw(Canvas canvas)
	{
		if (Alpha < 0.01f)
		{
			return;
		}

		Vec4 canvas_bounds = canvas.Bounds;

		// Draw bounds of map.
		Vec2 center = Vec2(canvas_bounds.Width*0.5f, canvas_bounds.Height*0.5f);
		Vec4 area = Vec4(center.X - Offset.X, center.Y - Offset.Y, Minimap_Bounds.X, Minimap_Bounds.Y);

		// Draw each room in map.
		Actor[] rooms = Scene.Find_Actors(typeof(DC_Room_Marker));
		foreach (DC_Room_Marker room in rooms)
		{
			Vec4 room_bounds = room.World_Bounding_Box;
			Vec4 room_bounds_minimap = room_bounds / Vec4(Downscale_Factor, Downscale_Factor, Downscale_Factor, Downscale_Factor);

			Vec4 room_area = Vec4(area.X, area.Y, 0.0f, 0.0f) + room_bounds_minimap;
			
			// Draw color based on if we have visited the room or not.
			if (room.Room_Type == DC_Room_Type.End)
			{
				canvas.Color = Vec4(0.0f, 200.0f, 0.0f, 128.0f * Alpha);
			}
			else if (room.Has_Been_Visited)
			{
				canvas.Color = Vec4(0.0f, 0.0f, 200.0f, 128.0f * Alpha);
			}
			else
			{
				canvas.Color = Vec4(32.0f, 32.0f, 32.0f, 128.0f * Alpha);
				
			}
			canvas.Draw_Rect(room_area);

			canvas.Color = Vec4(200.0f, 200.0f, 200.0f, 255.0f * Alpha);
			canvas.Draw_Hollow_Rect(room_area, 2.0f);
		}

		// Draw human heads.
		Actor[] players = Scene.Find_Actors(typeof(Human_Player));
		foreach (Human_Player player in players)
		{
			if (player.Owner != null)
			{
				Item head_item = player.Owner.Profile.Get_Item_Slot(Item_Slot.Head);
				Item accessory_item = player.Owner.Profile.Get_Item_Slot(Item_Slot.Accessory);

				Direction direction = player.Current_Direction;

				Vec4 color_tint = Vec4(1.0f, 1.0f, 1.0f, Alpha);

				Vec4 draw_avatar_box = Vec4
				(
					(area.X + (player.Center.X / Downscale_Factor)) - 32,					
					(area.Y + (player.Center.Y / Downscale_Factor)) - 32,
					64,
					64
				);

				canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * Alpha);

				if (head_item != null)
				{
					canvas.Draw_Animation(head_item.Archetype.Icon_Animation + "_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);
					if (head_item.Archetype.Is_Tintable)
					{
						canvas.Color = head_item.Primary_Color * color_tint;
						canvas.Draw_Animation(head_item.Archetype.Icon_Animation + "_tint_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);			
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;
					}
				}
				if (accessory_item != null && accessory_item.Archetype.Is_Icon_Overlay)
				{
					canvas.Draw_Animation(accessory_item.Archetype.Icon_Animation + "_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);
					if (accessory_item.Archetype.Is_Tintable)
					{
						canvas.Color = head_item.Primary_Color * color_tint;
						canvas.Draw_Animation(accessory_item.Archetype.Icon_Animation + "_tint_" + Direction_Helper.Mnemonics[direction], draw_avatar_box);		
						canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f) * color_tint;	
					}
				}
			}
		}

		// Draw chests.
		Actor[] chests = Scene.Find_Actors(typeof(Dungeon_Chest));
		foreach (Dungeon_Chest chest in chests)
		{			
			Vec4 draw_box = Vec4
			(
				(area.X + (chest.Center.X / Downscale_Factor)) - 16,					
				(area.Y + (chest.Center.Y / Downscale_Factor)) - 16,
				32,
				32
			);
		
			canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * Alpha);

			if (chest.bSpecial)
			{
				if (!chest.bOpened)
				{
					canvas.Draw_Frame("actor_chest_special_0", draw_box);
				}
				else
				{
					canvas.Draw_Frame("actor_chest_special_5", draw_box);
				}
			}
			else
			{
				if (!chest.bOpened)
				{
					canvas.Draw_Frame("actor_chest_standard_0", draw_box);
				}
				else
				{
					canvas.Draw_Frame("actor_chest_standard_5", draw_box);
				}
			}
		}

		// Draw fade pattern over everything.
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * Alpha);
		canvas.Draw_Frame("screen_minimap_background", canvas.Bounds);

		// Text at top.
		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * Alpha);
		canvas.Draw_Markup_Text(Locale.Get("#minimap_title").Format(new object[] { <object>(Scene.Get_Map_Dungeon_Level() + 1) }), Vec4(0.0f, 0.0f, canvas_bounds.Width, canvas_bounds.Height * 0.15f), 24.0f, Text_Alignment.Center, Text_Alignment.Center);
		canvas.Draw_Markup_Text(Locale.Get("#minimap_controls"), Vec4(0.0f, canvas_bounds.Height - (canvas_bounds.Height * 0.15f), canvas_bounds.Width, canvas_bounds.Height * 0.15f), 24.0f, Text_Alignment.Center, Text_Alignment.Center);
	}
	
	public static void Tick()
	{
		if (bVisible)
		{
			Alpha = Math.Lerp(Alpha, 1.0f, 8.0f * Time.Get_Delta_Seconds());
		
			bool up_down 		= Input.Is_Down(Instigator, OutputBindings.Up);
			bool down_down 		= Input.Is_Down(Instigator, OutputBindings.Down);
			bool left_down 		= Input.Is_Down(Instigator, OutputBindings.Left);
			bool right_down 	= Input.Is_Down(Instigator, OutputBindings.Right);

			if (up_down)	Offset.Y -= Move_Speed * Time.Get_Delta_Seconds();
			if (down_down)	Offset.Y += Move_Speed * Time.Get_Delta_Seconds();
			if (left_down)	Offset.X -= Move_Speed * Time.Get_Delta_Seconds();
			if (right_down) Offset.X += Move_Speed * Time.Get_Delta_Seconds();
		}
		else
		{
			Alpha = Math.Lerp(Alpha, 0.0f, 8.0f * Time.Get_Delta_Seconds());
		}
	}
}