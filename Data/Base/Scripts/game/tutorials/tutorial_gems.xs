// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Gems"), 
	Description("Shows an explanation of gems.") 
]
public class Tutorial_Gems : BaseTutorial
{	
	private int GemsSpawned = 0;

	Tutorial_Gems()
	{
		Description					= "#tutorial_gems";
		Title						= "#tutorial_title";
		MinimumInterval				= (60.0 * 1000.0) ;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}
	
	public event void On_Item_Dropped(Event_Item_Dropped evt)
	{
		if (evt.IsGem)
		{
			GemsSpawned++;
		}
	}

	public override bool Should_Show()
	{
		return (GemsSpawned > 0);
	}
}
 

