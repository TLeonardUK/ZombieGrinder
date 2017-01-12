// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.canvas;

public class CompassPip
{
	Vec2 Position;
	string Frame;
	int Except_Camera_Index;
    Vec4 Arrow_Color;
}

[
	Placeable(false), 
	Name("Compass"), 
	Description("Displays compass pips on participating huds.") 
]
public class Compass
{
	private const int MAX_PIPS = 32;

	private static CompassPip[] m_pips = null;
	private static int m_pip_count = 0;

	private static void Init()
	{
		m_pips = new CompassPip[MAX_PIPS];
		m_pip_count = 0;

		for (int i = 0; i < MAX_PIPS; i++)
		{
			m_pips[i] = new CompassPip();
		}
	}

	public static void Add_Pip(Vec3 position, string frame, int except_camera_index, Vec4 color)
	{
		if (m_pips == null)
		{
			Init();
		}

		if (m_pip_count >= MAX_PIPS)
		{
			return;
		}

		CompassPip pip = m_pips[m_pip_count];
		pip.Position = position.XY;
		pip.Frame = frame;
		pip.Except_Camera_Index = except_camera_index;
        pip.Arrow_Color = color;

		m_pip_count++;
	}

	public static void Draw(Canvas canvas, int camera_index, Vec2 icon_scale)
	{
		if (m_pip_count <= 0)
		{
			return;
		}

		Vec2 center = canvas.Bounds.Center();
		float radius = canvas.Bounds.Height * 0.35f;

		canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
		canvas.Scale = Vec2(canvas.UI_Scale, canvas.UI_Scale);

        Vec4 canvas_hide_bounds = canvas.Bounds;//Vec4(canvas.Bounds.Width * 0.10f, canvas.Bounds.Height * 0.10f, canvas.Bounds.Width - (canvas.Bounds.Width * 0.2f), canvas.Bounds.Height - (canvas.Bounds.Height * 0.2f));
		
		for (int i = 0; i < m_pip_count; i++)
		{
			CompassPip pip = m_pips[i];

			if (pip.Except_Camera_Index != camera_index)
			{
				Vec2 screen_space = canvas.World_BBox_To_Screen(Vec4(pip.Position.X, pip.Position.Y, 1.0f, 1.0f)).XY;
				Vec2 vec_to_target = screen_space - center;
				Vec2 unit = vec_to_target.Unit();
				float distance = vec_to_target.Length();
				float angle = Math.RadToDeg(unit.Direction());
				Vec2 draw_pos = center + (unit * radius);
                Vec4 screen_bounds = Vec4(screen_space.X, screen_space.Y, 1.0f, 1.0f);

				if (distance >= radius * 1.5f && !screen_bounds.Intersects(canvas_hide_bounds))
				{
					canvas.Rotation = angle + 90;
                    canvas.Color = pip.Arrow_Color;
					canvas.Draw_Frame("game_hud_player_compass_pip", Vec2(draw_pos.X, draw_pos.Y));
                    canvas.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);
                    
					if (pip.Frame != "")
					{
						draw_pos = draw_pos - (unit * 32.0f);
						canvas.Rotation = 0;
						canvas.Scale = Vec2(canvas.UI_Scale, canvas.UI_Scale) * icon_scale;
						canvas.Draw_Frame(pip.Frame, Vec2(draw_pos.X, draw_pos.Y));
						canvas.Scale = Vec2(canvas.UI_Scale, canvas.UI_Scale);
					}
				}
			}
		}
		
		canvas.Rotation = 0;
		canvas.Scale = Vec2(1.0f, 1.0f);
	}

	public static void Tick()
	{
		m_pip_count = 0;
	}
}