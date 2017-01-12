/* *****************************************************************

		CRuntime_Object.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_OBJECT_H_
#define _CRUNTIME_OBJECT_H_

class CVirtualMachine;

class CRuntime_Object
{
public:
	static CVMString ToString(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle GetType(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif