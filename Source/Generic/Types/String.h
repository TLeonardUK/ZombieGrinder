// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_STRING_
#define _GENERIC_STRING_

#include "Generic/Types/Array.h"

#include <string>
#include <vector>
#include <cstdarg>

// Copy-On-Write based std::string replacement. Be aware
// this is not multithread safe!

class String
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
	~String();
	String();
	String(char val, int size);
	String(int value);
	String(float value);
	String(const char* value);	
	String(const char* value, int length);	
	String(const String& other);
	String(String::MemoryBuffer* buffer);
	String(char chr);
	String(std::string val);
	String(std::vector<char> chr);

	int Compare(const String& other) const;
	String& operator =(const String& other);
	String operator +(const String& other) const;
	String operator +(const char* other) const;
	String& operator +=(const String& other);

	friend String operator+(const char* a, const String& b);
	friend String operator+(const std::string& a, const String& b);
	//friend String operator+(const String& a, const char* b);

	char operator [](int index) const;
	bool operator ==(const String& other) const;
	bool operator !=(const String& other) const;
	bool operator <(const String& other) const;
	bool operator >(const String& other) const;
	bool operator <=(const String& other) const;
	bool operator >=(const String& other) const;

	int Length() const;

	const char* C_Str() const;
	char* Str(); // Gross

	int To_Int() const;
	bool To_Bool() const;
	float To_Float() const; 

	static String Int_To_Hex(int chr);

	std::vector<String> Split(String seperator, int max_splits, bool remove_duplicates);
	String Join(std::vector<String> fragments);
	String Filter(String allowed_chars, String replacement);

	bool Is_Numeric();
	bool Is_Hex();
	bool Contains(String needle);
	bool Contains_Any(std::vector<String> needles);

	int Index_Of(String needle, int start_index) const;
	int Index_Of_Any(std::vector<String> needles, int start_index) const;
	int Last_Index_Of(String needle, int end_index) const;
	int Last_Index_Of_Any(std::vector<String> needles, int end_index) const;

	String Replace(int start_index, int length, String replacement) const;
	String Replace(String what, String with);
	String Insert(String what, int offset);
	String Remove(int start, int length);

	bool Starts_With(String what);
	bool Ends_With(String what);

	String To_Lower();
	String To_Upper();

	String Reverse();

	String Limit_End(int length, String postfix);
	String Limit_Start(int length, String prefix);

	String Pad_Left(int length, String padding);
	String Pad_Right(int length, String padding);

	String Sub_String(int offset, int count = 0x7FFFFFFF) const;

	String Trim(String chars);
	String Trim_Start(String chars);
	String Trim_End(String chars);

	int Hex_To_Int();

	String Slice(int start_index) const;
	String Slice(int start_index, int end_index) const;




	// --------------------------------------------------------------------------------------
	// Wrapper functions for std::string compatibility.
	// Would be far better to remove this shit.

	const char* c_str() const			 { return C_Str(); } 
	unsigned int size() const			 { return Length(); }
	unsigned int length() const			 { return Length(); } 
	char at(int index) const			 { return (*this)[index]; }
	char at(unsigned int index) const	 { return (*this)[index]; }
	
	enum
	{
		npos = -1
	};

	int find(String needle, size_t offset = 0) const
	{
		return Index_Of(needle, offset);
	}

	int find(const char* needle, size_t offset = 0) const
	{
		return Index_Of(needle, offset);
	}

	int find(const char needle, size_t offset = 0) const
	{
		return Index_Of(needle, offset);
	}

	int find_last_of(const char* needle, size_t offset = -1) const
	{
		std::vector<String> needles;
		for (int i = 0; needle[i] != '\0'; i++)
		{
			needles.push_back(needle[i]);
		}
		return Last_Index_Of_Any(needles, offset);
	}

	int find_last_of(const char needle, size_t offset = -1) const
	{
		return Last_Index_Of(needle, offset);
	}

	String substr(int pos, int len = -1) const
	{
		return Sub_String(pos, len == -1 ? Length() - pos : len);
	}

	String replace(int pos, int len, String replacement) const
	{
		return Replace(pos, len, replacement);
	}

	// --------------------------------------------------------------------------------------

};

#endif