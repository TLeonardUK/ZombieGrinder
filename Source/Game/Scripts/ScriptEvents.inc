// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Global events.
SCRIPT_EVENT_0
(
	On_Tick
)
SCRIPT_EVENT_0
(
	On_Global_Tick // Executed inside and outside game - don't use 
				   // for anything but achievements/stats/etc.
)

// Editor events.
SCRIPT_EVENT_0
(
	On_Editor_Property_Change
)

// Class-Specific events.
SCRIPT_EVENT_2
(
	On_Draw_Camera_HUD, 
	"Canvas", CVMObjectHandle,
	"int", int
)
SCRIPT_EVENT_1
(
	On_Draw_HUD, 
	"Canvas", CVMObjectHandle
)
SCRIPT_EVENT_0
(
	On_Possession_Synced
)

// General network events.
SCRIPT_EVENT_0
(
	On_Map_Load
)
SCRIPT_EVENT_0
(
	On_Game_Over
)

// User joining.
SCRIPT_EVENT_1
(
	On_User_Ready,
	"NetUser", CVMObjectHandle
)

// Collision events.
SCRIPT_EVENT_2
(
	On_Touch, 
	"CollisionComponent", CVMObjectHandle, 
	"Actor", CVMObjectHandle
)
SCRIPT_EVENT_2
(
	On_Enter, 
	"CollisionComponent", CVMObjectHandle, 
	"Actor", CVMObjectHandle
) 
SCRIPT_EVENT_2
(
	On_Leave, 
	"CollisionComponent", CVMObjectHandle, 
	"Actor", CVMObjectHandle
) 

// Damage events.
SCRIPT_EVENT_10
(
	On_Damage,
	"CollisionComponent", CVMObjectHandle,
	"Actor", CVMObjectHandle,
	"Vec3", CVMObjectHandle,
	"float", float,
	"float", float,
	"int", int,
	"int", int,
	"int", int,
	"int", int,
	"Item_Archetype", CVMObjectHandle
)

// Post process events.
SCRIPT_EVENT_0
(
	On_Shader_Setup
)

// Achievement events.
SCRIPT_EVENT_0
(
	On_Character_Created
)
SCRIPT_EVENT_0
(
	On_Level_Up
)
SCRIPT_EVENT_1
(
	On_Rank_Updated,
	"int", int
)