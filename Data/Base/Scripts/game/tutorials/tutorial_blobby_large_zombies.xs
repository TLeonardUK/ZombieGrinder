// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Blobby Large Zombies"), 
	Description("Shows an explanation of large blobby zombies.") 
]
public class Tutorial_Blobby_Large_Zombies : BaseTutorial
{	
	Tutorial_Blobby_Large_Zombies()
	{
		Description					= "#tutorial_large_blobby_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Large_Blobby_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

