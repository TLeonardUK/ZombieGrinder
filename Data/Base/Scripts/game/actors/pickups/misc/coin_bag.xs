// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.pickup;
using game.actors.player.human_player;

[
	Placeable(true), 
	Name("Coin Bag Pickup"), 
	Description("Base class for a coin bag a player has dropped.") 
]
public class Coin_Bag : Pickup
{	
	protected string Pickup_Sprite		= "actor_pickups_coin_bag_0";
	protected string Pickup_Name		= Locale.Get("#item_coin_bag_name");
	protected string Pickup_Sound		= "sfx_objects_coin_bag_pickup";

    public int Coin_Quantity = 0;
    
    Coin_Bag()
    {
        Use_On_Remote = true;
    }

	public override string Get_Sprite()
	{
		return Pickup_Sprite;
	}

	public override bool Can_Interact(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
            return true;
		}
		return false;
	}

	public override string Get_Buy_Prompt(Pawn other)
	{
		return Locale.Get("#buy_generic_prompt").Format(new object[] { Pickup_Name });
	}

	public override void Use(Pawn other)
	{
        if (Network.Is_Server())
        {
            Human_Player player = < Human_Player > other;
            if (player != null)
            {
                player.Owner.Pickup_Coins(Coin_Quantity);
                Coin_Quantity = 0;

                if (Pickup_Sound != "")
                {
                    Audio.Play2D(Pickup_Sound);
                }
            }
        }
	}
}