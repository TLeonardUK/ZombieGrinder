// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "Engine/Scripts/Runtime/CRuntime_Locale.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Localise/Locale.h"

CVMString CRuntime_Locale::Get_String(CVirtualMachine* vm, CVMString id)
{
	return Locale::Get()->Get_String(id.C_Str());
}

void CRuntime_Locale::Change_Language(CVirtualMachine* vm, CVMString id)
{
	Locale::Get()->Change_Language(id.C_Str());
}

void CRuntime_Locale::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<CVMString,CVMString>("Locale", "Get_String", &Get_String);
	vm->Get_Bindings()->Bind_Function<void,CVMString>("Locale", "Change_Language", &Change_Language);
}
