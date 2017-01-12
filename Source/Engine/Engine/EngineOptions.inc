// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef OPTION
#error "OPTION must be defined when including this file!"
#endif

// These define all the different configuration options used by the engine.

// All options are accessed in the engine using an f_ prefix and using the deref operator to get/set it's value. eg.
//	int some_blah = *f_render_frame_rate;
//	*f_render_frame_rate = 100;

//	   Type				Name								Flags				Default Value						Description				

// Tick settings.
OPTION(IntOption,		render_frame_rate,					OptionFlags::NONE,	60,									"Internal rendering frame rate");

// Display settings.
OPTION(IntOption,		render_display_width,				OptionFlags::Saved, 1280,								"Width of display window.")
OPTION(IntOption,		render_display_height,				OptionFlags::Saved, 720,								"Height of display window.")
#ifdef MASTER_BUILD
OPTION(IntOption,		render_display_mode,				OptionFlags::Saved, 0,									"Display window mode; fullscreen/fullscreen-windowed/windowed.")
#else
OPTION(IntOption,		render_display_mode,				OptionFlags::Saved, 2,									"Display window mode; fullscreen/fullscreen-windowed/windowed.")
#endif
OPTION(IntOption,		render_display_hertz,				OptionFlags::Saved, 60,									"Refresh rate of display window.")
OPTION(StringOption,	render_display_title,				OptionFlags::NONE,  "Zombie Grinder",					"Title shown in display window.")

OPTION(IntOption,		render_game_width,					OptionFlags::NONE, 320,									"The game's internal display width. Height is calculated based on the aspect-ratio of the display size.")
OPTION(IntOption,		render_game_height,					OptionFlags::NONE, 240,									"The game's internal display height. Width is calculated based on the aspect-ratio of the display size.")
OPTION(BoolOption,		render_game_aspect_shrink_height,	OptionFlags::NONE, false,								"If aspect ratio is larger than internal game resolution, then shrink height to match, otherwise shrink width to match.")

OPTION(BoolOption,		render_legacy,						OptionFlags::Saved, false,								"Uses legacy intermediate rendering. A lot of effects will be non-functional, but should get the game running on poor gpu's.")
OPTION(BoolOption,		render_legacy_was_forced,			OptionFlags::NONE,  false,								"Internal flag that determines if legacy-mode was forced on due to lack of support. If so, the user will not get the option to turn it off.")
OPTION(BoolOption,		render_legacy_was_informed,			OptionFlags::Saved, false,								"Internal flag that determines if user was notified of legacy rendering mode.")

OPTION(BoolOption,		render_logging,						OptionFlags::NONE, false,								"Writes a lot of render debugging information to the log. Very slow.")

// GUI settings.
#ifdef DEDICATED_SERVER_BUILD
OPTION(BoolOption,		nogui,								OptionFlags::NONE,  true,								"Runs the game without any form of graphical interface. COMMAND LINE POWAH!")
#else
OPTION(BoolOption,		nogui,								OptionFlags::NONE,  false,								"Runs the game without any form of graphical interface. COMMAND LINE POWAH!")
#endif

// Render pipeline settings.
OPTION(StringOption,	render_pipeline_file,				OptionFlags::NONE,  "Data/Config/Render_Pipeline.xml",	"Path to rendering pipeline definition file.")

// Audio settings.
OPTION(IntOption,		audio_max_channels,					OptionFlags::NONE,  256,								"Maximum number of audio channels.")
OPTION(FloatOption,		audio_bgm_volume,					OptionFlags::Saved, 1.0f,								"Volume of background music.")
OPTION(FloatOption,		audio_sfx_volume,					OptionFlags::Saved, 1.0f,								"Volume of sound effects.")
OPTION(FloatOption,		audio_voice_send_volume,			OptionFlags::Saved, 1.0f,								"Volume of audio sent from microphone.")
OPTION(FloatOption,		audio_voice_recv_volume,			OptionFlags::Saved, 0.5f,								"Volume of voice audio recieved from other users.")
OPTION(BoolOption,		audio_voice_enabled,				OptionFlags::Saved, true,								"Toggles enabled state of voice chat.")
OPTION(BoolOption,		audio_push_to_talk_enabled,			OptionFlags::Saved, true,								"Toggles push to talk enabled state.")

