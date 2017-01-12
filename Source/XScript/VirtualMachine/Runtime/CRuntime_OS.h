/* *****************************************************************

		CRuntime_OS.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_OS_H_
#define _CRUNTIME_OS_H_

class CVirtualMachine;

class CRuntime_OS
{
public:
	static float Ticks(CVirtualMachine* vm);

	static void Bind(CVirtualMachine* machine);

};

#endif