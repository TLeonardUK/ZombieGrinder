// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Fodder Zombies"), 
	Description("Shows an explanation of fodder zombies.") 
]
public class Tutorial_Fodder_Zombies : BaseTutorial
{	
	Tutorial_Fodder_Zombies()
	{
		Description					= "#tutorial_fodder_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Fodder_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

