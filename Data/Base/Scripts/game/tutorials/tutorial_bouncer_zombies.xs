// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Bouncer Zombies"), 
	Description("Shows an explanation of bouncer zombies.") 
]
public class Tutorial_Bouncer_Zombies : BaseTutorial
{	
	Tutorial_Bouncer_Zombies()
	{
		Description					= "#tutorial_bouncer_zombies";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		if (Scene.Are_Actors_Visible(typeof(Bouncer_Zombie)))
		{
			return true;
		}
		return false;
	}
}
 

