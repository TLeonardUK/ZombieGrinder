/* *****************************************************************

		CRuntime_Type.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_TYPE_H_
#define _CRUNTIME_TYPE_H_

class CVirtualMachine;

class CRuntime_Type
{
private:	
	static int Get_ID(CVirtualMachine* vm, CVMValue self);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle New(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Find(CVirtualMachine* vm, int id);
	static CVMObjectHandle Find_By_String(CVirtualMachine* vm, CVMString id);

public:
	static void Bind(CVirtualMachine* machine);

};

#endif

