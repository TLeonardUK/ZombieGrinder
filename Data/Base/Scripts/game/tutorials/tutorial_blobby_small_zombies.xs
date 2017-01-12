// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Blobby Small Zombies"), 
	Description("Shows an explanation of small blobby zombies.") 
]
public class Tutorial_Blobby_Small_Zombies : BaseTutorial
{	
	Tutorial_Blobby_Small_Zombies()
	{
		Description					= "#tutorial_small_blobby_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Small_Blobby_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

