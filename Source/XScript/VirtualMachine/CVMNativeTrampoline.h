/* *****************************************************************

		CVMNativeTrampoline.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMNATIVETRAMPOLINE_H_
#define _CVMNATIVETRAMPOLINE_H_

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"

// --------------------------------------------------------------------------------------------------
// Basic native trampoline.
// --------------------------------------------------------------------------------------------------
class CVMNativeTrampoline
{
public:
	virtual void Invoke(CVirtualMachine* vm) = 0;

};

#endif

