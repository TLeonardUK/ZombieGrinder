// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Types/UTF8String.h"
#include "Generic/Math/Math.h"

#include <sstream>
#include <cstring>
#include <stdio.h>
#include <cstdlib>

// The actual smart thing to do here is to internally store as u32's and convert to/from utf8 when requested
// but I'm being lazy right now. Fix plz.

struct Global_Real_UTF8Strings
{
	UTF8String::MemoryBuffer* g_empty_buffer;

	Global_Real_UTF8Strings()
	{
		g_empty_buffer = new UTF8String::MemoryBuffer();
		g_empty_buffer->buffer = new char[1];
		g_empty_buffer->buffer[0] = '\0';
		g_empty_buffer->size = 0;
		g_empty_buffer->ref_count = 1;
		g_empty_buffer->persistent = true;
		g_empty_buffer->char_length = 0;
		g_empty_buffer->char_length_dirty = false;
	}
};

Global_Real_UTF8Strings g_real_global_utf8_strings;

UTF8String::MemoryBuffer::MemoryBuffer()
	: ref_count(1)
	, size(0)
	, buffer(NULL)
	, persistent(false)
	, char_length_dirty(true)
{
}

void UTF8String::MemoryBuffer::Retain()
{
	ref_count++;
}

void UTF8String::MemoryBuffer::Release()
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

UTF8String::MemoryBuffer* UTF8String::MemoryBuffer::Allocate(int size)
{
	if (size == 0)
	{
		return g_real_global_utf8_strings.g_empty_buffer;
	}

	MemoryBuffer* buffer = new MemoryBuffer();
	buffer->buffer = new char[size + 1];
	buffer->buffer[size] = '\0';
	buffer->size = size;
	buffer->ref_count = 1;
	buffer->char_length = 0;
	buffer->char_length_dirty = true;

	return buffer;
}

UTF8String::~UTF8String()
{
	m_buffer->Release();
}

UTF8String::UTF8String()
{
	m_buffer = g_real_global_utf8_strings.g_empty_buffer;
}

const char* UTF8String::C_Str() const
{
	return m_buffer->buffer;
}

UTF8String::UTF8String(const char* value)
{
	int length = strlen(value);
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, value, length);
}

UTF8String::UTF8String(std::string val)
{
	unsigned int length = val.size();
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, val.c_str(), length);
}

UTF8String::UTF8String(const char* value, int length)
{
	m_buffer = MemoryBuffer::Allocate(length);
	memcpy(m_buffer->buffer, value, length);
}

UTF8String::UTF8String(UTF8String::MemoryBuffer* buffer)
{
	m_buffer = buffer;
}

UTF8String::UTF8String(const UTF8String& other)
{
	m_buffer = other.m_buffer;
	m_buffer->Retain();
}	

int UTF8String::Compare(const UTF8String& other) const
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

UTF8String& UTF8String::operator =(const UTF8String& other)
{
	other.m_buffer->Retain();
	m_buffer->Release();
	m_buffer = other.m_buffer;
	return *this;
}

UTF8String UTF8String::operator +(const UTF8String& other) const
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

	return UTF8String(newBuffer);
}

UTF8String UTF8String::operator +(const char* other) const
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
	
	return UTF8String(newBuffer);
}

UTF8String& UTF8String::operator +=(const UTF8String& other)
{
	return operator =(*this + other);
}

UTF8String operator+(const char* a, const UTF8String& b)
{
	return UTF8String(a) + b;
}

UTF8String operator+(const std::string& a, const UTF8String& b)
{
	return UTF8String(a) + b;
}

bool UTF8String::operator ==(const UTF8String& other) const
{
	if (m_buffer->size != other.m_buffer->size)
	{
		return false;
	}
	return (memcmp(m_buffer->buffer, other.m_buffer->buffer, m_buffer->size) == 0);
}

bool UTF8String::operator !=(const UTF8String& other) const
{
	return !(operator ==(other));
}

