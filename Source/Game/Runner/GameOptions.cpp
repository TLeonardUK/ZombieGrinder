// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Runner/GameOptions.h"

#include "Engine/Options/OptionRegistry.h"

std::vector<InputBindingConfig> GameOptions::g_default_input_bindings;

#define OPTION(type, name, flags, default_value, description) \
	type GameOptions::name(default_value, #name, description, flags);
#include "Game/Runner/GameOptions.inc"
#undef OPTION

void GameOptions::Register_Default_Input_Bindings()
{
#define BIND(in, out) \
	g_default_input_bindings.push_back(InputBindingConfig(InputBindings::in, OutputBindings::out));
	
	BIND(Keyboard_F,			GUI_Input_Select)
	BIND(Keyboard_Space,		GUI_Select)
	BIND(Keyboard_Enter,		GUI_Select)
	BIND(Keyboard_Escape,		GUI_Back)
	BIND(Keyboard_Up,			GUI_Up)
	BIND(Keyboard_Down,			GUI_Down)
	BIND(Keyboard_Left,			GUI_Left)
	BIND(Keyboard_Right,		GUI_Right)

	// Keyboard Controls
	BIND(Keyboard_Tab,			Scoreboard)
	BIND(Keyboard_Escape,		Escape)
	BIND(Keyboard_Space,		Fire)
	BIND(Keyboard_Up,			Up)
	BIND(Keyboard_Down,			Down)
	BIND(Keyboard_Left,			Left)
	BIND(Keyboard_Right,		Right)
	BIND(Keyboard_R,			Reload)
	BIND(Keyboard_Num1,			WeaponSlot1)
	BIND(Keyboard_Num2,			WeaponSlot2)
	BIND(Keyboard_Num3,			WeaponSlot3)
	BIND(Keyboard_Num4,			WeaponSlot4)
	BIND(Keyboard_Num5,			WeaponSlot5)
	BIND(Keyboard_Num6,			WeaponSlot6)
	BIND(Keyboard_E,			Interact)
	BIND(Keyboard_V,			PushToTalk)
	BIND(Keyboard_Q,			WeaponCycleBackward)
	BIND(Keyboard_W,			WeaponCycleForward)
	BIND(Keyboard_T,			Chat)
	BIND(Keyboard_LeftShift,	Strafe)
	BIND(Keyboard_LeftCtrl,		Sprint)
	BIND(Keyboard_LeftAlt,		AltFire)
	BIND(Keyboard_F1,			VoteYes)
	BIND(Keyboard_F2,			VoteNo)
	BIND(Keyboard_Z,			SkillSlot1)
	BIND(Keyboard_X,			SkillSlot2)
	BIND(Keyboard_C,			SkillSlot3)
				
	BIND(Keyboard_G,			DropCoins)
	BIND(Keyboard_G,			DropWeapon)

	// Joystick GUI Controls 
	BIND(Joystick_Button8,		GUI_Input_Select)
	BIND(Joystick_Button1,		GUI_Select)
	BIND(Joystick_Button7,		GUI_Back)
	BIND(Joystick_Button2,		GUI_Back)
	BIND(Joystick_Axis1_Neg,	GUI_Up)
	BIND(Joystick_Axis1_Pos,	GUI_Down)
	BIND(Joystick_Axis2_Neg,	GUI_Left)
	BIND(Joystick_Axis2_Pos,	GUI_Right)
	BIND(Joystick_Hat1_Up,		GUI_Up)
	BIND(Joystick_Hat1_Down,	GUI_Down)
	BIND(Joystick_Hat1_Left,	GUI_Left)
	BIND(Joystick_Hat1_Right,	GUI_Right)
			
	// Joystick Controls 	
	BIND(Joystick_Axis5_Pos,	Fire)
	BIND(Joystick_Axis6_Pos,	AltFire)
	BIND(Joystick_Axis1_Neg,	Up)
	BIND(Joystick_Axis1_Pos,	Down)
	BIND(Joystick_Axis2_Neg,	Left)
	BIND(Joystick_Axis2_Pos,	Right)
	BIND(Joystick_Axis3_Neg,	LookUp)
	BIND(Joystick_Axis3_Pos,	LookDown)
	BIND(Joystick_Axis4_Neg,	LookLeft)
	BIND(Joystick_Axis4_Pos,	LookRight)
	BIND(Joystick_Button1,		Interact)
	BIND(Joystick_Button5,		WeaponCycleBackward)
	BIND(Joystick_Button6,		WeaponCycleForward)
	BIND(Joystick_Hat1_Left,	SkillSlot1)
	BIND(Joystick_Hat1_Up,		SkillSlot2)
	BIND(Joystick_Hat1_Right,	SkillSlot3)
	BIND(Joystick_Button8,		Scoreboard)
	BIND(Joystick_Button7,		Escape)
	BIND(Joystick_Button4,		Reload)
	BIND(Joystick_Button2,		Strafe)
	BIND(Joystick_Button3,		Sprint)

	BIND(Joystick_Hat1_Down,	DropCoins)
	BIND(Joystick_Hat1_Down,	DropWeapon)

#undef BIND

	*input_bindings = g_default_input_bindings;
}

void GameOptions::Register_Options()
{
	Register_Default_Input_Bindings();

	#define OPTION(type, name, flags, default_value, description) \
		OptionRegistry::Get()->Register(&name);
	#include "Game/Runner/GameOptions.inc"
	#undef OPTION
}

