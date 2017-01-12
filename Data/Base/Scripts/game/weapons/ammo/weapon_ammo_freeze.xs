// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.weapon_ammo;

[
	Name("Freeze Ammo"), 
	Description("Freezes and slows down enemies for a period of time.") 
]
public class Weapon_Ammo_Freeze : Weapon_Ammo
{	
	Weapon_Ammo_Freeze()
	{
		Name = Locale.Get("#item_freeze_ammo_name");

		Effect_Modifier = "Freeze";
		HUD_Ammo_Icon = "game_hud_ammoicon_freeze";
	}
}
