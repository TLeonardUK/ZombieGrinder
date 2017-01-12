/* *****************************************************************

		CRuntime_String.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

		***************************************************************** */
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_String.h"

#include "Generic/Helper/StringHelper.h"

#include <cstdlib>
#include <stdio.h>

int ToInt(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.To_Int();
}

float ToFloat(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.To_Float();
}

int ToChar(CVirtualMachine* vm, CVMValue self)	
{
	vm->Assert(self.string_value.Length() > 0);
	return self.string_value[0];
}

int Length(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.Length();
}

CVMString GetIndex(CVirtualMachine* vm, CVMValue self, int index)	
{
	return CVMString(self.string_value[index]);
}

CVMString GetSlice1(CVirtualMachine* vm, CVMValue self, int start_index)	
{
	return self.string_value.Slice(start_index);
}

CVMString GetSlice2(CVirtualMachine* vm, CVMValue self, int start_index, int end_index)	
{
	return self.string_value.Slice(start_index, end_index);
}

int HexToInt(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.Hex_To_Int();
}

CVMString TrimStart(CVirtualMachine* vm, CVMValue self, CVMString chars)	
{
	return self.string_value.Trim_Start(chars);
}

CVMString TrimEnd(CVirtualMachine* vm, CVMValue self, CVMString chars)	
{
	return self.string_value.Trim_End(chars);
}

CVMString Trim(CVirtualMachine* vm, CVMValue self, CVMString chars)	
{
	return self.string_value.Trim(chars);
}

CVMString SubString(CVirtualMachine* vm, CVMValue self, int offset, int count)	
{
	return self.string_value.Sub_String(offset, count);
}

CVMString PadRight(CVirtualMachine* vm, CVMValue self, int length, CVMString padding)	
{
	return self.string_value.Pad_Right(length, padding);
}

CVMString PadLeft(CVirtualMachine* vm, CVMValue self, int length, CVMString padding)	
{
	return self.string_value.Pad_Left(length, padding);
}

CVMString LimitEnd(CVirtualMachine* vm, CVMValue self, int length, CVMString postfix)	
{
	return self.string_value.Limit_End(length, postfix);
}

CVMString LimitStart(CVirtualMachine* vm, CVMValue self, int length, CVMString postfix)	
{
	return self.string_value.Limit_Start(length, postfix);
}

CVMString Reverse(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.Reverse();
}

CVMString ToLower(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.To_Lower();
}

CVMString ToUpper(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.To_Upper();
}

int EndsWith(CVirtualMachine* vm, CVMValue self, CVMString postfix)	
{
	return self.string_value.Ends_With(postfix);
}

int StartsWith(CVirtualMachine* vm, CVMValue self, CVMString prefix)	
{
	return self.string_value.Starts_With(prefix);
}

CVMString Remove(CVirtualMachine* vm, CVMValue self, int start, int length)	
{
	return self.string_value.Remove(start, length);
}

CVMString Insert(CVirtualMachine* vm, CVMValue self, CVMString value, int index)	
{
	return self.string_value.Insert(value, index);
}

CVMString Replace(CVirtualMachine* vm, CVMValue self, CVMString what, CVMString with)	
{
	return self.string_value.Replace(what, with);
}

CVMString ReplaceSection(CVirtualMachine* vm, CVMValue self, int start, int length, CVMString mid)	
{
	return self.string_value.Replace(start, length, mid);
}

int Contains(CVirtualMachine* vm, CVMValue self, CVMString text)	
{
	return self.string_value.Contains(text);
}

int ContainsAny(CVirtualMachine* vm, CVMValue self, CVMObjectHandle texts)	
{
	CVMObject* arr = texts.Get();

	std::vector<CVMString> vals;	
	for (int i = 0; i < arr->Slot_Count(); i++)
	{
		vals.push_back(arr->Get_Slot(i).string_value);
	}

	return self.string_value.Contains_Any(vals);
}

int IndexOf(CVirtualMachine* vm, CVMValue self, CVMString needle, int start_index)	
{
	return self.string_value.Index_Of(needle, start_index);
}

int IndexOfAny(CVirtualMachine* vm, CVMValue self,CVMObjectHandle needles, int start_index)	
{
	CVMObject* arr = needles.Get();

	std::vector<CVMString> vals;	
	for (int i = 0; i < arr->Slot_Count(); i++)
	{
		vals.push_back(arr->Get_Slot(i).string_value);
	}

	return self.string_value.Index_Of_Any(vals, start_index);
}

