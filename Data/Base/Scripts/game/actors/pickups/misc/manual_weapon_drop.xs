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
public class Manual_Weapon_Drop_Pickup : Pickup
{	
	public serialized(1) string Pickup_Sprite	= "actor_pickups_coin_bag_0";
	public serialized(1) string Pickup_Name		= Locale.Get("#item_manual_drop_name");
	public serialized(1) string Pickup_Sound	= "sfx_objects_coin_bag_pickup";

    public Type             Weapon_Type = null;
    public Type             Ammo_Type = null;
    
	public Vec4             Primary_Color;
    public Item_Archetype   Upgrade_Item_Archetype;
    public int[]            Upgrade_Tree_Ids;
	public int              Clip_Size;
	public int              Reserve_Size;
	public int              Reserve_Ammo;
    public int              Clip_Ammo;	

    Manual_Weapon_Drop_Pickup()
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
                player.Give_Weapon_Extended(
                    Weapon_Type,
                    Ammo_Type,
                    Primary_Color,
                    Upgrade_Item_Archetype,
                    Upgrade_Tree_Ids,
                    Clip_Size,
                    Reserve_Size,
                    Clip_Ammo,
                    Reserve_Ammo
                );

                if (Pickup_Sound != "")
                {
                    Audio.Play2D(Pickup_Sound);
                }
            }
        }
	}
}