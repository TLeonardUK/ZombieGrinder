// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Run"), 
	Description("Shows an explanation of running.") 
]
public class Tutorial_Run : BaseTutorial
{	
	public serialized(1) int Sprints;
	public const int LearnSprintsThreshold = 3;

	Tutorial_Run()
	{
		Description					= "#tutorial_sprint";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 0;
	}
	
	public event void On_Pawn_Sprint(Event_Pawn_Sprint evt)
	{
		if (evt.Instigator != null && evt.Instigator.Owner != null && evt.Instigator.Owner.Is_Local)
		{
			Sprints++;
			if (Sprints >= LearnSprintsThreshold)
			{
				IsComplete = true;
			}
		}
	}

	public override bool Should_Show()
	{
		MinimumInterval = (60.0 * 1000.0) * (Sprints + 1);

		if (Scene.Find_Actors_Distance_From_Cameras(typeof(Enemy), 0, 64.0f).Length())
		{
			return true;
		}

		return false;
	}
}


