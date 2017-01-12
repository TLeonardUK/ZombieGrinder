// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef BINDING
#error "BINDING must be defined when including this file!"
#endif

// WARNING: Keep in sync with script system.input!

BINDING(INVALID,					"INVALID")

// UI based stuff
BINDING(GUI_START,					"GUI_START")

	BINDING(GUI_Select,				"GUI Select")
	BINDING(GUI_Input_Select,		"GUI Input Select")
	BINDING(GUI_Back,				"GUI Back")
	BINDING(GUI_Up,					"GUI Up")
	BINDING(GUI_Down,				"GUI Down")
	BINDING(GUI_Left,				"GUI Left")
	BINDING(GUI_Right,				"GUI Right")

BINDING(GUI_END,					"GUI_END")

// Game based stuff (Should be in game project >_>)
BINDING(GAME_START,					"GAME_START")

	BINDING(Scoreboard,				"Show Scoreboard")
	BINDING(Escape,					"Back")
	BINDING(Fire,					"Fire")
	BINDING(Reload,					"Reload")
	BINDING(Up,						"Up")
	BINDING(Down,					"Down")
	BINDING(Left,					"Left")
	BINDING(Right,					"Right")
	BINDING(LookUp,					"Look Up")
	BINDING(LookDown,				"Look Down")
	BINDING(LookLeft,				"Look Left")
	BINDING(LookRight,				"Look Right")
	BINDING(WeaponSlot1,			"Weapon Slot 1")
	BINDING(WeaponSlot2,			"Weapon Slot 2")
	BINDING(WeaponSlot3,			"Weapon Slot 3")
	BINDING(WeaponSlot4,			"Weapon Slot 4")
	BINDING(WeaponSlot5,			"Weapon Slot 5")
	BINDING(WeaponSlot6,			"Weapon Slot 6")
	BINDING(Interact,				"Interact")
	BINDING(WeaponCycleBackward,	"Cycle Weapon Backwards")
	BINDING(WeaponCycleForward,		"Cycle Weapon Forwards")
	BINDING(Chat,					"Open Chat")
	BINDING(Strafe,					"Strafe")
	BINDING(Sprint,					"Sprint")
	BINDING(AltFire,				"Alternate Fire")
	BINDING(VoteYes,				"Vote Yes")
	BINDING(VoteNo,					"Vote No")
	BINDING(PushToTalk,				"Push To Talk")
	BINDING(SkillSlot1,				"Skill Slot 1")
	BINDING(SkillSlot2,				"Skill Slot 2")
	BINDING(SkillSlot3,				"Skill Slot 3")
	BINDING(DropCoins,				"Drop Coins (Tap)")
	BINDING(DropWeapon,				"Drop Weapon (Hold)")

BINDING(GAME_END,					"GAME_END")

BINDING(COUNT,						"COUNT")