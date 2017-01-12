// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Game/Scripts/Runtime/CRuntime_Time.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "Engine/Engine/GameEngine.h"

float CRuntime_Time::Get_Frame_Time(CVirtualMachine* vm)
{
	return GameEngine::Get()->Get_Time()->Get_Frame_Time();
}

float CRuntime_Time::Get_Delta(CVirtualMachine* vm)
{
	return GameEngine::Get()->Get_Time()->Get_Delta();
}

float CRuntime_Time::Get_Delta_Seconds(CVirtualMachine* vm)
{
	return GameEngine::Get()->Get_Time()->Get_Delta_Seconds();
}

float CRuntime_Time::Get_Ticks(CVirtualMachine* vm)
{
	// TODO: Fix this properly. This is a terrible fix for the fact that xscript dosen't support doubles atm
	//		 and float accuracy can go to shit after running for a while. We reset the script ticks on each map load.
	double elapsed = (Platform::Get()->Get_Ticks() - GameEngine::Get()->Get_Script_Tick_Base());
	return (float)elapsed;
}

void CRuntime_Time::Sleep(CVirtualMachine* vm, float seconds)
{
	vm->Sleep(seconds);
}

void CRuntime_Time::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<float>("Time", "Get_Frame_Time", &Get_Frame_Time);
	vm->Get_Bindings()->Bind_Function<float>("Time", "Get_Delta", &Get_Delta);
	vm->Get_Bindings()->Bind_Function<float>("Time", "Get_Delta_Seconds", &Get_Delta_Seconds);
	vm->Get_Bindings()->Bind_Function<float>("Time", "Get_Ticks", &Get_Ticks);
	vm->Get_Bindings()->Bind_Function<void,float>("Time", "Sleep", &Sleep);
}

