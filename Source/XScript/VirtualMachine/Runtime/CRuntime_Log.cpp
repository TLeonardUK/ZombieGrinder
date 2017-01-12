/* *****************************************************************

		CRuntime_Log.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Log.h"

#include "XScript/Helpers/CPathHelper.h"

#include "Generic/Helper/StringHelper.h"
#include "Generic/Helper/PersistentLogHelper.h"

#include "Generic/Types/Color.h"

void Write(CVirtualMachine* vm, CVMString message)
{
	CVMDebugInfo* info = vm->Resolve_Debug_Info(vm->Get_Activation_Context(1));

	int line = info->row;
	int file_index = info->file_index;

	CVMLinkedSymbol* func_symbol  = vm->Get_Stack_Frame(2);
	CVMLinkedSymbol* class_symbol = vm->Get_Symbol_Table_Entry(func_symbol->symbol->class_scope_index);

	if (file_index >= 0)
	{
		const char* original_path = vm->Get_String_Table_Entry(file_index);
		const char* stripped_path = strrchr(original_path, '/');

		if (stripped_path == NULL)
		{
			stripped_path = original_path;
		}
		else
		{
			stripped_path++;
		}

		DBG_LOG("[%s:%i] [%s.%s] %s", stripped_path, line, class_symbol->symbol->name, func_symbol->symbol->name, message.C_Str());
	}
	else
	{
		DBG_LOG("[unknown-script] [%s.%s] %s", class_symbol->symbol->name, func_symbol->symbol->name, message.C_Str());
	}
}

void Assert(CVirtualMachine* vm, int cond)
{
	if (cond == 0)
	{
		vm->Assert(!!cond);
	}
}

void Assert_Message(CVirtualMachine* vm, int cond, CVMString message)
{
	if (cond == 0)
	{
		vm->Assert(!!cond, message.C_Str());
	}
}

void WriteOnScreen(CVirtualMachine* vm, CVMString id, CVMObjectHandle color, float lifetime, CVMString message)
{
	int hash = StringHelper::Hash(id.C_Str());
	Vector4 color_vec = color.Get()->To_Vec4();
	Color c
	(
		color_vec.X * 255.0f,
		color_vec.Y * 255.0f,
		color_vec.Z * 255.0f,
		color_vec.W * 255.0f
	);
	DBG_ONSCREEN_LOG(hash, c, lifetime, message.C_Str());
}

void CRuntime_Log::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Function<void,CVMString>("Log", "Write", &Write);
	vm->Get_Bindings()->Bind_Function<void, CVMString, CVMObjectHandle, float, CVMString>("Log", "WriteOnScreen", &WriteOnScreen);
	vm->Get_Bindings()->Bind_Function<void,int>("Log", "Assert", &Assert);	
	vm->Get_Bindings()->Bind_Function<void,int,CVMString>("Log", "Assert_Message", &Assert_Message);
}