// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Coins"), 
	Description("Shows an explanation of coins.") 
]
public class Tutorial_Coins : BaseTutorial
{	
	private int CoinsSpawned = 0;

	Tutorial_Coins()
	{
		Description					= "#tutorial_coins";
		Title						= "#tutorial_title";
		MinimumInterval				= (60.0 * 1000.0);
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 1;
	}
	
	public event void On_Item_Dropped(Event_Item_Dropped evt)
	{
		if (evt.IsCoin)
		{
			CoinsSpawned++;
		}
	}

	public override bool Should_Show()
	{
		return (CoinsSpawned > 0);
	}
}
 

