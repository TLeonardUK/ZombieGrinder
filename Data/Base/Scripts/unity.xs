// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
 
// Doing a massive unity build is actually faster at the moment than doing incremental
// builds. Christ that compiler needs some optimization ...

using system.achievement;
using System.challenge;
using system.tutorial;
using system.leaderboard;

using system.statistic;     


// Fun little modifiers for each day.
using game.challenges.daily.challenge_1hp_survival;
using game.challenges.daily.challenge_hp_bullets;
using game.challenges.daily.challenge_hp_movement;
using game.challenges.daily.challenge_max_wave;
using game.challenges.daily.challenge_max_score;
using game.challenges.daily.challenge_max_multiplier;
using game.challenges.daily.challenge_weapon_switch;
using game.challenges.daily.challenge_reversed_controls;
using game.challenges.daily.challenge_play_modded_map;

// Longer challenges that promote different play styles - healing, reviving, etc.
using game.challenges.weekly.challenge_revive_players;
using game.challenges.weekly.challenge_heal_players;
using game.challenges.weekly.challenge_pvp_kills;
using game.challenges.weekly.challenge_pvp_top_place;
using game.challenges.weekly.challenge_weapon_kills;
using game.challenges.weekly.challenge_halloween;

// Excessively long challenges that give achievements for getting stupidly large values on
// things - bullets fired, zombies killed, boxes destroyed, etc.
using game.challenges.monthly.challenge_zombie_kills;
using game.challenges.monthly.challenge_bullets_fired;
using game.challenges.monthly.challenge_boxes_destroyed;



using game.drops.challenges.challenge_coin_drop_table;
using game.drops.challenges.challenge_item_drop_table;
using game.drops.challenges.challenge_xp_drop_table;
using game.drops.challenges.challenge_rare_coin_drop_table;
using game.drops.challenges.challenge_rare_item_drop_table;
using game.drops.challenges.challenge_rare_xp_drop_table;
using game.drops.items.small_gem_grab_bag_drop_table;
using game.drops.items.medium_gem_grab_bag_drop_table;
using game.drops.items.large_gem_grab_bag_drop_table;

using game.achievements.achievement_bronze;
using game.achievements.achievement_canttouchthis;
using game.achievements.achievement_cheater;
using game.achievements.achievement_cremationship;
using game.achievements.achievement_developerdeath;
using game.achievements.achievement_dick;
using game.achievements.achievement_eagleeye;
using game.achievements.achievement_failure;
using game.achievements.achievement_frigid;
using game.achievements.achievement_gold;
using game.achievements.achievement_headshothoncho;
using game.achievements.achievement_healinglevel1;
using game.achievements.achievement_healinglevel2;
using game.achievements.achievement_healinglevel3;
using game.achievements.achievement_healinglevel4;
using game.achievements.achievement_horder;
using game.achievements.achievement_iceandfire;
using game.achievements.achievement_ihateboxes;
using game.achievements.achievement_itshotinhere;
using game.achievements.achievement_mammy;
using game.achievements.achievement_marathonman;
using game.achievements.achievement_multiplier10;
using game.achievements.achievement_multiplier15;
using game.achievements.achievement_multiplier5;
using game.achievements.achievement_n00b;
using game.achievements.achievement_owned;
using game.achievements.achievement_penetrator;
using game.achievements.achievement_winner;
using game.achievements.achievement_redmist;
using game.achievements.achievement_saviour;
using game.achievements.achievement_silver;
using game.achievements.achievement_theghostisclear;
using game.achievements.achievement_trickshot;
using game.achievements.achievement_youdidntcrash;
using game.achievements.achievement_zombiegrinder;
using game.achievements.achievement_zombiekiller;
using game.achievements.achievement_zombiemaster;
using game.achievements.achievement_zombinator;
using game.achievements.achievement_dungeonlevel1;
using game.achievements.achievement_dungeonlevel2;
using game.achievements.achievement_dungeonlevel3;
using game.achievements.achievement_workshop;