// Language settings.
OPTION(StringOption,	language_default,					OptionFlags::Saved, "EN-GB",							"Default language name.")

// Task settings
OPTION(IntOption,		tasks_max_workers,					OptionFlags::NONE,  7,									"Maximum number of task workers in pool.")
OPTION(IntOption,		tasks_max_tasks,					OptionFlags::NONE,  2048,								"Maximum number of tasks in queue.")

// Steam settings
OPTION(IntOption,		steam_appid,						OptionFlags::NONE,  STEAM_APP_ID,						"AppID on steamworks platform.")
OPTION(IntOption,		steam_notification_corner,			OptionFlags::Saved, 3,									"Corner that notifications are shown when running steamworks overlay.")

// Data setting
OPTION(StringOption,	data_directory,						OptionFlags::NONE,  "../Data/",							"Directory that data is stored in.")

// Matchmaking
OPTION(IntOption,		matching_max_ping,					OptionFlags::Saved, 200,								"Maximum ping of servers looked for during matchmaking.")
OPTION(IntOption,		matching_search_timeout,			OptionFlags::Saved, 5000,								"Timeout in milliseconds when searching for lobbies or servers.")

// Server settings.
#ifdef DEDICATED_SERVER_BUILD
OPTION(BoolOption,		server,								OptionFlags::NONE,	true,								"If set to true the game runs as a server.");
#else
OPTION(BoolOption,		server,								OptionFlags::NONE,	false,								"If set to true the game runs as a server.");
#endif
OPTION(StringOption,	server_name,						OptionFlags::NONE,	"ZombieGrinder Server",				"Name that this game advertises itself with when running as a server.");
OPTION(BoolOption,		server_secure,						OptionFlags::NONE,	true,								"If set to true then cheat-protection is enabled on this server.");
OPTION(StringOption,	server_ip,							OptionFlags::NONE,	"",									"Set to IP of interface to listen on. If not set, will bind to all addresses.");
OPTION(IntOption,		server_port,						OptionFlags::NONE,	57897,								"Set to port number to listen on.");
OPTION(IntOption,		server_auth_port,					OptionFlags::NONE,	8766,								"Set to authentication port number to listen on.");
OPTION(IntOption,		server_master_port,					OptionFlags::NONE,	27016,								"Set to port number than master server updater will listen on.");
OPTION(StringOption,	server_game_dir,					OptionFlags::NONE,  "zombiegrinder",					"Steamworks game-directory used for filtering lobbies and servers.")
OPTION(IntOption,		server_max_players,					OptionFlags::NONE,	16,									"Set the maximum number of players this server can host.");
OPTION(StringOption,	server_map_name,					OptionFlags::NONE,  "wave_tiny",						"Set the defualt map to load.")
OPTION(StringOption,	server_safe_map_name,				OptionFlags::NONE,  "wave_tiny",						"Set the map to load if for some reason it is unable to load the main map. Ideally this is never used! Don't modify if you don't know what your doing!")
OPTION(StringOption,	server_steam_version,				OptionFlags::NONE,  "1.0.0.0",							"Steam server version shown to the master server.")
#ifdef OPT_PREMIUM_ACCOUNTS
OPTION(BoolOption,		server_premium,						OptionFlags::NONE,	false,								"If set then only premium account holders can join.");
#endif

// Net settings
OPTION(IntOption,		net_poll_interval,					OptionFlags::NONE,	2,									"How many milliseconds between each polling of the network connections.");
OPTION(BoolOption,		net_allow_turn_servers,				OptionFlags::NONE,	true,								"Sets if network traffic is allowed to pass through TURN servers to traverse NAT.");
OPTION(IntOption,		net_connect_timeout,				OptionFlags::NONE,	10000,								"Sets the timeout for establishing connections to remote users.");
OPTION(IntOption,		net_virtual_port,					OptionFlags::NONE,	0,									"Sets the virtual port used to multiplex multiple socket connections between the same users.");
OPTION(IntOption,		net_max_active_connections,			OptionFlags::NONE,	64,									"Maximum number of simultaneous conenctions, any amount above this will be dropped.");