int LastIndexOf(CVirtualMachine* vm, CVMValue self, CVMString needle, int end_index)	
{
	return self.string_value.Last_Index_Of(needle, end_index);
}

int LastIndexOfAny(CVirtualMachine* vm, CVMValue self, CVMObjectHandle needles, int end_index)	
{
	CVMObject* arr = needles.Get();

	std::vector<CVMString> vals;	
	for (int i = 0; i < arr->Slot_Count(); i++)
	{
		vals.push_back(arr->Get_Slot(i).string_value);
	}

	return self.string_value.Last_Index_Of_Any(vals, end_index);
}

CVMString Filter(CVirtualMachine* vm, CVMValue self, CVMString allowed_chars, CVMString replacement_char)	
{
	return self.string_value.Filter(allowed_chars, replacement_char);
}

int IsHex(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.Is_Hex();
}

int IsNumeric(CVirtualMachine* vm, CVMValue self)	
{
	return self.string_value.Is_Numeric();
}

CVMString Join(CVirtualMachine* vm, CVMValue self, CVMObjectHandle haystack)	
{
	CVMObject* arr = haystack.Get();

	std::vector<CVMString> vals;	
	for (int i = 0; i < arr->Slot_Count(); i++)
	{
		vals.push_back(arr->Get_Slot(i).string_value);
	}

	return self.string_value.Join(vals);
}

CVMObjectHandle Split(CVirtualMachine* vm, CVMValue self, CVMString seperator, int max_splits, int remove_duplicates)	
{
	std::vector<CVMString> splits = self.string_value.Split(seperator, max_splits, remove_duplicates != 0);
	
	CVMLinkedSymbol* string_class = vm->Find_Class("array<string>");

	CVMObjectHandle arr = vm->New_Array(string_class, splits.size());
	for (int i = 0; i < (int)splits.size(); i++)
	{
		arr.Get()->Get_Slot(i).string_value = splits.at(i);
	}

	return arr;
}

