// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/String.h"
#include "Generic/Math/Math.h"

#include <sstream>
#include <cstring>
#include <stdio.h>
#include <cstdlib>

struct Global_Real_Strings
{
	String::MemoryBuffer* g_empty_buffer;

	Global_Real_Strings()
	{
		g_empty_buffer = new String::MemoryBuffer();
		g_empty_buffer->buffer = new char[1];
		g_empty_buffer->buffer[0] = '\0';
		g_empty_buffer->size = 0;
		g_empty_buffer->ref_count = 1;
		g_empty_buffer->persistent = true;
	}
};

Global_Real_Strings g_real_global_strings;

String::MemoryBuffer::MemoryBuffer()
	: ref_count(1)
	, size(0)
	, buffer(NULL)
	, persistent(false)
{
}

void String::MemoryBuffer::Retain()
{
	ref_count++;
}

void String::MemoryBuffer::Release()
{
	ref_count--;

	if (ref_count <= 0)
	{
		if (persistent == false)
		{
			delete[] buffer;
			delete this;
		}
	}
}

String::MemoryBuffer* String::MemoryBuffer::Allocate(int size)
{
	if (size == 0)
	{
		return g_real_global_strings.g_empty_buffer;
	}

	MemoryBuffer* buffer = new MemoryBuffer();
	buffer->buffer = new char[size + 1];
	buffer->buffer[size] = '\0';
	buffer->size = size;
	buffer->ref_count = 1;

	return buffer;
}

String::~String()
{
	m_buffer->Release();
}

String::String()
{
	m_buffer = g_real_global_strings.g_empty_buffer;
}

const char* String::C_Str() const
{
	return m_buffer->buffer;
}

char* String::Str()
{
	return m_buffer->buffer;
}

int String::Length() const
{
	return m_buffer->size;
}

String::String(char val, int size)
{
	m_buffer = MemoryBuffer::Allocate(size);
	memset(m_buffer->buffer, val, size);
}

String::String(int value)
{
	char buffer[128];
	sprintf(buffer, "%i", value);
	
	int length = strlen(buffer);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, buffer, length);
}

String::String(float value)
{
	char buffer[128];
	sprintf(buffer, "%f", value);
	
	int length = strlen(buffer);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, buffer, length);
}

String::String(const char* value)
{
	int length = strlen(value);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, value, length);
}

String::String(std::string val)
{
	unsigned int length = val.size();
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, val.c_str(), length);
}

String::String(const char* value, int length)
{
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, value, length);
}

String::String(String::MemoryBuffer* buffer)
{
	m_buffer = buffer;
}

String::String(const String& other)
{
	m_buffer = other.m_buffer;
	m_buffer->Retain();
}	

String::String(char chr)
{
	m_buffer = MemoryBuffer::Allocate(1);
	m_buffer->buffer[0] = chr;
	m_buffer->buffer[1] = '\0';
}

String::String(std::vector<char> chr)
{
	m_buffer = MemoryBuffer::Allocate(chr.size());
	for (unsigned int i = 0; i < chr.size(); i++)
	{
		m_buffer->buffer[i] = chr.at(i);
	}
	m_buffer->buffer[chr.size()] = '\0';
}

int String::Compare(const String& other) const
{
	int min_size = (other.m_buffer->size < m_buffer->size ? 
					other.m_buffer->size : 
					m_buffer->size);

	for (int i = 0; i < min_size; i++)
	{
		int diff = (other.m_buffer->buffer[i] - m_buffer->buffer[i]);
		if (diff != 0)
		{
			return diff;
		}
	}

	return (other.m_buffer->size - m_buffer->size);
}

String& String::operator =(const String& other)
{
	other.m_buffer->Retain();
	m_buffer->Release();
	m_buffer = other.m_buffer;
	return *this;
}

