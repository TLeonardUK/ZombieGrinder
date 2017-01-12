/* *****************************************************************

		CRuntime_Array.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Array.h"
#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMBinary.h"

int CRuntime_Array::Length(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Slot_Count();
}

void CRuntime_Array::Resize(CVirtualMachine* vm, CVMValue self, int size)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Resize(size);
}

void CRuntime_Array::SetIndex(CVirtualMachine* vm, CVMValue self, int index, CVMValue value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Get_Slot(index) = value;
}

CVMValue CRuntime_Array::GetIndex(CVirtualMachine* vm, CVMValue self, int index)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Get_Slot(index);
}

CVMValue CRuntime_Array::GetSlice1(CVirtualMachine* vm, CVMValue self, int start)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Slice(start);
}

CVMValue CRuntime_Array::GetSlice2(CVirtualMachine* vm, CVMValue self, int start, int end)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Slice(start, end);
}

void CRuntime_Array::Shift(CVirtualMachine* vm, CVMValue self, int offset)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Shift(offset);
}

void CRuntime_Array::AddFirst(CVirtualMachine* vm, CVMValue self, CVMValue value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Add_First(value);
}

void CRuntime_Array::AddArrayFirst(CVirtualMachine* vm, CVMValue self, CVMValue value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);
	
	CVMObject* other_obj =  value.object_value.Get();
	vm->Assert(other_obj != NULL);

	array_obj->Add_Array_First(other_obj);
}

void CRuntime_Array::AddLast(CVirtualMachine* vm, CVMValue self, CVMValue value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Add_Last(value);
}

void CRuntime_Array::AddArrayLast(CVirtualMachine* vm, CVMValue self, CVMValue value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	CVMObject* other_obj =  value.object_value.Get();
	vm->Assert(other_obj != NULL);

	array_obj->Add_Array_Last(other_obj);
}

void CRuntime_Array::Clear(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Clear();
}

CVMValue CRuntime_Array::Clone(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Clone();
}

int CRuntime_Array::Contains(CVirtualMachine* vm, CVMValue self, CVMValue needle)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Contains(needle);
}

void CRuntime_Array::Replace(CVirtualMachine* vm, CVMValue self, CVMValue what, CVMValue to)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Replace(what, to);
}

void CRuntime_Array::Reverse(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Reverse();
}

CVMValue CRuntime_Array::RemoveIndex(CVirtualMachine* vm, CVMValue self, int index)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Remove_Index(index);
}

void CRuntime_Array::Remove(CVirtualMachine* vm, CVMValue self, CVMValue needle)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Remove(needle);
}

CVMValue CRuntime_Array::RemoveFirst(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Remove_First();
}

CVMValue CRuntime_Array::RemoveLast(CVirtualMachine* vm, CVMValue self)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Remove_Last();
}

void CRuntime_Array::Insert(CVirtualMachine* vm, CVMValue self, CVMValue value, int index)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Insert(value, index);
}

void CRuntime_Array::TrimStart(CVirtualMachine* vm, CVMValue self, CVMValue needle)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Trim_Start(needle);
}

void CRuntime_Array::TrimEnd(CVirtualMachine* vm, CVMValue self, CVMValue needle)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Trim_End(needle);
}

void CRuntime_Array::Trim(CVirtualMachine* vm, CVMValue self, CVMValue needle)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Trim(needle);
}

void CRuntime_Array::PadLeft(CVirtualMachine* vm, CVMValue self, int length, CVMValue pad_value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Pad_Left(length, pad_value);
}

void CRuntime_Array::PadRight(CVirtualMachine* vm, CVMValue self, int length, CVMValue pad_value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	array_obj->Pad_Right(length, pad_value);
}

int CRuntime_Array::IndexOf(CVirtualMachine* vm, CVMValue self, int start, CVMValue pad_value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Index_Of(pad_value, start);
}

int CRuntime_Array::IndexOfAny(CVirtualMachine* vm, CVMValue self, int start, CVMValue pad_value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Index_Of_Any(pad_value, start);
}

int CRuntime_Array::LastIndexOf(CVirtualMachine* vm, CVMValue self, int end, CVMValue pad_value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Last_Index_Of(pad_value, end);
}

int CRuntime_Array::LastIndexOfAny(CVirtualMachine* vm, CVMValue self, int end, CVMValue pad_value)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	return array_obj->Last_Index_Of_Any(pad_value, end);
}

void CRuntime_Array::Sort(CVirtualMachine* vm, CVMValue self, CVMValue comparer)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	CVMObject* comparer_obj =  comparer.object_value.Get();
	vm->Assert(comparer_obj != NULL);

	array_obj->Sort(comparer_obj);
}

void CRuntime_Array::ClearElements(CVirtualMachine* vm, CVMValue self, CVMValue needle)
{
	CVMObject* array_obj =  self.object_value.Get();
	vm->Assert(array_obj != NULL);

	for (int i = 0; i < array_obj->Slot_Count(); i++)
	{
		array_obj->Get_Slot(i) = needle;
	}
}

void CRuntime_Array::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>					("Array", "Length",		&Length);
	vm->Get_Bindings()->Bind_Method<void,int>				("Array", "Resize",		&Resize);
	vm->Get_Bindings()->Bind_Method<void,int,CVMValue>		("Array", "SetIndex",		&SetIndex);
	vm->Get_Bindings()->Bind_Method<CVMValue,int>			("Array", "GetIndex",		&GetIndex);
	vm->Get_Bindings()->Bind_Method<CVMValue,int>			("Array", "GetSlice1",		&GetSlice1);
	vm->Get_Bindings()->Bind_Method<CVMValue,int,int>		("Array", "GetSlice2",		&GetSlice2);
	vm->Get_Bindings()->Bind_Method<void,int>				("Array", "Shift",			&Shift);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "AddFirst",		&AddFirst);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "AddArrayFirst",	&AddArrayFirst);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "AddLast",		&AddLast);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "AddArrayLast",	&AddArrayLast);
	vm->Get_Bindings()->Bind_Method<void>					("Array", "Clear",			&Clear);
	vm->Get_Bindings()->Bind_Method<CVMValue>				("Array", "Clone",			&Clone);
	vm->Get_Bindings()->Bind_Method<int,CVMValue>			("Array", "Contains",		&Contains);
	vm->Get_Bindings()->Bind_Method<void,CVMValue,CVMValue>	("Array", "Replace",		&Replace);
	vm->Get_Bindings()->Bind_Method<void>					("Array", "Reverse",		&Reverse);
	vm->Get_Bindings()->Bind_Method<CVMValue,int>			("Array", "RemoveIndex",	&RemoveIndex);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "Remove",		&Remove);
	vm->Get_Bindings()->Bind_Method<CVMValue>				("Array", "RemoveFirst",	&RemoveFirst);
	vm->Get_Bindings()->Bind_Method<CVMValue>				("Array", "RemoveLast",	&RemoveLast);	
	vm->Get_Bindings()->Bind_Method<void,CVMValue,int>		("Array", "Insert",		&Insert);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "TrimStart",		&TrimStart);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "TrimEnd",		&TrimEnd);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "Trim",			&Trim);
	vm->Get_Bindings()->Bind_Method<void,int,CVMValue>		("Array", "PadLeft",		&PadLeft);
	vm->Get_Bindings()->Bind_Method<void,int,CVMValue>		("Array", "PadRight",		&PadRight);
	vm->Get_Bindings()->Bind_Method<int,int,CVMValue>		("Array", "IndexOf",		&IndexOf);
	vm->Get_Bindings()->Bind_Method<int,int,CVMValue>		("Array", "IndexOfAny",	&IndexOfAny);
	vm->Get_Bindings()->Bind_Method<int,int,CVMValue>		("Array", "LastIndexOf",	&LastIndexOf);
	vm->Get_Bindings()->Bind_Method<int,int,CVMValue>		("Array", "LastIndexOfAny",&LastIndexOfAny);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "Sort",			&Sort);
	vm->Get_Bindings()->Bind_Method<void,CVMValue>			("Array", "ClearElements",		&ClearElements);
}