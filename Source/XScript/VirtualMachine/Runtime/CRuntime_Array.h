/* *****************************************************************

		CRuntime_Array.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CRUNTIME_ARRAY_H_
#define _CRUNTIME_ARRAY_H_

#include "XScript/VirtualMachine/CVMValue.h"

class CVirtualMachine;
class CVMObject;

class CRuntime_Array
{
public:
	static int Length(CVirtualMachine* vm, CVMValue self);
	static void Resize(CVirtualMachine* vm, CVMValue self, int size);
	static void SetIndex(CVirtualMachine* vm, CVMValue self, int index, CVMValue value);
	static CVMValue GetIndex(CVirtualMachine* vm, CVMValue self, int index);
	static CVMValue GetSlice1(CVirtualMachine* vm, CVMValue self, int start);
	static CVMValue GetSlice2(CVirtualMachine* vm, CVMValue self, int start, int end);
	static void Shift(CVirtualMachine* vm, CVMValue self, int offset);
	static void AddFirst(CVirtualMachine* vm, CVMValue self, CVMValue value);
	static void AddArrayFirst(CVirtualMachine* vm, CVMValue self, CVMValue value);
	static void AddLast(CVirtualMachine* vm, CVMValue self, CVMValue value);
	static void AddArrayLast(CVirtualMachine* vm, CVMValue self, CVMValue value);
	static void Clear(CVirtualMachine* vm, CVMValue self);
	static CVMValue Clone(CVirtualMachine* vm, CVMValue self);
	static void CopyTo(CVirtualMachine* vm, CVMValue self, CVMValue other);
	static int Contains(CVirtualMachine* vm, CVMValue self, CVMValue needle);
	static void Replace(CVirtualMachine* vm, CVMValue self, CVMValue what, CVMValue to);
	static void Reverse(CVirtualMachine* vm, CVMValue self);
	static CVMValue RemoveIndex(CVirtualMachine* vm, CVMValue self, int index);
	static void Remove(CVirtualMachine* vm, CVMValue self, CVMValue needle);
	static CVMValue RemoveFirst(CVirtualMachine* vm, CVMValue self);
	static CVMValue RemoveLast(CVirtualMachine* vm, CVMValue self);
	static void Insert(CVirtualMachine* vm, CVMValue self, CVMValue value, int index);
	static void TrimStart(CVirtualMachine* vm, CVMValue self, CVMValue needle);
	static void TrimEnd(CVirtualMachine* vm, CVMValue self, CVMValue needle);
	static void Trim(CVirtualMachine* vm, CVMValue self, CVMValue needle);
	static void PadLeft(CVirtualMachine* vm, CVMValue self, int length, CVMValue pad_value);
	static void PadRight(CVirtualMachine* vm, CVMValue self, int length, CVMValue pad_value);
	static int IndexOf(CVirtualMachine* vm, CVMValue self, int start, CVMValue pad_value);
	static int IndexOfAny(CVirtualMachine* vm, CVMValue self, int start, CVMValue pad_value);
	static int LastIndexOf(CVirtualMachine* vm, CVMValue self, int end, CVMValue pad_value);
	static int LastIndexOfAny(CVirtualMachine* vm, CVMValue self, int end, CVMValue pad_value);
	static void Sort(CVirtualMachine* vm, CVMValue self, CVMValue comparer);
	static void ClearElements(CVirtualMachine* vm, CVMValue self, CVMValue needle);

	static void Bind(CVirtualMachine* machine);

};

#endif