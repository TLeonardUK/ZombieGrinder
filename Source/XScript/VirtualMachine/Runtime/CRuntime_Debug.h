/* *****************************************************************

		CRuntime_Debug.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_DEBUG_H_
#define _CRUNTIME_DEBUG_H_

class CVirtualMachine;

class CRuntime_Debug
{
public:
	static void Error(CVirtualMachine* vm, CVMString msg);
	static void Break(CVirtualMachine* vm);
	static void Assert(CVirtualMachine* vm, int ret);
	static void AssertMsg(CVirtualMachine* vm, int ret, CVMString msg);

	static void Bind(CVirtualMachine* machine);

};

#endif