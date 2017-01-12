// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RUNTIME_DISPLAY_
#define _ENGINE_RUNTIME_DISPLAY_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Display
{
public:
	static CVMObjectHandle Get_Resolution(CVirtualMachine* vm);
	static CVMObjectHandle Get_Game_Resolution(CVirtualMachine* vm);
	static float Get_Aspect_Ratio(CVirtualMachine* vm);

	static void Bind(CVirtualMachine* machine);
};

#endif
