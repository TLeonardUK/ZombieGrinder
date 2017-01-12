// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
[
	Name("Tutorial"), 
	Description("Base class for all in-game tutorials.") 
]
public class BaseTutorial
{	
	// Title/Description to show when triggered.
	public string Description;
	public string Title;

	// Maximum frequency to show this tutorial.
	public float MinimumInterval;
	public float MinimumIntervalScaleFactor; // Minimum interval is scaled up after each tutorial is shown.
	public int   MaximumTimesToShow;

	public float TimeSinceLastShow;
	public serialized(1) int TimesShown;

	// If set to true, this tutorial will be considered completed and will never be ticked again.
	public serialized(1) bool IsComplete;

	// Called by base class to determine if this tutorial should be shown or not.
	public abstract bool Should_Show();

	// Tick and determine if we should show the tutorial!
	default struct Normal
	{
		event void On_Enter()
		{
			TimeSinceLastShow = MinimumInterval;
		}
		event void On_Tick()
		{
			TimeSinceLastShow += Time.Get_Frame_Time();
			if (TimeSinceLastShow >= MinimumInterval)
			{
				if (Should_Show())
				{				
					Game_HUD game_hud = <Game_HUD>Scene.Active_Game_Mode.Get_HUD();
					if (!game_hud.Is_Message_Visible())
					{
						game_hud.Show_Message(Locale.Get(Description), Locale.Get(Title), "tutorial_help");
						
						TimeSinceLastShow = 0.0f;
						TimesShown++;
						
						if (TimesShown >= MaximumTimesToShow && MaximumTimesToShow != 0)
						{
							IsComplete = true;
						}
					}
				}
			}
		}
	}
}
 

