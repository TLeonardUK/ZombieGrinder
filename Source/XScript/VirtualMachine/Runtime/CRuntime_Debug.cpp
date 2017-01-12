/* *****************************************************************

		CRuntime_Debug.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Debug.h"

void CRuntime_Debug::Error(CVirtualMachine* vm, CVMString msg)
{
	vm->Assert(false, msg.C_Str());
}

void CRuntime_Debug::Break(CVirtualMachine* vm)
{
	vm->Assert(false);
}

void CRuntime_Debug::Assert(CVirtualMachine* vm, int ret)
{
	vm->Assert(ret != 0);
}

void CRuntime_Debug::AssertMsg(CVirtualMachine* vm, int ret, CVMString msg)
{
	vm->Assert(ret != 0, msg.C_Str());
}

void CRuntime_Debug::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<void,CVMString>		("Debug", "Error",		&Error);
	vm->Get_Bindings()->Bind_Function<void>					("Debug", "Break",		&Break);
	vm->Get_Bindings()->Bind_Function<void,int>				("Debug", "Assert",	&Assert);
	vm->Get_Bindings()->Bind_Function<void,int,CVMString>	("Debug", "AssertMsg",	&AssertMsg);
}