/* *****************************************************************

		CRuntime_OS.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_OS.h"

#include "Engine/Platform/Platform.h"

float CRuntime_OS::Ticks(CVirtualMachine* vm)
{
	return (float)Platform::Get()->Get_Ticks();
}

void CRuntime_OS::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<float>("OS", "Ticks", &Ticks);
}