using game.tutorials.tutorial_alt_fire;
using game.tutorials.tutorial_blobby_large_zombies;
using game.tutorials.tutorial_blobby_small_zombies;
using game.tutorials.tutorial_bouncer_zombies;
using game.tutorials.tutorial_chaser_zombies;
using game.tutorials.tutorial_coins;
using game.tutorials.tutorial_cycle_weapons;
using game.tutorials.tutorial_exploder_zombies;
using game.tutorials.tutorial_fire;
using game.tutorials.tutorial_floater_zombies;
using game.tutorials.tutorial_fodder_zombies;
using game.tutorials.tutorial_gems;
using game.tutorials.tutorial_interact;
using game.tutorials.tutorial_reload;
using game.tutorials.tutorial_run;
using game.tutorials.tutorial_strafe;
using game.tutorials.tutorial_full_inventory;
using game.tutorials.tutorial_full_wallet;

using game.tutorials.tutorial_walk;
using game.dlc.bisoncapgold_dlc;
using game.dlc.bsod_dlc;
using game.dlc.christmas_dlc;
using game.dlc.desura_dlc;
using game.dlc.halo_dlc;
using game.dlc.premium_dlc;

using game.statistics.statistic_aggregated_boxes_destroyed;
using game.statistics.statistic_aggregated_characters_created;
using game.statistics.statistic_aggregated_gold_gathered;
using game.statistics.statistic_aggregated_human_deaths;
using game.statistics.statistic_aggregated_human_revives;
using game.statistics.statistic_aggregated_level_ups;
using game.statistics.statistic_aggregated_maps_played;
using game.statistics.statistic_aggregated_pixels_moved;
using game.statistics.statistic_aggregated_pvp_kills;
using game.statistics.statistic_aggregated_zombies_frozen;
using game.statistics.statistic_aggregated_zombies_ignited;
using game.statistics.statistic_aggregated_zombies_killed;
using game.statistics.statistic_aggregated_zombies_shocked;
using game.statistics.statistic_aggregated_play_time;
using game.statistics.statistic_boxes_destroyed;
using game.statistics.statistic_damage_dealt;
using game.statistics.statistic_gold_gathered;
using game.statistics.statistic_healing_dealt;
using game.statistics.statistic_highest_multiplier;
using game.statistics.statistic_highest_rank;
using game.statistics.statistic_highest_score;
using game.statistics.statistic_highest_wave;
using game.statistics.statistic_human_deaths;
using game.statistics.statistic_human_revives;
using game.statistics.statistic_long_distance_ignite;
using game.statistics.statistic_maps_played;
using game.statistics.statistic_pixels_moved;
using game.statistics.statistic_pvp_kills;
using game.statistics.statistic_rebound_shots;
using game.statistics.statistic_zombies_frozen;
using game.statistics.statistic_zombies_frozen_ignited;
using game.statistics.statistic_zombies_ignited;
using game.statistics.statistic_zombies_killed;
using game.statistics.statistic_zombies_shocked;
using game.statistics.statistic_play_time;
using game.statistics.statistic_dungeon_level;

using game.profile.profile;
    
using game.effects.deus_ex_post_process_fx;
using game.effects.gameboy_post_process_fx;
using game.effects.chromatic_arboration_post_process_fx;
using game.effects.health_warning_post_process_fx;
using game.effects.enter_the_light_post_process_fx;
using game.effects.fire_darken_post_process_fx;
using game.effects.hq2x_post_process_fx;



		
		
		

using game.actors.audio.triggered_audio;
using game.actors.audio.triggered_bgm;
    
using game.actors.decoration.enemy_barricade;
using game.actors.decoration.decal;
using game.actors.decoration.blinking_decal;
using game.actors.decoration.animated_decal;
using game.actors.decoration.button;
using game.actors.decoration.ammo_crate;
using game.actors.decoration.door;
using game.actors.decoration.intro_door;
using game.actors.decoration.food_vending_machine;
using game.actors.decoration.soda_vending_machine;
using system.actors.effect;
    
