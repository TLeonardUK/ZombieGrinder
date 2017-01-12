// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RUNTIME_LOCALE_
#define _ENGINE_RUNTIME_LOCALE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Locale
{
public:
	static CVMString Get_String(CVirtualMachine* vm, CVMString id);
	static void Change_Language(CVirtualMachine* vm, CVMString id);

	static void Bind(CVirtualMachine* machine);
};

#endif
