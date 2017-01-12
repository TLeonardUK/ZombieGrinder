// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Walk"), 
	Description("Shows an explanation of walking.") 
]
public class Tutorial_Walk : BaseTutorial
{	
	Tutorial_Walk()
	{
		Description					= "#tutorial_movement";
		Title						= "#tutorial_title";
		MinimumInterval				= 60.0 * 1000.0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}

	public override bool Should_Show()
	{
		Actor[] local_pawns = Scene.Get_Local_Controlled_Pawns();
		if (local_pawns.Length())
		{
			return true;
		}
		return false;
	}
}
 

