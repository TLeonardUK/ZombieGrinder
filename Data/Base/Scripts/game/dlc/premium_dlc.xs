// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.dlc;

public class Premium_DLC : DLC
{
	Premium_DLC()
	{
		ID = 321500;
		Name = "Gold Account";
		Description = "Golden account. Gives access to lots of intresting bits and pieces!";
	}

	public override void On_Recieve(Profile profile)
	{
    /*
        // Start with full wallet!
        profile.Coins = profile.Wallet_Size;		

        // 2 pages by default. Premium ups us to 16 pages.
        profile.Inventory_Size = Math.Min(profile.Max_Inventory_Size, profile.Inventory_Size + (profile.Inventory_Slots_Per_Page * 16));
    */
	}
}