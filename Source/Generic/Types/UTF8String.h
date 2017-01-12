// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_UTF8STRING_
#define _GENERIC_UTF8STRING_

#include "Generic/Types/Array.h"

#include <string>
#include <vector>
#include <cstdarg>

// Copy-On-Write based string replacement, all functions
// are utf8 aware. Use other string classes in preference to this
// as they will typically be faster. This is primarily used for 
// visual text-ouput and input.

class UTF8String
{
public:
	struct MemoryBuffer
	{
		int		ref_count;
		char*	buffer;
		int		size;
		bool	char_length_dirty;
		int		char_length;
		bool	persistent;

	public:
		MemoryBuffer();

		void Retain();
		void Release();

		static MemoryBuffer* Allocate(int size);
	};

	struct UTF8Iterator
	{
		const char* buffer;
		int offset;

	public:
		UTF8Iterator(const char* buffer);

		int GetStartByteOffset(int char_index);
		int GetEndByteOffset(int char_index);
		int GetCharSize(const char* data);
		int Length();
		bool Advance(u32& output);

	};

private:
	MemoryBuffer* m_buffer;
	char m_tmp_char_buffer[8];

public:
	~UTF8String();
	UTF8String();
	UTF8String(const char* value);	
	UTF8String(const char* value, int byte_len);	
	UTF8String(const UTF8String& other);
	UTF8String(std::string other);
	UTF8String(UTF8String::MemoryBuffer* buffer);

	int Compare(const UTF8String& other) const;
	UTF8String& operator =(const UTF8String& other);
	UTF8String operator +(const UTF8String& other) const;
	UTF8String operator +(const char* other) const;
	UTF8String operator +(const std::string& other) const;
	UTF8String& operator +=(const UTF8String& other);

	friend UTF8String operator+(const char* a, const UTF8String& b);
	friend UTF8String operator+(const std::string& a, const UTF8String& b);

	const char* operator [](int index);
	
	bool operator ==(const UTF8String& other) const;
	bool operator !=(const UTF8String& other) const;
	bool operator <(const UTF8String& other) const;
	bool operator >(const UTF8String& other) const;
	bool operator <=(const UTF8String& other) const;
	bool operator >=(const UTF8String& other) const;

	u32 GetCharCode(int index) const;

	int Length() const;

	const char* C_Str() const;

	std::vector<UTF8String> Split(UTF8String seperator, int max_splits, bool remove_duplicates);

	int Index_Of(UTF8String needle, int start_index) const;

	UTF8String Replace(int start_index, int length, UTF8String replacement) const;
	UTF8String Replace(UTF8String what, UTF8String with);

	UTF8String Sub_String(int offset, int count = 0x7FFFFFFF) const;

	UTF8String Slice(int start_index) const;
	UTF8String Slice(int start_index, int end_index) const;

};

#endif