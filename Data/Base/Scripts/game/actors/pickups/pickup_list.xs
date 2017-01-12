// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// ----------------------------------------------------------------------------- 
using game.actors.pickups.pickup;
using game.actors.pickups.local_pickup;
using game.actors.pickups.local_item_pickup;
using game.actors.pickups.weapon_pickup;
using game.actors.pickups.health_pickup;
using game.actors.pickups.inventory_pickup;
using game.actors.pickups.random_weapon;

using game.actors.pickups.ammo.ammo;
using game.actors.pickups.ammo.explosive_ammo;
using game.actors.pickups.ammo.freeze_ammo;
using game.actors.pickups.ammo.incendiary_ammo;

using game.actors.pickups.grenades.buff_grenade_ammo;
using game.actors.pickups.grenades.buff_grenade_damage;
using game.actors.pickups.grenades.buff_grenade_health;
using game.actors.pickups.grenades.buff_grenade_price;
using game.actors.pickups.grenades.buff_grenade_reload;
using game.actors.pickups.grenades.buff_grenade_rof;
using game.actors.pickups.grenades.buff_grenade_speed;
using game.actors.pickups.grenades.buff_grenade_xp;

using game.actors.pickups.grenades.frag;
using game.actors.pickups.grenades.molotov;
using game.actors.pickups.guns.rocket_launcher_2;
using game.actors.pickups.guns.rocket_launcher_3;
using game.actors.pickups.melee.sword;

using game.actors.pickups.guns.acid_gun;
using game.actors.pickups.guns.ak47;
using game.actors.pickups.guns.boltgun;
using game.actors.pickups.guns.double_barrel_shotgun;
using game.actors.pickups.guns.flamethrower;
using game.actors.pickups.guns.flaregun;
using game.actors.pickups.guns.freeze_ray;
using game.actors.pickups.guns.grenade_launcher;
using game.actors.pickups.guns.laser_cannon;
using game.actors.pickups.guns.love_cannon;
using game.actors.pickups.guns.love_thrower;
using game.actors.pickups.guns.assault_rifle;
using game.actors.pickups.guns.magnum;
using game.actors.pickups.guns.rocket_launcher;
using game.actors.pickups.guns.rpc;
using game.actors.pickups.guns.scatter_gun;
using game.actors.pickups.guns.shock_rifle;
using game.actors.pickups.guns.shotgun;
using game.actors.pickups.guns.spaz;
using game.actors.pickups.guns.sticky_launcher;
using game.actors.pickups.guns.uzi;
using game.actors.pickups.guns.winchester;
using game.actors.pickups.guns.pistol;
using game.actors.pickups.guns.barricade;

using game.actors.pickups.health.drink;
using game.actors.pickups.health.medipack;
using game.actors.pickups.health.nachos;
using game.actors.pickups.health.pill;
using game.actors.pickups.health.pill_container;
using game.actors.pickups.health.large_armor;
using game.actors.pickups.health.medium_armor;
using game.actors.pickups.health.small_armor;

using game.actors.pickups.machines.turret;
using game.actors.pickups.machines.fire_turret;
using game.actors.pickups.machines.freeze_turret;
using game.actors.pickups.machines.rocket_turret;
using game.actors.pickups.machines.tripmine;
using game.actors.pickups.machines.love_pylon;

using game.actors.pickups.melee.baseball_bat;
using game.actors.pickups.melee.fight_saber;

using game.actors.pickups.misc.bag;
using game.actors.pickups.misc.chest_key;
using game.actors.pickups.misc.key;
using game.actors.pickups.misc.keycard;
using game.actors.pickups.misc.map;
using game.actors.pickups.misc.random;
using game.actors.pickups.misc.candy_corn;
using game.actors.pickups.misc.coin_bag;
using game.actors.pickups.misc.manual_weapon_drop;

using game.actors.pickups.gems.ammo.ammo_1;
using game.actors.pickups.gems.ammo.ammo_2;
using game.actors.pickups.gems.ammo.ammo_3;
using game.actors.pickups.gems.ammo.ammo_4;
using game.actors.pickups.gems.ammo.ammo_5;

using game.actors.pickups.gems.damage.damage_1;
using game.actors.pickups.gems.damage.damage_2;
using game.actors.pickups.gems.damage.damage_3;
using game.actors.pickups.gems.damage.damage_4;
using game.actors.pickups.gems.damage.damage_5;

using game.actors.pickups.gems.health.health_1;
using game.actors.pickups.gems.health.health_2;
using game.actors.pickups.gems.health.health_3;
using game.actors.pickups.gems.health.health_4;
using game.actors.pickups.gems.health.health_5;

using game.actors.pickups.gems.xp.xp_1;
using game.actors.pickups.gems.xp.xp_2;
using game.actors.pickups.gems.xp.xp_3;
using game.actors.pickups.gems.xp.xp_4;
using game.actors.pickups.gems.xp.xp_5;

using game.actors.pickups.gems.speed.speed_1;
using game.actors.pickups.gems.speed.speed_2;
using game.actors.pickups.gems.speed.speed_3;
using game.actors.pickups.gems.speed.speed_4;
using game.actors.pickups.gems.speed.speed_5;

using game.actors.pickups.gems.rof.rof_1;
using game.actors.pickups.gems.rof.rof_2;
using game.actors.pickups.gems.rof.rof_3;
using game.actors.pickups.gems.rof.rof_4;
using game.actors.pickups.gems.rof.rof_5;

using game.actors.pickups.gems.reload.reload_1;
using game.actors.pickups.gems.reload.reload_2;
using game.actors.pickups.gems.reload.reload_3;
using game.actors.pickups.gems.reload.reload_4;
using game.actors.pickups.gems.reload.reload_5;

using game.actors.pickups.gems.price.price_1;
using game.actors.pickups.gems.price.price_2;
using game.actors.pickups.gems.price.price_3;
using game.actors.pickups.gems.price.price_4;
using game.actors.pickups.gems.price.price_5;