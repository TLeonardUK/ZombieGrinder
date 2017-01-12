// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

[
	Name("Tutorial Full Wallet"), 
	Description("Shows an explanation of full wallet.") 
]
public class Tutorial_Full_Wallet : BaseTutorial
{	
	bool bWasFull = false;
	bool bWasInit = false;

	Tutorial_Full_Wallet()
	{
		Description					= "#message_wallet_full";
		Title						= "#message_wallet_full_title";
		MinimumInterval				= 0;
		MinimumIntervalScaleFactor	= 1.0;
		MaximumTimesToShow			= 0;
	}

	public bool IsFull()
	{
		NetUser[] local_users = Network.Get_Local_Users();
		foreach (NetUser user in local_users)
		{
			if (user.Local_Profile.Coins >= user.Local_Profile.Wallet_Size)
			{
				return true;
			}
		}
		return false;
	}

	public override bool Should_Show()
	{
		if (!bWasInit)
		{
			bWasFull = IsFull();
			bWasInit = true;
		}

		bool isFull = IsFull();
		bool ret = isFull && !bWasFull;
		bWasFull = isFull;

		return ret;
	}
}
 