String String::operator +(const String& other) const
{
	if (m_buffer->size == 0)
	{
		return other;
	}
	if (other.m_buffer->size == 0)
	{
		return *this;
	}

	MemoryBuffer* newBuffer = MemoryBuffer::Allocate(m_buffer->size + other.m_buffer->size);

	memcpy(newBuffer->buffer, 
			m_buffer->buffer, 
			m_buffer->size);

	memcpy(newBuffer->buffer + m_buffer->size, 
			other.m_buffer->buffer, 
			other.m_buffer->size);

	return String(newBuffer);
}

String String::operator +(const char* other) const
{
	int other_len = strlen(other);

	if (m_buffer->size == 0)
	{
		return other;
	}
	if (other_len == 0)
	{
		return *this;
	}

	MemoryBuffer* newBuffer = MemoryBuffer::Allocate(m_buffer->size + other_len);

	memcpy(newBuffer->buffer, 
		m_buffer->buffer, 
		m_buffer->size);

	memcpy(newBuffer->buffer + m_buffer->size, 
		other, 
		other_len);

	return String(newBuffer);
}

String& String::operator +=(const String& other)
{
	return operator =(*this + other);
}

String operator+(const char* a, const String& b)
{
	return String(a) + b;
}

String operator+(const std::string& a, const String& b)
{
	return String(a) + b;
}

/*
String operator+(const String& a, const char* b)
{
	return a + String(b);
}
*/
char String::operator [](int index) const
{
	DBG_ASSERT(index >= 0 && index < m_buffer->size);
	return m_buffer->buffer[index];
}

bool String::operator ==(const String& other) const
{
	if (m_buffer->size != other.m_buffer->size)
	{
		return false;
	}
	return (memcmp(m_buffer->buffer, other.m_buffer->buffer, m_buffer->size) == 0);
}

bool String::operator !=(const String& other) const
{
	return !(operator ==(other));
}

bool String::operator <(const String& other) const
{
	return Compare(other) < 0;
}

bool String::operator >(const String& other) const
{
	return Compare(other) > 0;
}

bool String::operator <=(const String& other) const
{
	return Compare(other) <= 0;
}

bool String::operator >=(const String& other) const
{
	return Compare(other) >= 0;
}

int String::To_Int() const
{
	return atoi(m_buffer->buffer);
}

bool String::To_Bool() const
{
	return m_buffer->size == 0 || !(strcmp(m_buffer->buffer, "false") == 0 || strcmp(m_buffer->buffer, "0") == 0);
}

float String::To_Float() const
{
	return (float)atof(m_buffer->buffer);
}

String String::Int_To_Hex(int chr)
{
	std::stringstream sstream;
	sstream << std::hex << chr;
	return String(sstream.str().c_str());
}

std::vector<String> String::Split(String seperator, int max_splits, bool remove_duplicates)
{
	std::vector<String> result;
	String split = "";
		
	if (seperator == "")
	{
		result.push_back(*this);
		return result;
	}
		
	for (int i = 0; i < Length(); i++)
	{
		String res = Slice(i, i + 1);
		if (i <= Length() - seperator.Length())
		{
			res = Slice(i, i + seperator.Length());
		}
			
		if (res == seperator && ((int)result.size() < max_splits || max_splits <= 0))
		{
			if (split != "" || remove_duplicates == false)
			{
				result.push_back(split);
				split = "";
			}
			i += (seperator.Length() - 1);
			continue;
		}
		else
		{
			split += Slice(i, i+1);
		}		
	}
		
	if (split != "" || remove_duplicates == false)
	{
		result.push_back(split);
		split = "";
	}
		
	return result;
}

String String::Join(std::vector<String> fragments)
{
	String result = "";
	for (std::vector<String>::iterator iter = fragments.begin(); iter != fragments.end(); iter++)
	{
		if (result != "")
		{
			result += (*this);
		}
		result += *iter;
	}
	return result;
}

