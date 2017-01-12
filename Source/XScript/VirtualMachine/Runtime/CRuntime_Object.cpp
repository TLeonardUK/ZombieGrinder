/* *****************************************************************

		CRuntime_Object.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Object.h"
#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMBinary.h"

CVMString CRuntime_Object::ToString(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return CVMString(array_obj->Get_Symbol()->symbol->name);
}

CVMObjectHandle CRuntime_Object::GetType(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	//DBG_LOG("GetType Sym=%s Type=%s", array_obj->Get_Symbol()->symbol->name, array_obj->Get_Symbol()->type_instance.Get()->Get_Symbol()->symbol->name);
	return array_obj->Get_Symbol()->type_instance;
}

void CRuntime_Object::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<CVMString>("Object", "ToString", &ToString);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Object", "GetType", &GetType);
}