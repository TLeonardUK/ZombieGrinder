// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.weapon_ammo;

[
	Name("Explosive Ammo"), 
	Description("Bombs not bullets!") 
]
public class Weapon_Ammo_Explosive : Weapon_Ammo
{	
	Weapon_Ammo_Explosive()
	{
		Name = Locale.Get("#item_explosive_ammo_name");

		Effect_Modifier = "Explosive";
		HUD_Ammo_Icon = "game_hud_ammoicon_explosive";
	}
}