String String::Filter(String allowed_chars, String replacement)
{
	String result = "";
	
	for (int i = 0; i < Length(); i++)
	{
		char chr = (*this)[i];
		bool found = false;
			
		for (int j = 0; j < allowed_chars.Length(); j++)
		{
			char chr2 = allowed_chars[j];
			if (chr == chr2)
			{
				found = true;
				break;
			}
		}
			
		if (found == true)
		{
			result += chr;
		}
		else
		{
			result += replacement;
		}
	}

	return result;
}

bool String::Is_Numeric()
{
	for (int i = 0; i < Length(); i++)
	{
		char chr = m_buffer->buffer[i];
		bool is_num = (chr >= '0' && chr <= '9');
		if (!is_num)
		{
			return false;
		}
	}
	return true;
}

bool String::Is_Hex()
{
	for (int i = 0; i < Length(); i++)
	{
		char chr = m_buffer->buffer[i];
		bool is_hex = (chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'F') || (chr >= 'a' && chr <= 'f');
		if (!is_hex)
		{
			return false;
		}
	}
	return true;
}

bool String::Contains(String needle)
{
	return (Index_Of(needle, 0) >= 0);
}

bool String::Contains_Any(std::vector<String> needles)
{
	return (Index_Of_Any(needles, 0) >= 0);
}

int String::Index_Of(String needle, int start_index) const
{
	String result = "";	
	if (needle == "")
	{
		return -1;
	}
		
	for (int i = start_index; i < (Length() - needle.Length()) + 1; i++)
	{
		String res = Slice(i, i + needle.Length());			
		if (res == needle)
		{
			return i;
		}
	}
		
	return -1;	
}

int String::Index_Of_Any(std::vector<String> needles, int start_index) const
{
	String result = "";	
	if (needles.size() == 0)
	{
		return -1;
	}
		
	for (int i = start_index; i < Length(); i++)
	{
		for (std::vector<String>::iterator iter = needles.begin(); iter != needles.end(); iter++)
		{
			String& needle = *iter;
			String res = Slice(i, i + needle.Length());			
			if (res == needle)
			{
				return i;
			}
		}
	}
		
	return -1;	
}

int String::Last_Index_Of(String needle, int end_index) const
{
	String result = "";	
	if (needle == "")
	{
		return -1;
	}
		
	if (end_index == -1)
	{
		end_index = Length() - 1;
	}
		
	for (int i = end_index; i >= 0; i--)
	{
		String res = Slice(i, i + needle.Length());			
		if (res == needle)
		{
			return i;
		}
	}
		
	return -1;	
}

int String::Last_Index_Of_Any(std::vector<String> needles, int end_index) const
{
	String result = "";	
	if (needles.size() == 0)
	{
		return -1;
	}
		
	if (end_index == -1)
	{
		end_index = Length() - 1;
	}
		
	for (int i = end_index; i >= 0; i--)
	{
		for (std::vector<String>::iterator iter = needles.begin(); iter != needles.end(); iter++)
		{
			String& needle = *iter;
			String res = Slice(i, i + needle.Length());			

			if (res == needle)
			{
				return i;
			}
		}
	}
		
	return -1;	
}

String String::Replace(int start_index, int length, String replacement) const
{
	return Slice(0, start_index) + replacement + Slice(start_index + length);
}

String String::Replace(String what, String with)
{
	String result = "";
	
	if (what == "")
	{
		return *this;
	}
		
	for (int i = 0; i < Length(); i++)
	{
		String res = Slice(i, i + 1);
		if (i <= Length() - what.Length())
		{
			res = Slice(i, i + what.Length());
		}
			
		if (res == what)
		{
			result += with;
			i += (what.Length() - 1);
			continue;
		}
		else
		{
			result += Slice(i, i + 1);
		}		
	}
		
	return result;		
}

String String::Insert(String what, int offset)
{
	return Slice(0, offset) + what + Slice(offset);
}

String String::Remove(int start, int length)
{
	return Slice(0, start) + Slice(start + length);
}

bool String::Starts_With(String what)
{
	return (Slice(0, what.Length()) == what);
}

