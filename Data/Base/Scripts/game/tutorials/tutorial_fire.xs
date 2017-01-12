// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Fire"), 
	Description("Shows an explanation of fireing.") 
]
public class Tutorial_Fire : BaseTutorial
{	
	private float AliveTimer = 0.0f;

	Tutorial_Fire()
	{
		Description					= "#tutorial_shoot";
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
			AliveTimer += Time.Get_Frame_Time();
			if (AliveTimer > 20.0f)
			{
				return true;
			}
		}
		return false;
	}
}
 

