/* *****************************************************************

		CVMString.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CVMString.h"

#include "Generic/Threads/Atomic.h"

#include <sstream>
#include <cstring>
#include <cstdlib>
#include <stdio.h>

struct Global_Strings
{
	CVMString::MemoryBuffer* g_empty_buffer;

	Global_Strings()
	{
		g_empty_buffer = new CVMString::MemoryBuffer();
		g_empty_buffer->buffer = new char[1];
		g_empty_buffer->buffer[0] = '\0';
		g_empty_buffer->size = 0;
		g_empty_buffer->ref_count = 1;
		g_empty_buffer->persistent = true;
	}
};

Global_Strings g_global_strings;

CVMString::MemoryBuffer::MemoryBuffer()
	: ref_count(1)
	, size(0)
	, buffer(NULL)
	, persistent(false)
{
}

void CVMString::MemoryBuffer::Retain()
{
#ifdef VM_ALLOW_MULTITHREADING
	Atomic::Increment32(&ref_count);
#else
	ref_count++;
#endif
}

void CVMString::MemoryBuffer::Release()
{
#ifdef VM_ALLOW_MULTITHREADING
	if (Atomic::Decrement32(&ref_count) == 0)
#else
	if ((--ref_count) == 0)
#endif
	{
		if (persistent == false)
		{
			delete[] buffer;
			delete this;
		}
	}
}

CVMString::MemoryBuffer* CVMString::MemoryBuffer::Allocate(int size)
{
	if (size == 0)
	{
		return g_global_strings.g_empty_buffer;
	}

	MemoryBuffer* buffer = new MemoryBuffer();
	buffer->buffer = new char[size + 1];
	buffer->buffer[size] = '\0';
	buffer->size = size;
	buffer->ref_count = 1;

	return buffer;
}

CVMString::~CVMString()
{
	m_buffer->Release();
}

CVMString::CVMString()
{
	m_buffer = g_global_strings.g_empty_buffer;
}

const char* CVMString::C_Str()
{
	return m_buffer->buffer;
}

char* CVMString::Str()
{
	return m_buffer->buffer;
}

int CVMString::Length()
{
	return m_buffer->size;
}

CVMString::CVMString(char val, int size)
{
	m_buffer = MemoryBuffer::Allocate(size);
	memset(m_buffer->buffer, val, size);
}

CVMString::CVMString(int value)
{
	char buffer[128];
	sprintf(buffer, "%i", value);
	
	int length = strlen(buffer);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, buffer, length);
}

CVMString::CVMString(unsigned long long value)
{
	char buffer[128];
	sprintf(buffer, "%llu", value);

	int length = strlen(buffer);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, buffer, length);
}

CVMString::CVMString(float value)
{
	char buffer[128];
	sprintf(buffer, "%f", value);
	
	int length = strlen(buffer);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, buffer, length);
}

CVMString::CVMString(const char* value)
{
	int length = strlen(value);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, value, length);
}

CVMString::CVMString(const char* value, int length)
{
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, value, length);
}

CVMString::CVMString(CVMString::MemoryBuffer* buffer)
{
	m_buffer = buffer;
}

CVMString::CVMString(const CVMString& other)
{
	m_buffer = other.m_buffer;
	m_buffer->Retain();
}	

CVMString::CVMString(char chr)
{
	m_buffer = MemoryBuffer::Allocate(1);
	m_buffer->buffer[0] = chr;
	m_buffer->buffer[1] = '\0';
}

CVMString::CVMString(std::vector<char> chr)
{
	m_buffer = MemoryBuffer::Allocate(chr.size());
	for (unsigned int i = 0; i < chr.size(); i++)
	{
		m_buffer->buffer[i] = chr.at(i);
	}
	m_buffer->buffer[chr.size()] = '\0';
}

int CVMString::Compare(const CVMString& other) const
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

CVMString& CVMString::operator =(const CVMString& other)
{
	other.m_buffer->Retain();
	m_buffer->Release();
	m_buffer = other.m_buffer;
	return *this;
}

CVMString CVMString::operator +(const CVMString& other) const
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

	return CVMString(newBuffer);
}

CVMString& CVMString::operator +=(const CVMString& other)
{
	return operator =(*this + other);
}

char CVMString::operator [](int index) const
{
	DBG_ASSERT(index >= 0 && index < m_buffer->size);
	return m_buffer->buffer[index];
}

bool CVMString::operator ==(const CVMString& other) const
{
	if (m_buffer->size != other.m_buffer->size)
	{
		return false;
	}
	return (memcmp(m_buffer->buffer, other.m_buffer->buffer, m_buffer->size) == 0);
}

bool CVMString::operator !=(const CVMString& other) const
{
	return !(operator ==(other));
}

bool CVMString::operator <(const CVMString& other) const
{
	return Compare(other) < 0;
}

bool CVMString::operator >(const CVMString& other) const
{
	return Compare(other) > 0;
}

bool CVMString::operator <=(const CVMString& other) const
{
	return Compare(other) <= 0;
}

bool CVMString::operator >=(const CVMString& other) const
{
	return Compare(other) >= 0;
}

int CVMString::To_Int() const
{
	return atoi(m_buffer->buffer);
}

bool CVMString::To_Bool() const
{
	return m_buffer->size == 0 || !(strcmp(m_buffer->buffer, "false") == 0 || strcmp(m_buffer->buffer, "0") == 0);
}

float CVMString::To_Float() const
{
	return (float)atof(m_buffer->buffer);
}

CVMString CVMString::Int_To_Hex(int chr)
{
	std::stringstream sstream;
	sstream << std::hex << chr;
	return CVMString(sstream.str().c_str());
}

std::vector<CVMString> CVMString::Split(CVMString seperator, int max_splits, bool remove_duplicates)
{
	std::vector<CVMString> result;
	CVMString split = "";
		
	if (seperator == "")
	{
		result.push_back(*this);
		return result;
	}
		
	for (int i = 0; i < Length(); i++)
	{
		CVMString res = Slice(i, i + 1);
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

CVMString CVMString::Join(std::vector<CVMString> fragments)
{
	CVMString result = "";
	for (std::vector<CVMString>::iterator iter = fragments.begin(); iter != fragments.end(); iter++)
	{
		if (result != "")
		{
			result += (*this);
		}
		result += *iter;
	}
	return result;
}

CVMString CVMString::Filter(CVMString allowed_chars, CVMString replacement)
{
	CVMString result = "";
	
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

bool CVMString::Is_Numeric()
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

bool CVMString::Is_Hex()
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

bool CVMString::Contains(CVMString needle)
{
	return (Index_Of(needle, 0) >= 0);
}

bool CVMString::Contains_Any(std::vector<CVMString> needles)
{
	return (Index_Of_Any(needles, 0) >= 0);
}

int CVMString::Index_Of(CVMString needle, int start_index)
{
	CVMString result = "";	
	if (needle == "")
	{
		return -1;
	}
		
	for (int i = start_index; i < Length() - needle.Length(); i++)
	{
		CVMString res = Slice(i, i + needle.Length());			
		if (res == needle)
		{
			return i;
		}
	}
		
	return -1;	
}

int CVMString::Index_Of_Any(std::vector<CVMString> needles, int start_index)
{
	CVMString result = "";	
	if (needles.size() == 0)
	{
		return -1;
	}
		
	for (int i = start_index; i < Length(); i++)
	{
		for (std::vector<CVMString>::iterator iter = needles.begin(); iter != needles.end(); iter++)
		{
			CVMString& needle = *iter;
			CVMString res = Slice(i, i + needle.Length());			
			if (res == needle)
			{
				return i;
			}
		}
	}
		
	return -1;	
}

int CVMString::Last_Index_Of(CVMString needle, int end_index)
{
	CVMString result = "";	
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
		CVMString res = Slice(i, i + needle.Length());			
		if (res == needle)
		{
			return i;
		}
	}
		
	return -1;	
}

int CVMString::Last_Index_Of_Any(std::vector<CVMString> needles, int end_index)
{
	CVMString result = "";	
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
		for (std::vector<CVMString>::iterator iter = needles.begin(); iter != needles.end(); iter++)
		{
			CVMString& needle = *iter;
			CVMString res = Slice(i, i + needle.Length());			

			if (res == needle)
			{
				return i;
			}
		}
	}
		
	return -1;	
}

CVMString CVMString::Replace(int start_index, int length, CVMString replacement)
{
	return Slice(0, start_index) + replacement + Slice(start_index + length);
}

CVMString CVMString::Replace(CVMString what, CVMString with)
{
	CVMString result = "";
	
	if (what == "")
	{
		return *this;
	}
		
	for (int i = 0; i < Length(); i++)
	{
		CVMString res = Slice(i, i + 1);
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

CVMString CVMString::Insert(CVMString what, int offset)
{
	return Slice(0, offset) + what + Slice(offset);
}

CVMString CVMString::Remove(int start, int length)
{
	return Slice(0, start) + Slice(start + length);
}

bool CVMString::Starts_With(CVMString what)
{
	return (Slice(0, what.Length()) == what);
}

bool CVMString::Ends_With(CVMString what)
{
	return (Slice(-what.Length()) == what);
}

CVMString CVMString::To_Lower()
{
	CVMString result = "";
	const char* buffer = m_buffer->buffer;
	while (buffer[0] != '\0')
	{
		result += (char)::tolower(buffer[0]);
		buffer++;
	}
	return result;
}

CVMString CVMString::To_Upper()
{
	CVMString result = "";
	const char* buffer = m_buffer->buffer;
	while (buffer[0] != '\0')
	{
		result += (char)::toupper(buffer[0]);
		buffer++;
	}
	return result;
}

CVMString CVMString::Reverse()
{
	CVMString result = "";
	const char* buffer = m_buffer->buffer + m_buffer->size;
	while (buffer >= m_buffer->buffer + 1)
	{
		result += buffer[-1];
		buffer--;
	}
	return result;
}
	
CVMString CVMString::Limit_End(int length, CVMString postfix)
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

CVMString CVMString::Limit_Start(int length, CVMString postfix)
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
	
CVMString CVMString::Pad_Left(int length, CVMString padding)
{
	CVMString result = *this;

	for (int offset = 0; result.Length() < length; offset++)
	{
		result = CVMString(padding[(offset++) % padding.Length()]) + result;
	}		

	return result;
}

CVMString CVMString::Pad_Right(int length, CVMString padding)
{
	CVMString result = *this;

	for (int offset = 0; result.Length() < length; offset++)
	{
		result += padding[(offset++) % padding.Length()];
	}		

	return result;
}

CVMString CVMString::Sub_String(int offset, int count)
{
	DBG_ASSERT(offset >= 0 && offset + count < m_buffer->size);
	return CVMString(m_buffer->buffer + offset, count);
}

CVMString CVMString::Trim(CVMString chars)
{
	return Trim_Start(chars).Trim_End(chars);
}

CVMString CVMString::Trim_Start(CVMString chars)
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

	return CVMString(result_buffer, result_length);
}

CVMString CVMString::Trim_End(CVMString chars)
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

	return CVMString(m_buffer->buffer, result_length);
}

int CVMString::Hex_To_Int()
{
	return strtol(C_Str(), NULL, 16);
}

CVMString CVMString::Slice(int start_index)
{
	return Slice(start_index, Length());
}

CVMString CVMString::Slice(int start_index, int end_index)
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
		return CVMString(m_buffer->buffer + start_index, end_index - start_index);
	}
}
