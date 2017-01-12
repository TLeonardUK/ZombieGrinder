// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Floater Zombies"), 
	Description("Shows an explanation of floater zombies.") 
]
public class Tutorial_Floater_Zombies : BaseTutorial
{	
	Tutorial_Floater_Zombies()
	{
		Description					= "#tutorial_floater_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Floater_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

