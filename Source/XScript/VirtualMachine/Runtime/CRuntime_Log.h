/* *****************************************************************

		CRuntime_Log.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_LOG_H_
#define _CRUNTIME_LOG_H_

class CVirtualMachine;

class CRuntime_Log
{
public:
	static void Bind(CVirtualMachine* machine);

};

#endif