bool UTF8String::operator <(const UTF8String& other) const
{
	return Compare(other) < 0;
}

bool UTF8String::operator >(const UTF8String& other) const
{
	return Compare(other) > 0;
}

bool UTF8String::operator <=(const UTF8String& other) const
{
	return Compare(other) <= 0;
}

bool UTF8String::operator >=(const UTF8String& other) const
{
	return Compare(other) >= 0;
}

std::vector<UTF8String> UTF8String::Split(UTF8String seperator, int max_splits, bool remove_duplicates)
{
	std::vector<UTF8String> result;
	UTF8String split = "";
		
	if (seperator == "")
	{
		result.push_back(*this);
		return result;
	}
		
	for (int i = 0; i < Length(); i++)
	{
		UTF8String res = Slice(i, i + 1);
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

int UTF8String::Index_Of(UTF8String needle, int start_index) const
{
	UTF8String result = "";	
	if (needle == "")
	{
		return -1;
	}
		
	for (int i = start_index; i < Length() - needle.Length(); i++)
	{
		UTF8String res = Slice(i, i + needle.Length());			
		if (res == needle)
		{
			return i;
		}
	}
		
	return -1;	
}

UTF8String UTF8String::Replace(int start_index, int length, UTF8String replacement) const
{
	return Slice(0, start_index) + replacement + Slice(start_index + length);
}

UTF8String UTF8String::Replace(UTF8String what, UTF8String with)
{
	UTF8String result = "";
	
	if (what == "")
	{
		return *this;
	}
		
	for (int i = 0; i < Length(); i++)
	{
		UTF8String res = Slice(i, i + 1);
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


// -------------------------------------------------------------------------------------------------------------------
// The following functions are all the true "utf8" aware functions, everything else builds ontop of these.
// -------------------------------------------------------------------------------------------------------------------

UTF8String::UTF8Iterator::UTF8Iterator(const char* inBuffer)
	: buffer(inBuffer)
	, offset(0)
{
}

int UTF8String::UTF8Iterator::GetStartByteOffset(int char_index)
{
	int start_offset = offset;
	offset = 0;

	u32 chr = 0;
	for (int i = 0; i < char_index; i++)
	{
		//DBG_ASSERT_STR(Advance(chr), "Attempt to get byte offset outside of utf8 string range.");
		if (!Advance(chr))
		{
			break;
		}
	}

	int char_start_offset = offset;

	offset = start_offset;

	return char_start_offset;
}

int UTF8String::UTF8Iterator::GetEndByteOffset(int char_index)
{
	int byte_offset = GetStartByteOffset(char_index);
	return byte_offset + GetCharSize(buffer + byte_offset);
}

int UTF8String::UTF8Iterator::GetCharSize(const char* data)
{
	u8 c1 = data[offset];

	// Null terminator.
	if (c1 == 0)
	{
		return 0;
	}
	// 1-byte sequence 
	else if (c1 < 0x80) 
	{	
		return 1;
	} 
	// continuation or overlong 2-byte sequence 
	else if (c1 < 0xC2) 
	{
		return 2;
	}
	// 2-byte sequence 
	else if (c1 < 0xE0) 
	{
		return 2;
	} 
	// 3-byte sequence 
	else if (c1 < 0xF0) 
	{
		return 3;
	} 
	// 4-byte sequence 
	else if (c1 < 0xF5) 
	{
		return 4;
	} 
	// Invalid, > U+10FFFF
	else 
	{
		return 0;
	}

	// wut
	return 0;
}

int UTF8String::UTF8Iterator::Length()
{
	int start_offset = offset;
	offset = 0;

	u32 chr;
	int length = 0;
	while (Advance(chr))
		length++;

	offset = start_offset;

	return length;
}

bool UTF8String::UTF8Iterator::Advance(u32& output)
{
	u8 c1 = buffer[offset];

	// Null terminator.
	if (c1 == 0)
	{
		return false;
	}
	// 1-byte sequence 
	else if (c1 < 0x80) 
	{	
		offset += 1;
		output = c1;
		return true;
	} 
	// continuation or overlong 2-byte sequence 
	else if (c1 < 0xC2) 
	{
		offset += 1;
		output = c1;
		return true; // Invalid char.
	}
	// 2-byte sequence 
	else if (c1 < 0xE0) 
	{
		u8 c2 = buffer[offset + 1];
		if ((c2 & 0xC0) != 0x80)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		output = (c1 << 6) + c2 - 0x3080;

		offset += 2;
		return true;
	} 
	// 3-byte sequence 
	else if (c1 < 0xF0) 
	{
		u8 c2 = buffer[offset + 1];
		if ((c2 & 0xC0) != 0x80) 
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		if (c1 == 0xE0 && c2 < 0xA0)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		u8 c3 = buffer[offset + 2];
		if ((c3 & 0xC0) != 0x80)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		output = (c1 << 12) + (c2 << 6) + c3 - 0xE2080;

		offset += 3;
		return true;
	} 
	// 4-byte sequence 
	else if (c1 < 0xF5) 
	{
		u8 c2 = buffer[offset + 1];
		if ((c2 & 0xC0) != 0x80)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		if (c1 == 0xF0 && c2 < 0x90)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		if (c1 == 0xF4 && c2 >= 0x90)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		u8 c3 = buffer[offset + 2];
		if ((c3 & 0xC0) != 0x80)
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		u8 c4 = buffer[offset + 3];
		if ((c4 & 0xC0) != 0x80) 
		{
			offset += 1;
			output = c1;
			return true; // Invalid char.
		}

		output = (c1 << 18) + (c2 << 12) + (c3 << 6) + c4 - 0x3C82080;

		offset += 4;
		return true;
	} 
	// Invalid, > U+10FFFF
	else 
	{
		offset += 1;
		output = c1;

		return true; // Invalid char.
	}

	offset += 1;
	output = c1;
	return true; // Invalid char.
}

int UTF8String::Length() const
{
	if (m_buffer->char_length_dirty)
	{
		UTF8Iterator iterator(m_buffer->buffer);
		m_buffer->char_length = iterator.Length();
		m_buffer->char_length_dirty = false;
	}
	return m_buffer->char_length;
}

const char* UTF8String::operator [](int index) 
{
	UTF8Iterator iterator(m_buffer->buffer);
	int offset = iterator.GetStartByteOffset(index);
	char* buffer = m_buffer->buffer + offset;
	int count = iterator.GetCharSize(buffer);

	memcpy((void*)m_tmp_char_buffer, buffer, count);
	m_tmp_char_buffer[count] = '\0';
	return m_tmp_char_buffer;
}

u32 UTF8String::GetCharCode(int index) const
{
	if (!(index >= 0 && index < Length()))
	{
		return '?';
	}

	UTF8Iterator iterator(m_buffer->buffer);

	u32 output = 0;
	for (int i = 0; i <= index; i++)
	{
		iterator.Advance(output);
	}

	return output;
}

UTF8String UTF8String::Sub_String(int offset, int count) const
{
	UTF8Iterator iterator(m_buffer->buffer);

	int end_offset = offset;
	if (count != 0x7FFFFFFF)
	{
		end_offset += count;
	}
	else
	{
		end_offset = Length();
	}

	int byteStart = iterator.GetStartByteOffset(offset);
	int byteEnd   = iterator.GetStartByteOffset(end_offset);

	return UTF8String(m_buffer->buffer + byteStart, byteEnd - byteStart);
}

UTF8String UTF8String::Slice(int start_index) const
{
	return Slice(start_index, Length());
}

UTF8String UTF8String::Slice(int start_index, int end_index) const
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
		UTF8Iterator iterator(m_buffer->buffer);
		int start_byte = iterator.GetStartByteOffset(start_index);
		int end_byte = iterator.GetEndByteOffset(start_index + ((end_index - start_index) - 1));
		return UTF8String(m_buffer->buffer + start_byte, end_byte - start_byte);
	}
}

