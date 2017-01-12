// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.weapon_ammo;

[
	Name("Incendiary  Ammo"), 
	Description("Sets enemies on fire!") 
]
public class Weapon_Ammo_Incendiary : Weapon_Ammo
{	
	Weapon_Ammo_Incendiary()
	{
		Name = Locale.Get("#item_incendiary_ammo_name");

		Effect_Modifier = "Incendiary";
		HUD_Ammo_Icon = "game_hud_ammoicon_incendiary";
	}
}