OPTION(IntOption,		net_server_tick_rate,				OptionFlags::NONE,	50,									"How many milliseconds between each network update sent by the server.");
OPTION(IntOption,		net_server_target_rate,				OptionFlags::NONE,	8 * 1024,							"How many bytes-per-second we should aim to send to clients per second. Lower priority objects will be updated less to maintain this rate.");
OPTION(IntOption,		net_client_tick_rate,				OptionFlags::NONE,	50,									"How many milliseconds between each network update sent by the client.");
OPTION(IntOption,		net_client_target_rate,				OptionFlags::NONE,	8 * 1024,							"How many bytes-per-second we should aim to send to the server per second. Lower priority objects will be updated less to maintain this rate.");
OPTION(IntOption,		net_snapshot_history,				OptionFlags::NONE,	20 * 1,								"How many state snapshots to hold for each entity. This determines how far back in time the game can rewind when doing prediction / replay.");

OPTION(FloatOption,		net_simulate_latency,				OptionFlags::NONE,	0,									"Applies the given latency to all sent and recieved packets.");
OPTION(FloatOption,		net_simulate_jitter,				OptionFlags::NONE,	0,									"Applies the given jitter to all sent and recieved packets.");

// Moved from GameOptions.inc for reasons >_>
OPTION(StringOption,	map_seed,							OptionFlags::NONE,	"",									"Map seed when starting editor/server automatically.");

// Product settings.
OPTION(StringOption,	product_name,						OptionFlags::NONE,  "ZombieGrinder",					"Name of this product.")
OPTION(StringOption,	product_description,				OptionFlags::NONE,  "Zombie Grinder",					"Description of this product.")

// Connection settings.
OPTION(LongOption,		connect_lobby,						OptionFlags::NONE,	0,									"Sets the address of the lobby to connect to.");
OPTION(LongOption,		connect,							OptionFlags::NONE,	0,									"Sets the address of the server to connect to.");

// Achievement stuff.
OPTION(BoolOption,		reset_achievements,					OptionFlags::NONE,	false,								"Resets internal representation of achievements.");
OPTION(BoolOption,		reset_stats,						OptionFlags::NONE,	false,								"Resets internal representation of statistics.");
OPTION(BoolOption,		reset_tutorials,					OptionFlags::NONE,	false,								"Resets internal representation of tutorials.");
OPTION(BoolOption,		reset_save_data,					OptionFlags::NONE,  false,								"Clear user configuration.")

OPTION(StringOption,	local_save_data_file,				OptionFlags::NONE,  "",									"Loads a save file from a local file, saving will be disabled when this is active.")


// Testing stuff.
OPTION(IntOption,		spawn_clients,						OptionFlags::NONE,	0,									"If above 0, this many clients will be spawned and will connect to this client. If we have less clients than this we will spawn more.");
OPTION(IntOption,		client_id,							OptionFlags::NONE,	0,									"What client ID we are (if spawned by spawn_clients).");

#ifndef MASTER_BUILD
OPTION(BoolOption,		enable_ai,							OptionFlags::NONE,	false,								"If set then players will use basic AI markers to navigate the level.");
OPTION(BoolOption,		enable_local_ai,					OptionFlags::NONE,	false,								"If set then local players (other than the first) will use basic AI markers to navigate the level.");
#endif

OPTION(IntOption,		client_min_life,					OptionFlags::NONE,	0,									"Minimum time spawn clients live for.");
OPTION(IntOption,		client_max_life,					OptionFlags::NONE,	0,									"Maximum time spawn clients live for.");
OPTION(IntOption,		client_lifetime,					OptionFlags::NONE,	0,									"How long this client should live for, or 0 for forever.");

OPTION(IntOption,		map_auto_change_timer,				OptionFlags::NONE,	0,									"How often the map should auto-change, 0 for never.");

OPTION(BoolOption,		log,								OptionFlags::NONE,	false,								"If set to true, the TTY of the program will be dumped to a file called game.log.");

// Global chat.
//103582791436067822ULL
OPTION(LongOption,		global_chat_group_id,				OptionFlags::NONE,	110338190877123566ULL,				"SteamID of global chat group.");

// Inventory system
OPTION(BoolOption,		generate_item_schema,				OptionFlags::NONE,	false,								"If set an inventory schema will be dumped based on item script values.");
OPTION(BoolOption,		give_self_all_items,				OptionFlags::NONE,	false,								"Generates inventory items for every item and gives them to the local player. Only works with server set in dev mode.");


