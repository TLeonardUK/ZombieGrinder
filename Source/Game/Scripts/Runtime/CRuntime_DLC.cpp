// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_DLC.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Online/DLCManager.h"

void CRuntime_DLC::Set_ID(CVirtualMachine* vm, CVMValue self, int string)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->id = string;
}

int CRuntime_DLC::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->id;
}

void CRuntime_DLC::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->name = string.C_Str();
}

CVMString CRuntime_DLC::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->name.c_str();
}

void CRuntime_DLC::Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	achievement->description = string.C_Str();
}

CVMString CRuntime_DLC::Get_Description(CVirtualMachine* vm, CVMValue self)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->description.c_str();
}

int CRuntime_DLC::Get_Purchased(CVirtualMachine* vm, CVMValue self)
{
	DLC* achievement = reinterpret_cast<DLC*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return achievement->purchased;
}

void CRuntime_DLC::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void,int>	("DLC", "Set_ID",					&Set_ID);
	vm->Get_Bindings()->Bind_Method<int>		("DLC", "Get_ID",					&Get_ID);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("DLC", "Set_Name",				&Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>		("DLC", "Get_Name",				&Get_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("DLC", "Set_Description",		&Set_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>		("DLC", "Get_Description",		&Get_Description);
	vm->Get_Bindings()->Bind_Method<int>			("DLC", "Get_Purchased",			&Get_Purchased);
}

