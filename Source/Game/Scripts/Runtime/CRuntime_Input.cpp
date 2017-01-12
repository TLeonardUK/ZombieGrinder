// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Game/Scripts/Runtime/CRuntime_Input.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Network/GameNetUser.h"

#include "Game/Runner/Game.h"

#include "Engine/Input/Input.h"

#include "Engine/Display/GfxDisplay.h"

int CRuntime_Input::Is_Down(CVirtualMachine* vm, CVMObjectHandle net_user, int binding)
{
	if (Game::Get()->Is_Sub_Menu_Open())
		return 0;

	if (!Game::Get()->Get_Game_Input_Enabled())
		return 0;

	GameNetUser* user = reinterpret_cast<GameNetUser*>(net_user.GetNullCheck(vm)->Get_Meta_Data());
	InputSource source = user->Get_Online_User()->Get_Input_Source();
	if (source.Device_Index < 0)
	{
		return 0;
	}

	Input* input = Input::Get(source.Device_Index);
	OutputBindings::Type output = (OutputBindings::Type)binding;

	if (source.Device == InputSourceDevice::Joystick)
	{
		InputBindings::Type joystick_select = input->Output_To_Active_Input(output, InputBindingType::Joystick);			
		if (input->Get_Joystick_State()->Is_Key_Down(joystick_select) ||
			input->Get_Joystick_State()->Is_Action_Down(output))
		{
			return 1;
		}
	}
	else
	{
		InputBindings::Type keyboard_select = input->Output_To_Active_Input(output, InputBindingType::Keyboard);			
		if (input->Get_Keyboard_State()->Is_Key_Down(keyboard_select))
		{
			return 1;
		}
	}
	return 0;
}

int CRuntime_Input::Was_Pressed(CVirtualMachine* vm, CVMObjectHandle net_user, int binding)
{	
	if (Game::Get()->Is_Sub_Menu_Open())
		return 0;

	if (!Game::Get()->Get_Game_Input_Enabled())
		return 0;

	GameNetUser* user = reinterpret_cast<GameNetUser*>(net_user.GetNullCheck(vm)->Get_Meta_Data());
	InputSource source = user->Get_Online_User()->Get_Input_Source();
	if (source.Device_Index < 0)
	{
		return 0;
	}

	Input* input = Input::Get(source.Device_Index);
	if (source.Device == InputSourceDevice::Joystick)
	{
		InputBindings::Type joystick_select = input->Output_To_Active_Input((OutputBindings::Type)binding, InputBindingType::Joystick);			
		if (input->Get_Joystick_State()->Was_Key_Pressed(joystick_select) ||
			input->Get_Joystick_State()->Was_Action_Pressed((OutputBindings::Type)binding))
		{
			return 1;
		}
	}
	else
	{
		InputBindings::Type keyboard_select = input->Output_To_Active_Input((OutputBindings::Type)binding, InputBindingType::Keyboard);			
		if (input->Get_Keyboard_State()->Was_Key_Pressed(keyboard_select))
		{
			return 1;
		}
	}
	return 0;
}

void CRuntime_Input::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle,int>("Input", "Is_Down", &Is_Down);
	vm->Get_Bindings()->Bind_Function<int,CVMObjectHandle,int>("Input", "Was_Pressed", &Was_Pressed);
}