bool String::Ends_With(String what)
{
	return (Slice(-what.Length()) == what);
}

String String::To_Lower()
{
	String result = "";
	const char* buffer = m_buffer->buffer;
	while (buffer[0] != '\0')
	{
		result += (char)::tolower(buffer[0]);
		buffer++;
	}
	return result;
}

String String::To_Upper()
{
	String result = "";
	const char* buffer = m_buffer->buffer;
	while (buffer[0] != '\0')
	{
		result += (char)::toupper(buffer[0]);
		buffer++;
	}
	return result;
}

String String::Reverse()
{
	String result = "";
	const char* buffer = m_buffer->buffer + m_buffer->size;
	while (buffer >= m_buffer->buffer + 1)
	{
		result += buffer[-1];
		buffer--;
	}
	return result;
}
	
String String::Limit_End(int length, String postfix)
{
	if (length <= postfix.Length())
	{
		return postfix.Slice(0, length);
	}
	else
	{
		return Slice(0, length - postfix.Length()) + postfix;
	}
}

String String::Limit_Start(int length, String postfix)
{
	if (length <= postfix.Length())
	{
		return postfix.Slice(0, length);
	}
	else
	{
		int cutLength = (Length() - length) + postfix.Length();
		return postfix + Slice(cutLength);
	}
}
	
String String::Pad_Left(int length, String padding)
{
	String result = *this;

	for (int offset = 0; result.Length() < length; offset++)
	{
		result = String(padding[(offset++) % padding.Length()]) + result;
	}		

	return result;
}

String String::Pad_Right(int length, String padding)
{
	String result = *this;

	for (int offset = 0; result.Length() < length; offset++)
	{
		result += padding[(offset++) % padding.Length()];
	}		

	return result;
}

String String::Sub_String(int offset, int count) const
{
//	DBG_ASSERT(offset >= 0 && offset + count < m_buffer->size);
	DBG_ASSERT(offset >= 0);
	if (count == 0x7FFFFFFF)
	{
		count = Length();
	}
	int excess = Min(0, Length() - (offset + count));
	return String(m_buffer->buffer + offset, count + excess);
}

String String::Trim(String chars)
{
	return Trim_Start(chars).Trim_End(chars);
}

String String::Trim_Start(String chars)
{
	char* result_buffer = m_buffer->buffer;
	int result_length = Length();

	while (result_length > 0)
	{
		bool should_trim = false;

		for (int i = 0; i < chars.Length(); i++)
		{
			if (chars[i] == result_buffer[0])
			{
				should_trim = true;
				break;
			}
		}

		if (should_trim == false)
			break;

		result_buffer++;
		result_length--;
	}

	return String(result_buffer, result_length);
}

String String::Trim_End(String chars)
{
	int result_length = Length();
	char* result_buffer = m_buffer->buffer + result_length;

	while (result_length > 0)
	{
		bool should_trim = false;

		for (int i = 0; i < chars.Length(); i++)
		{
			if (chars[i] == result_buffer[-1])
			{
				should_trim = true;
				break;
			}
		}

		if (should_trim == false)
			break;

		result_buffer--;
		result_length--;
	}

	return String(m_buffer->buffer, result_length);
}

int String::Hex_To_Int()
{
	return strtol(C_Str(), NULL, 16);
}

String String::Slice(int start_index) const
{
	return Slice(start_index, Length());
}

String String::Slice(int start_index, int end_index) const
{
	int length = Length();

	if (start_index < 0)
	{
		start_index += length;
		if (start_index < 0)
		{
			start_index = 0;
		}		
	}
	else if (start_index > length)
	{
		start_index = length;
	}
	
	if (end_index < 0)
	{
		end_index += length;
	}
	else if (end_index > length)
	{
		end_index = length;
	}

	if (start_index >= end_index)
	{
		return "";
	}
	else if (start_index == 0 && end_index == length)
	{
		return *this;
	}
	else
	{
		return String(m_buffer->buffer + start_index, end_index - start_index);
	}
}

