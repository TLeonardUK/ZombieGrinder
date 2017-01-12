/* *****************************************************************

		CVMString.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMSTRING_H_
#define _CVMSTRING_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/VirtualMachine/CVMString.h"

// =================================================================
//	A fast, simple, ref-counted string implementation for the VM.
//  Allocation is based on copy-on-write semantics.
// =================================================================
class CVMString
{
public:
	struct MemoryBuffer
	{
		int		ref_count;
		char*	buffer;
		int		size;
		bool	persistent;
	
	public:
		MemoryBuffer();

		void Retain();
		void Release();

		static MemoryBuffer* Allocate(int size);
	};
	
private:
	MemoryBuffer* m_buffer;

public:
	~CVMString();
	CVMString();
	CVMString(char val, int size);
	CVMString(int value);
	CVMString(unsigned long long value);
	CVMString(float value);
	CVMString(const char* value);	
	CVMString(const char* value, int length);	
	CVMString(const CVMString& other);
	CVMString(CVMString::MemoryBuffer* buffer);
	CVMString(char chr);
	CVMString(std::vector<char> chr);
	
	int Compare(const CVMString& other) const;
	CVMString& operator =(const CVMString& other);
	CVMString operator +(const CVMString& other) const;
	CVMString& operator +=(const CVMString& other);

	char operator [](int index) const;
	bool operator ==(const CVMString& other) const;
	bool operator !=(const CVMString& other) const;
	bool operator <(const CVMString& other) const;
	bool operator >(const CVMString& other) const;
	bool operator <=(const CVMString& other) const;
	bool operator >=(const CVMString& other) const;

	int Length();

	const char* C_Str();
	char* Str(); // Gross

	int To_Int() const;
	bool To_Bool() const;
	float To_Float() const; 

	static CVMString Int_To_Hex(int chr);

	std::vector<CVMString> Split(CVMString seperator, int max_splits, bool remove_duplicates);
	CVMString Join(std::vector<CVMString> fragments);
	CVMString Filter(CVMString allowed_chars, CVMString replacement);

	bool Is_Numeric();
	bool Is_Hex();
	bool Contains(CVMString needle);
	bool Contains_Any(std::vector<CVMString> needles);

	int Index_Of(CVMString needle, int start_index);
	int Index_Of_Any(std::vector<CVMString> needles, int start_index);
	int Last_Index_Of(CVMString needle, int end_index);
	int Last_Index_Of_Any(std::vector<CVMString> needles, int end_index);

	CVMString Replace(int start_index, int length, CVMString replacement);
	CVMString Replace(CVMString what, CVMString with);
	CVMString Insert(CVMString what, int offset);
	CVMString Remove(int start, int length);

	bool Starts_With(CVMString what);
	bool Ends_With(CVMString what);

	CVMString To_Lower();
	CVMString To_Upper();

	CVMString Reverse();
	
	CVMString Limit_End(int length, CVMString postfix);
	CVMString Limit_Start(int length, CVMString prefix);
	
	CVMString Pad_Left(int length, CVMString padding);
	CVMString Pad_Right(int length, CVMString padding);

	CVMString Sub_String(int offset, int count);

	CVMString Trim(CVMString chars);
	CVMString Trim_Start(CVMString chars);
	CVMString Trim_End(CVMString chars);

	int Hex_To_Int();
	
	CVMString Slice(int start_index);
	CVMString Slice(int start_index, int end_index);

};

#endif