CVMString Format(CVirtualMachine* vm, CVMValue self, CVMObjectHandle args)	
{	
	CVMString result = "";
	CVMString format = self.string_value;

	CVMLinkedSymbol* object_class = vm->Find_Class("object");
	CVMLinkedSymbol* to_string_func = vm->Find_Function(object_class, "ToString", 0);

	// Format is very very limited :S. Basically we just call ToString on every
	// item and insert it. Different value specifiers don't actually do shit.

	// Needs fixing when I get the chance. Some kinda proper varidic argument support
	// would be nice.

	// Go through each item.
	int format_count = 0;
	bool in_tag = false;
	char buffer[1024];

	CVMString tag = "";

	for (int i = 0; i < format.Length(); i++)
	{
		char chr = format[i];

		if (chr == '%')
		{
			tag = "%";
			in_tag = true;
		}
		else
		{
			if (in_tag == false)
			{
				result += chr;
			}
			else
			{
				tag += chr;
				if (chr == '%' || (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z'))
				{
					vm->Assert(format_count < args.Get()->Slot_Count());
					CVMValue tag_value = args.Get()->Get_Slot(format_count);

					char type = chr;
					switch (type)
					{
					// Integer
					case 'd':
					case 'i':
					case 'u':
					case 'o':
					case 'x':
					case 'X':
					case 'p':
					case 'P':
					case 'n':
						{
							int int_value = tag_value.object_value.GetNullCheck(vm)->Get_Slot(0).int_value;
							sprintf(buffer, tag.C_Str(), int_value);
							result += buffer;
							break;
						}

					// Float
					case 'f':
					case 'F':
					case 'e':
					case 'E':
					case 'g':
					case 'G':
					case 'A':
					case 'a':
						{
							float float_value = tag_value.object_value.GetNullCheck(vm)->Get_Slot(0).float_value;
							sprintf(buffer, tag.C_Str(), float_value);
							result += buffer;
							break;
						}

					// String
					case 's':
					case 'c':
						{
							vm->Invoke(to_string_func, tag_value, false, false);
							CVMValue retval;
							vm->Get_Return_Value(retval);
							result += retval.string_value;
							break;
						}

					default:
						{
							vm->Assert(false);
						}
					}

					format_count++;
					
					in_tag = false;
					tag = "";
				}
			}
		}
	}

	return result;
}

CVMString IntToHex(CVirtualMachine* vm, int chr)	
{
	return CVMString::Int_To_Hex(chr);
}

CVMString FromChar(CVirtualMachine* vm, int chr)	
{
	return CVMString((char*)&chr, 1);
}

CVMString FromChars(CVirtualMachine* vm, CVMObjectHandle chars)	
{
	CVMObject* arr = chars.Get();

	std::vector<char> vals;	
	for (int i = 0; i < arr->Slot_Count(); i++)
	{
		vals.push_back(arr->Get_Slot(i).int_value);
	}

	return CVMString(vals);
}


CVMString FormatNumber(CVirtualMachine* vm, float val)
{
	std::string v = StringHelper::Format_Number(val);
	return CVMString(v.c_str());
}


void CRuntime_String::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<int>										("String", "ToInt",				&ToInt);
	vm->Get_Bindings()->Bind_Method<float>										("String", "ToFloat",				&ToFloat);
	vm->Get_Bindings()->Bind_Method<int>										("String", "ToChar",				&ToChar);
	vm->Get_Bindings()->Bind_Method<int>										("String", "Length",				&Length);			
	vm->Get_Bindings()->Bind_Method<CVMString, int>								("String", "GetIndex",				&GetIndex);
	vm->Get_Bindings()->Bind_Method<CVMString, int>								("String", "GetSlice1",			&GetSlice1);
	vm->Get_Bindings()->Bind_Method<CVMString, int, int>						("String", "GetSlice2",			&GetSlice2);
	vm->Get_Bindings()->Bind_Method<int>										("String", "HexToInt",				&HexToInt);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString>						("String", "TrimStart",			&TrimStart);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString>						("String", "TrimEnd",				&TrimEnd);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString>						("String", "Trim",					&Trim);
	vm->Get_Bindings()->Bind_Method<CVMString,int,int>							("String", "SubString",			&SubString);
	vm->Get_Bindings()->Bind_Method<CVMString,int,CVMString>					("String", "PadRight",				&PadRight);
	vm->Get_Bindings()->Bind_Method<CVMString,int,CVMString>					("String", "PadLeft",				&PadLeft);
	vm->Get_Bindings()->Bind_Method<CVMString,int,CVMString>					("String", "LimitEnd",				&LimitEnd);
	vm->Get_Bindings()->Bind_Method<CVMString,int,CVMString>					("String", "LimitStart",			&LimitStart);
	vm->Get_Bindings()->Bind_Method<CVMString>									("String", "Reverse",				&Reverse);
	vm->Get_Bindings()->Bind_Method<CVMString>									("String", "ToLower",				&ToLower);
	vm->Get_Bindings()->Bind_Method<CVMString>									("String", "ToUpper",				&ToUpper);
	vm->Get_Bindings()->Bind_Method<int,CVMString>								("String", "EndsWith",				&EndsWith);
	vm->Get_Bindings()->Bind_Method<int,CVMString>								("String", "StartsWith",			&StartsWith);
	vm->Get_Bindings()->Bind_Method<CVMString,int,int>							("String", "Remove",				&Remove);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString,int>					("String", "Insert",				&Insert);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString,CVMString>				("String", "Replace",				&Replace);
	vm->Get_Bindings()->Bind_Method<CVMString,int,int,CVMString>				("String", "ReplaceSection",		&ReplaceSection);
	vm->Get_Bindings()->Bind_Method<int,CVMString>								("String", "Contains",				&Contains);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle>						("String", "ContainsAny",			&ContainsAny);
	vm->Get_Bindings()->Bind_Method<int,CVMString,int>							("String", "IndexOf",				&IndexOf);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle, int>					("String", "IndexOfAny",			&IndexOfAny);
	vm->Get_Bindings()->Bind_Method<int,CVMString,int>							("String", "LastIndexOf",			&LastIndexOf);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle, int>					("String", "LastIndexOfAny",		&LastIndexOfAny);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMString, CVMString>				("String", "Filter",				&Filter);
	vm->Get_Bindings()->Bind_Method<int>										("String", "IsHex",				&IsHex);
	vm->Get_Bindings()->Bind_Method<int>										("String", "IsNumeric",			&IsNumeric);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMObjectHandle>					("String", "Join",					&Join);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMString,int,int>			("String", "Split",				&Split);
	vm->Get_Bindings()->Bind_Method<CVMString,CVMObjectHandle>					("String", "Format",				&Format);

	vm->Get_Bindings()->Bind_Function<CVMString,int>							("String", "IntToHex",				&IntToHex);
	vm->Get_Bindings()->Bind_Function<CVMString,int>							("String", "FromChar",				&FromChar);
	vm->Get_Bindings()->Bind_Function<CVMString,CVMObjectHandle>				("String", "FromChars",			&FromChars);
	vm->Get_Bindings()->Bind_Function<CVMString, float>("String", "FormatNumber", &FormatNumber);
}