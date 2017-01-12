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

//	   Type										Name								Flags				Default Value						Description				

// Rendering settings.
OPTION(FloatOption,								graphics_quality,					OptionFlags::Saved,	1.0f,								"Graphics quality level, between 0 and 1. Affects several internal settings such as gib counts.");
OPTION(BoolOption,								distortion_enabled,					OptionFlags::Saved,	true,								"Toggles distortion overlay shaders.");
OPTION(BoolOption,								shaders_enabled,					OptionFlags::Saved,	true,								"Toggles between programmable and fixed function pipelines.");
OPTION(BoolOption,								vsync_enabled,						OptionFlags::Saved,	true,								"Toggles vsync refresh rate locking.");
OPTION(BoolOption,								hq2x_enabled,						OptionFlags::NONE,	false,								"Toggles HQ2X graphics smoothing effect.");

// Gameplay settings.
OPTION(BoolOption,								screen_shake_enabled,				OptionFlags::Saved,	true,								"Toggles screen shake effect.");
OPTION(BoolOption,								interaction_arrows_enabled,			OptionFlags::Saved,	true,								"Toggles arrows above interactable objects.");
OPTION(BoolOption,								interaction_messages_enabled,		OptionFlags::Saved,	true,								"Toggles messages when near interactable objects.");
OPTION(BoolOption,								occlusion_transparency_enabled,		OptionFlags::Saved,	true,								"Toggles occlusion transparency when objects overlap players.");
OPTION(BoolOption,								require_input_to_pickup_items,		OptionFlags::Saved,	false,								"Toggles the requirement for pressing action to pickup items.");
OPTION(BoolOption,								show_perf_overlay,					OptionFlags::Saved,	false,								"Toggles the in-game performance overlay.");
OPTION(BoolOption,								show_extended_perf_overlay,			OptionFlags::Saved,	false,								"Toggles the extended in-game performance overlay.");
OPTION(BoolOption,								fire_when_aiming_enabled,			OptionFlags::Saved,	false,								"Toggles firing when aiming.");
OPTION(BoolOption,								show_numeric_stats,					OptionFlags::Saved,	false,								"Toggles numeric stats.");

// Input settings.
OPTION(FloatOption,								joystick_deadzone,					OptionFlags::Saved,	0.33f,								"Percentage of joystick range that produces no input.");
OPTION(FloatOption,								joystick_vibration,					OptionFlags::Saved,	1.0f,								"Strength of joystick vibration from 0 to 1.");
OPTION(StructListOption<InputBindingConfig>,	input_bindings,						OptionFlags::Saved,	g_default_input_bindings,			"Bindings between physics HID inputs and virtual actions");
OPTION(IntOption,								binding_upgrade_version,			OptionFlags::Saved,	0,									"Version number used for upgrading input bindings after large changes to input system.");

// Network settings.
OPTION(FloatOption,								network_user_state_update_rate,		OptionFlags::NONE,	500.0f,								"How many milliseconds between each users state update. User states include minor information like ping/scoreboard-entries/etc.");
OPTION(IntOption,								network_chat_max_history,			OptionFlags::NONE,	30,									"How large the buffer is for chat message history.");
OPTION(FloatOption,								chat_fade_delay,					OptionFlags::NONE,	8000.0f,							"How long to show a chat message in milliseconds before fading it out.");
OPTION(FloatOption,								chat_fade_duration,					OptionFlags::NONE,	500.0f,								"How long to it takes for a chat message to fade out.");

// Map settings.
OPTION(BoolOption,								editor,								OptionFlags::NONE,	false,								"Starts the editor.");
OPTION(StringOption,							map,								OptionFlags::NONE,	"",									"Map used when starting editor/server automatically.");
OPTION(IntOption,								local_players,						OptionFlags::NONE,	1,									"Number of local players to automatically login.");

// General settings.
OPTION(BoolOption,								convert_legacy_maps,				OptionFlags::NONE,	false,								"WARNING DEBUG: Converts old-style ZG maps to new ones. Overwrites existing maps.");

#ifdef OPT_PREMIUM_ACCOUNTS
// DLC ID's.
OPTION(IntOption,								premium_account_appid,				OptionFlags::NONE,	321500,								"AppID for premium accounts.");
#endif

// Crash sentinel.
OPTION(BoolOption,								graceful_shutdown,					OptionFlags::Saved,	true,								"Shutdown sentinel.");

// Global chat.
OPTION(BoolOption,								pipe_global_chat,					OptionFlags::Saved,	true,								"If enabled global chat is piped into game chat box.");

// Admin setup
OPTION(StringOption,							admin_username,						OptionFlags::NONE,	"",									"Administration username to access admin panel. Empty if admin panel is not enabled.");
OPTION(StringOption,							admin_password,						OptionFlags::NONE,	"",									"Administration password to access admin panel. Empty if admin panel is not enabled.");
OPTION(IntOption,								admin_port,							OptionFlags::NONE,	77889,								"Port used to access admin panel.");

// Monitor setup
OPTION(IntOption,								monitor_port,						OptionFlags::NONE,	34567,								"Port used to access monitor and debug the game. Only functional in development builds.");
OPTION(BoolOption,								monitor_enabled,					OptionFlags::NONE,	false,								"Enables monitor server. Only functional in development builds.");

// Workshop
OPTION(BoolOption,								workshop_warning_shown,				OptionFlags::Saved,	false,								"Defines if the workshop warning has been shown yet or not.");
OPTION(BoolOption,								workshop_show_unapproved,			OptionFlags::Saved,	false,								"Defines if unapproved items should be shown in the workshop.");

// Cheat  Reporting
OPTION(BoolOption,								ban_message_shown,					OptionFlags::NONE,	false,								"Flag to keep track of if the ban message has been shown.");
OPTION(BoolOption,								mod_message_shown,					OptionFlags::Saved,	false,								"Flag to keep track of if the moderation message has been shown.");
