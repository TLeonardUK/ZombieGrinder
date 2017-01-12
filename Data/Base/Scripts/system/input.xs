// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//	Input binding types, keep synced with values in;
//		Engine/Input/OutputBindings.inc
// -----------------------------------------------------------------------------
public enum OutputBindings
{
	INVALID,

	GUI_START,
		GUI_Select,
		GUI_Input_Select,		
		GUI_Back,				
		GUI_Up,					
		GUI_Down,				
		GUI_Left,			
		GUI_Right,			
	GUI_END,					

	GAME_START,				
		Scoreboard,				
		Escape,					
		Fire,	
		Reload,		
		Up,					
		Down,				
		Left,				
		Right,		
		LookUp,
		LookDown,
		LookLeft,
		LookRight,
		WeaponSlot1,		
		WeaponSlot2,			
		WeaponSlot3,	
		WeaponSlot4,		
		WeaponSlot5,	
		WeaponSlot6,			
		Interact,				
		WeaponCycleBackward,	
		WeaponCycleForward,		
		Chat,					
		Strafe,					
		Sprint,					
		AltFire,				
		VoteYes,				
		VoteNo,					
		PushToTalk,				
		SkillSlot1,			
		SkillSlot2,				
		SkillSlot3,	
        DropCoins,
        DropWeapon,
	GAME_END,				

	COUNT,					
}

// -----------------------------------------------------------------------------
//	Gets the input status for a given user. Get an instance from NetUser.Input.
// -----------------------------------------------------------------------------
public static native("Input") class Input
{
	public static native("Is_Down") bool Is_Down(NetUser user, OutputBindings bind);
	public static native("Was_Pressed") bool Was_Pressed(NetUser user, OutputBindings bind);
}
