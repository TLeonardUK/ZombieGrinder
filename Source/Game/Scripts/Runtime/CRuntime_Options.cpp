// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Game/Scripts/Runtime/CRuntime_Options.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Options/OptionRegistry.h"
#include "Engine/Options/Types/BoolOption.h"

int CRuntime_Options::Get_Bool(CVirtualMachine* vm, CVMString name)
{
	Option* opt = OptionRegistry::Get()->Find(StringHelper::Hash(name.C_Str()));
	if (opt == NULL)
	{
		return false;
	}

	BoolOption* bool_opt = dynamic_cast<BoolOption*>(opt);
	if (bool_opt == NULL)
	{
		return false;
	}

	bool val = **bool_opt;
	return val ? 1 : 0;
}

void CRuntime_Options::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<int,CVMString>("Options", "Get_Bool", &Get_Bool);
}
