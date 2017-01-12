// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Strafe"), 
	Description("Shows an explanation of strafing.") 
]
public class Tutorial_Strafe : BaseTutorial
{	
	public serialized(1) int Strafes;
	public const int LearnStrafingThreshold = 3;

	Tutorial_Strafe()
	{
		Description					= "#tutorial_strafe";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 0;
	}
	
	public event void On_Pawn_Strafe(Event_Pawn_Strafe evt)
	{
		if (evt.Instigator != null && evt.Instigator.Owner != null && evt.Instigator.Owner.Is_Local)
		{
			Strafes++;
			if (Strafes >= LearnStrafingThreshold)
			{
				IsComplete = true;
			}
		}
	}

	public override bool Should_Show()
	{
		MinimumInterval = (60.0 * 1000.0) * (Strafes + 1);

		// TODO: Ideally do a dot product on user/enemy direction and only show this
		// if we are trying to move away from them facing in the opposite direction.
		Actor[] actors = Scene.Find_Actors_Distance_From_Cameras(typeof(Enemy), 0, 150.0f);
		if (actors.Length())
		{
			return true;
		}

		return false;
	}
}


