// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Chaser Zombies"), 
	Description("Shows an explanation of chaser zombies.") 
]
public class Tutorial_Chaser_Zombies : BaseTutorial
{	
	Tutorial_Chaser_Zombies()
	{
		Description					= "#tutorial_chaser_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Chaser_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

