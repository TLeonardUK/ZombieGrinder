// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Exploder Zombies"), 
	Description("Shows an explanation of exploder zombies.") 
]
public class Tutorial_Exploder_Zombies : BaseTutorial
{	
	Tutorial_Exploder_Zombies()
	{
		Description					= "#tutorial_explosive_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Exploder_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

