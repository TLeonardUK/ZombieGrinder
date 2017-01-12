/* *****************************************************************

		CRuntime_Object.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Type.h"
#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMBinary.h"

int CRuntime_Type::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	return class_symbol->index;
}

CVMString CRuntime_Type::Get_Name(CVirtualMachine* vm, CVMValue self)
{	
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	return class_symbol->symbol->name;
}

CVMObjectHandle CRuntime_Type::New(CVirtualMachine* vm, CVMValue self)
{	
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	return vm->New_Object(class_symbol);
}

CVMObjectHandle CRuntime_Type::Find(CVirtualMachine* vm, int id)
{
	// wut is the point of this -_-
	if (id == -1)
	{
		return NULL;
	}
	return vm->Get_Symbol_Table_Entry(id)->type_instance;
}

CVMObjectHandle CRuntime_Type::Find_By_String(CVirtualMachine* vm, CVMString id)
{
	CVMLinkedSymbol* sym = vm->Find_Class(id.C_Str());
	return sym == NULL ? NULL : sym->type_instance.Get();
}

void CRuntime_Type::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>("Type", "Get_ID", &Get_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>("Type", "Get_Name", &Get_Name);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Type", "New", &New);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,int>("Type", "Find", &Find);
	vm->Get_Bindings()->Bind_Function<CVMObjectHandle,CVMString>("Type", "Find_By_String", &Find_By_String);
}