using game.actors.decoration.movable.movable;
using game.actors.decoration.movable.movable_list;

using game.actors.pickups.pickup_list;

using game.actors.player.human_player;
using game.actors.player.player_spawn;
using game.actors.player.player_spawn_cutscene_point;

using game.actors.survivor.survivor;
using game.actors.cucco.cucco;

using game.actors.enemies.fodder_zombie;
using game.actors.enemies.chaser_zombie;
using game.actors.enemies.bouncer_zombie;
using game.actors.enemies.exploder_zombie;
using game.actors.enemies.floater_zombie;

using game.actors.enemies.enemy_manual_spawner;

using game.actors.enemies.bosses.boss;
using game.actors.enemies.bosses.whipper.boss_whipper;
using game.actors.enemies.bosses.whipper.boss_whipper_emerge_marker;
using game.actors.enemies.bosses.whipper.boss_whipper_trigger;
using game.actors.enemies.bosses.whipper.boss_whipper_door;
using game.actors.enemies.bosses.whipper.boss_whipper_tentacle;


using game.actors.controllers.pawn_controller;
using game.actors.controllers.bosses.ai_boss_whipper_controller;
    
using game.actors.controllers.ai_fodder_zombie_controller;
using game.actors.controllers.ai_bouncer_zombie_controller;
using game.actors.controllers.ai_chaser_zombie_controller;
using game.actors.controllers.ai_exploder_zombie_controller;
using game.actors.controllers.ai_small_blobby_zombie_controller;
using game.actors.controllers.ai_large_blobby_zombie_controller;
using game.actors.controllers.ai_floater_zombie_controller;

using game.actors.controllers.player_generic_zombie_controller;
using game.actors.controllers.player_chaser_zombie_controller;
using game.actors.controllers.player_exploder_zombie_controller;
using game.actors.controllers.player_large_blobby_zombie_controller;
using game.actors.controllers.player_small_blobby_zombie_controller;
using game.actors.controllers.player_floater_zombie_controller;
using game.actors.controllers.player_bouncer_zombie_controller;

using game.actors.controllers.ai_human_controller;
using game.actors.controllers.player_human_controller;


using game.actors.markers.move_zone;
using game.actors.markers.trigger;
using game.actors.markers.interactive_trigger;
using game.actors.markers.interactive_item_trigger;
using game.actors.markers.capture_zone;
using game.actors.markers.actor_spawner;
using game.actors.markers.path_pylon;
using game.actors.markers.bounding_area;
using game.actors.markers.spawn_control_trigger;
using game.actors.markers.tutorial;
using game.actors.markers.actor_chooser;
using game.actors.markers.ai_breadcrumb;
using game.actors.markers.preview_image_center;

using game.actors.dungeon.traps.mounted_gun;
using game.actors.dungeon.dc_junction_marker;
using game.actors.dungeon.dc_room_marker;
using game.actors.dungeon.dc_template_marker;
using game.actors.dungeon.dungeon_stairs;
using game.actors.dungeon.dungeon_blackout;
using game.actors.dungeon.dungeon_door;
using game.actors.dungeon.dungeon_arena;
using game.actors.dungeon.dungeon_chest;
using game.actors.dungeon.dungeon_destroyable_door;

using game.modes.wave;
using game.modes.objective;
using game.modes.deathmatch;
using game.modes.team_deathmatch;
using game.modes.wave_gun_game;
using game.modes.gun_game;
using game.modes.rack_em_up;
using game.modes.capture_the_bag;
using game.modes.dungeon;

using game.weapons.weapon_list;

using game.items.item_upgrade_tree;
using game.items.item_list;

using game.skills.skill_list;

using game.drops.gem_drop_table;
using game.drops.weapon_drop_table;
using game.drops.chest_drop_table;