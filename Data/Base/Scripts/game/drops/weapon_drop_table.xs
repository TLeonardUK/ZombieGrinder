// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.drops.drop_table;
using game.actors.pickups.pickup_list;

[
	Placeable(false), 
	Name("Weapon Drop Table"), 
	Description("Drop table for weapons.") 
]
public class Weapon_Drop_Table : Drop_Table
{
	Weapon_Drop_Table()
	{
		Drop_Probability = 0.05f; // 5%
		
		// Using a rough scale of:
		// 20=common
		// 10=uncommon
		//  5=rare
		//	2=ultra-rare

		Add(typeof(Ammo_Pickup),					20);
		Add(typeof(Explosive_Ammo_Pickup),			10);
		Add(typeof(Freeze_Ammo_Pickup),				10);
		Add(typeof(Incendiary_Ammo_Pickup),			10);
		
		Add(typeof(Buff_Grenade_Ammo_Pickup),		5);
		Add(typeof(Buff_Grenade_Damage_Pickup),		5);
		Add(typeof(Buff_Grenade_Health_Pickup),		5);
		Add(typeof(Buff_Grenade_Price_Pickup),		5);
		Add(typeof(Buff_Grenade_Reload_Pickup),		5);
		Add(typeof(Buff_Grenade_ROF_Pickup),		5);
		Add(typeof(Buff_Grenade_Speed_Pickup),		5);
		Add(typeof(Buff_Grenade_XP_Pickup),			5);
		        
		Add(typeof(Frag_Pickup),		        	5);
		Add(typeof(Molotov_Pickup),			        5);
		Add(typeof(Rocket_Launcher_2_Pickup),		5);
		Add(typeof(Rocket_Launcher_3_Pickup),		5);
		Add(typeof(Sword_Pickup),				    10);

		Add(typeof(Acid_Gun_Pickup),				5);
		Add(typeof(AK47_Pickup),					20);
		Add(typeof(Boltgun_Pickup),					10);
		Add(typeof(DB_Shotgun_Pickup),				20);
		Add(typeof(Flamethrower_Pickup),			5);
		Add(typeof(Flaregun_Pickup),				5);
		Add(typeof(Freeze_Ray_Pickup),				5);
		Add(typeof(Grenade_Launcher_Pickup),		10);
		Add(typeof(Laser_Cannon_Pickup),			2);
		Add(typeof(Love_Cannon_Pickup),				10);
		Add(typeof(Love_Thrower_Pickup),			10);
		Add(typeof(Assault_Rifle_Pickup),			20);
		Add(typeof(Magnum_Pickup),					10);
		Add(typeof(Rocket_Launcher_Pickup),			5);
		Add(typeof(RPC_Pickup),						5);
		Add(typeof(Scatter_Gun_Pickup),				20);
		Add(typeof(Shock_Rifle_Pickup),				5);
		Add(typeof(Shotgun_Pickup),					20);
		Add(typeof(Spaz_Pickup),					20);
		Add(typeof(Sticky_Launcher_Pickup),			5);
		Add(typeof(Uzi_Pickup),						20);
		Add(typeof(Winchester_Rifle_Pickup),		5);
		
		Add(typeof(Drink_Pickup),					5);
		Add(typeof(Medipack_Pickup),				2);
		Add(typeof(Nachos_Pickup),					5);
		Add(typeof(Pill_Pickup),					10);
		Add(typeof(Pill_Container_Pickup),			5);
		Add(typeof(Large_Armor_Pickup),				3);
		Add(typeof(Medium_Armor_Pickup),			5);
		Add(typeof(Small_Armor_Pickup),				8);
		
		Add(typeof(Turret_Pickup),					5);
		Add(typeof(Fire_Turret_Pickup),				5);
		//Add(typeof(Freeze_Turret_Pickup),			5);
		Add(typeof(Rocket_Turret_Pickup),			5);
		//Add(typeof(Tripmine_Pickup),				5);

		Add(typeof(Baseball_Bat_Pickup),			10);
		Add(typeof(Fight_Saber_Pickup),				10);
	}
}
