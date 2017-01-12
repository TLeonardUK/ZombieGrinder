// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Statistics.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Game/Online/StatisticsManager.h"

void CRuntime_Statistics::Set_ID(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->id = string.C_Str();
}

CVMString CRuntime_Statistics::Get_ID(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->id.c_str();
}

void CRuntime_Statistics::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->name = string.C_Str();
}

CVMString CRuntime_Statistics::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->name.c_str();
}

void CRuntime_Statistics::Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->description = string.C_Str();
}

CVMString CRuntime_Statistics::Get_Description(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->description.c_str();
}

void CRuntime_Statistics::Set_Category(CVirtualMachine* vm, CVMValue self, CVMString string)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->category = string.C_Str();
}

CVMString CRuntime_Statistics::Get_Category(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->category.c_str();
}

void CRuntime_Statistics::Set_Aggregated(CVirtualMachine* vm, CVMValue self, int value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->aggregated = value != 0;
}

int CRuntime_Statistics::Get_Aggregated(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->aggregated;
}

void CRuntime_Statistics::Set_Aggregation_Period(CVirtualMachine* vm, CVMValue self, int value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->aggregation_period = value;
}

int CRuntime_Statistics::Get_Aggregation_Period(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->aggregation_period;
}

void CRuntime_Statistics::Set_Display(CVirtualMachine* vm, CVMValue self, int value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->display = value != 0;
}

int CRuntime_Statistics::Get_Display(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->display;
}

void CRuntime_Statistics::Set_Value(CVirtualMachine* vm, CVMValue self, float value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->value = value;
}

float CRuntime_Statistics::Get_Value(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->value;
}

float CRuntime_Statistics::Get_Aggregated_Value(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->aggregated_value;
}

void CRuntime_Statistics::Set_Aggregated_Value(CVirtualMachine* vm, CVMValue self, float value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
//	statistic->aggregated_value = value;
}

void CRuntime_Statistics::Set_Data_Type(CVirtualMachine* vm, CVMValue self, int value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	statistic->type = (StatisticDataType::Type)value;
}

int CRuntime_Statistics::Get_Data_Type(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return (int)statistic->type;
}

void CRuntime_Statistics::Set_Mirror_Statistic(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	CVMLinkedSymbol* class_symbol = reinterpret_cast<CVMLinkedSymbol*>(value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	statistic->mirror_stat_type = class_symbol;
}

CVMObjectHandle CRuntime_Statistics::Get_Mirror_Statistic(CVirtualMachine* vm, CVMValue self)
{
	Statistic* statistic = reinterpret_cast<Statistic*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return statistic->mirror_stat_type == NULL ? NULL : statistic->mirror_stat_type->type_instance.Get();
}

void CRuntime_Statistics::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Statistic", "Set_ID",					&Set_ID);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Statistic", "Get_ID",					&Get_ID);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Statistic", "Set_Name",				&Set_Name);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Statistic", "Get_Name",				&Get_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Statistic", "Set_Description",		&Set_Description);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Statistic", "Get_Description",		&Get_Description);
	vm->Get_Bindings()->Bind_Method<void,CVMString>	("Statistic", "Set_Category",			&Set_Category);
	vm->Get_Bindings()->Bind_Method<CVMString>		("Statistic", "Get_Category",			&Get_Category);
	vm->Get_Bindings()->Bind_Method<void,int>		("Statistic", "Set_Aggregated",			&Set_Aggregated);
	vm->Get_Bindings()->Bind_Method<int>			("Statistic", "Get_Aggregated",			&Get_Aggregated);
	vm->Get_Bindings()->Bind_Method<void,int>		("Statistic", "Set_Aggregation_Period",	&Set_Aggregation_Period);
	vm->Get_Bindings()->Bind_Method<int>			("Statistic", "Get_Aggregation_Period",	&Get_Aggregation_Period);
	vm->Get_Bindings()->Bind_Method<void,int>		("Statistic", "Set_Display",			&Set_Display);
	vm->Get_Bindings()->Bind_Method<int>			("Statistic", "Get_Display",			&Get_Display);
	vm->Get_Bindings()->Bind_Method<void,float>		("Statistic", "Set_Value",				&Set_Value);
	vm->Get_Bindings()->Bind_Method<float>			("Statistic", "Get_Value",				&Get_Value);
	vm->Get_Bindings()->Bind_Method<float>			("Statistic", "Get_Aggregated_Value",	&Get_Aggregated_Value);
	vm->Get_Bindings()->Bind_Method<void,float>		("Statistic", "Set_Aggregated_Value",	&Set_Aggregated_Value);
	vm->Get_Bindings()->Bind_Method<void,int>		("Statistic", "Set_Data_Type",			&Set_Data_Type);
	vm->Get_Bindings()->Bind_Method<int>			("Statistic", "Get_Data_Type",			&Get_Data_Type);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>	("Statistic", "Set_Mirror_Statistic",			&Set_Mirror_Statistic);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>		("Statistic", "Get_Mirror_Statistic",			&Get_Mirror_Statistic);

}

