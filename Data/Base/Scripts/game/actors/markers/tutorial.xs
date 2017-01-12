// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Tutorial"), 
	Description("Shows a tutorial if triggered.") 
]
public class Tutorial : Actor
{
	// Gets/sets the sprite animation.
	[
		Name("Tutorial Text"),
		Description("Text shown in the tutorial box.")
	]
	public serialized(1) string Tutorial_Text = "";

	Tutorial()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}
	
	public override void On_Trigger()
	{
		// Show tutorial!
		Game_HUD game_hud = <Game_HUD>Scene.Active_Game_Mode.Get_HUD();
		game_hud.Show_Message(Locale.Get(Tutorial_Text), Locale.Get("#tutorial_title"), "tutorial_help");
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_tutorial_default_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Visible = false;
			Editor_Visible = true;
		}
	}
}