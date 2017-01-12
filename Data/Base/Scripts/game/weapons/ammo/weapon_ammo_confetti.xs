// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.weapon_ammo;

[
	Name("Confetti Ammo"), 
	Description("So sparkly! Fires burts of confetti instead of bullets.") 
]
public class Weapon_Ammo_Confetti : Weapon_Ammo
{
	Weapon_Ammo_Confetti()
	{
		Name = Locale.Get("#item_confetti_ammo_name");

		Effect_Modifier = "Confetti";
		HUD_Ammo_Icon = "game_hud_ammoicon_confetti";
	}
}
