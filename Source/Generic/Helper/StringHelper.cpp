// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Helper/StringHelper.h"

#include <stdarg.h> 
#include <cstring>
#include <cstdio>
#include <math.h>

int StringHelper::Split(const char* v, char deliminator, std::vector<std::string>& segments)
{
	std::string value = v;

	if (strlen(v) == 0)
	{
		return 0;
	}

	int startIndex = 0;

	while (true)
	{
		int offset = value.find(deliminator, startIndex);
		if (offset < 0)
		{
			break;
		}
	
		segments.push_back(value.substr(startIndex, offset - startIndex));

		startIndex = offset + 1;
	}

	segments.push_back(value.substr(startIndex, value.size() - startIndex));

	return segments.size();
}

void StringHelper::Find_Line_And_Column(const char* text, int offset, int& line, int& column)
{
	// Reset values.
	line = 1;
	column = 0;

	// Calculate length.
	//int len = strlen(text); // Assumption made that text length is inside bounds, as this function is used with strings containing multiple \0's
	for (int i = 0; /*i < len &&*/ i <= offset; i++)
	{
		char chr = text[i];
		if (chr == '\n')
		{
			line++;
			column = 0;
		}
		else if (chr != '\r')
		{
			column++;
		}
	}
}

std::string StringHelper::Trim(const char* value)
{
	int start_offset = 0;
	int end_offset = 0;
	int length = strlen(value);

	for (start_offset = 0; start_offset < length; start_offset++)
	{
		if (!iswspace(value[start_offset]))
		{
			break;
		}
	}
	
	for (end_offset = length - 1; end_offset >= 0; end_offset--)
	{
		if (!iswspace(value[end_offset]))
		{
			end_offset++;
			break;
		}
	}

	std::string result = std::string(value).substr(start_offset, end_offset - start_offset);

	return result;
}

std::string StringHelper::Remove_Whitespace(const char* value)
{
	std::string result = "";
	int length = strlen(value);

	for (int i = 0; i < length; i++)
	{
		if (!iswspace(value[i]))
		{
			result += value[i];
		}
	}
	
	return result;
}

std::string	StringHelper::Replace(const char* value, const char* from, const char* to)
{
	std::string val = value;

	int len = strlen(from);
	int to_len = strlen(to);
	int offset = 0;

	while (true)
	{
		size_t position = val.find(from, offset);
		if (position == std::string::npos)
		{
			break;
		}

		val = val.replace(position, len, to);
		offset = position + to_len;
	}

	return val;
}

std::string	StringHelper::Format_Time(const char* format, int time)
{
	std::string val = format;

	// MS to Seconds.
	time = time / 1000;

	int seconds = time % 60;
	int minutes = (time / 60) % 60;
	int hours   = (time / 60) / 60;

	val = Replace(val.c_str(), "ss", Format("%02d", seconds).c_str());
	val = Replace(val.c_str(), "mm", Format("%02d", minutes).c_str());
	val = Replace(val.c_str(), "hh", Format("%02d", hours).c_str());

	return val;
}

std::string	StringHelper::Format_Number(float number, bool bWithSign)
{
	int whole_number_int = (int)number;
	std::string whole_number = StringHelper::Format("%i", whole_number_int);

	// Do comma-deliminated number.
	std::string result = "";

	int comma_counter = whole_number.size();
	for (unsigned int i = 0; i < whole_number.size(); i++, comma_counter--)
	{
		char val = whole_number.at(i);
		if ((comma_counter % 3) == 0 && i > 0)
		{
			result += ",";
		}
		result += val;
	}

	float frac = fabs(number) - fabs((float)whole_number_int);

	// Add fractional part on if its large enough.
	if (frac > 0.01f)
	{
		result += StringHelper::Format("%.2f", frac).substr(1);
	}

	if (bWithSign && number > 0.0f)
	{
		result = "+" + result;
	}

	return result;
}

int StringHelper::Hash(const char* value)
{
	unsigned int hash = 0;

	for (; *value; ++value)
	{
		hash += *value;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

bool StringHelper::IsHex(char value)
{
	return (value >= '0' && value <= '9') || (value >= 'A' && value <= 'F') || (value >= 'a' && value <= 'f');
}


std::string	StringHelper::Lowercase(const char* value)
{
	std::string result = "";
	while (value[0] != '\0')
	{
		result += ::tolower(value[0]);
		value++;
	}
	return result;
}

std::string	StringHelper::Uppercase(const char* value)
{
	std::string result = "";
	while (value[0] != '\0')
	{
		result += ::toupper(value[0]);
		value++;
	}
	return result;
}

std::string StringHelper::Format(const char* format, ...)
{
	va_list va;
	va_start(va, format);

	char buffer[512];
	int num = vsnprintf(buffer, 512, format, va);
	if (num >= 511)
	{
		char* new_buffer = new char[num + 1];
		vsnprintf(new_buffer, num + 1, format, va);
		std::string result = new_buffer;
		delete[] new_buffer;
		
		va_end(va);
		return result;
	}

	va_end(va);

	return buffer;
}

std::string StringHelper::Join(const std::vector<std::string>::iterator& begin, const std::vector<std::string>::iterator& end, std::string glue)
{
	std::string result = "";

	for (std::vector<std::string>::iterator iter = begin; iter != end; iter++)
	{
		if (iter != begin)
		{
			result += glue;
		}
		result += *iter;
	}

	return result;
}

// Matches a string similar to fnmatch, based on code from;
// http://xoomer.virgilio.it/acantato/dev/wildcard/wildmatch.html
bool StringHelper::Match(std::string value_in, std::string pattern_in)
{
	const char* value = value_in.c_str();
	const char* pattern = pattern_in.c_str();

	const char* s = NULL;
	const char* p = NULL;
	bool star = false;

loopStart:
	for (s = value, p = pattern; *s; ++s, ++p)
	{
		switch (*p)
		{
		case '?':
			{
				if (*s == '.')
					goto starCheck;

				break;
			}
		case '*':
			{
				star = true;
				value = s;
				pattern = p;

				do
				{ 
					++pattern; 
				} 
				while (*pattern == '*');

				if (!*pattern) 
					return true;

				goto loopStart;
			}
		default:
			{
				if (::toupper(*s) != ::toupper(*p))
					goto starCheck;

				break;
			}
		}
	}

	while (*p == '*') 
		++p;
	
	return (!*p);

starCheck:
	if (!star)
		return false;
	
	value++;
	goto loopStart;
}

void StringHelper::Split_UTF8(const char* input, std::vector<u32>& output)
{
	const u8* utf8 = reinterpret_cast<const u8*>(input);

	// Based on http://en.wikipedia.org/wiki/UTF-8#Description

	int offset = 0;
	while (true)
	{
		u8 c1 = utf8[offset];

		// Null terminator.
		if (c1 == 0)
		{
			break;
		}
		// 1-byte sequence 
		else if (c1 < 0x80) 
		{
			output.push_back(c1);
			offset += 1;
		} 
		// continuation or overlong 2-byte sequence 
		else if (c1 < 0xC2) 
		{
			break;
		}
		// 2-byte sequence 
		else if (c1 < 0xE0) 
		{
			u8 c2 = utf8[offset + 1];
			if ((c2 & 0xC0) != 0x80)
			{
				return;
			}

			u32 value = (c1 << 6) + c2 - 0x3080;

			output.push_back(value);
			offset += 2;
		} 
		// 3-byte sequence 
		else if (c1 < 0xF0) 
		{
			u8 c2 = utf8[offset + 1];
			if ((c2 & 0xC0) != 0x80) 
			{
				return;
			}

			if (c1 == 0xE0 && c2 < 0xA0)
			{
				return;
			}

			u8 c3 = utf8[offset + 2];
			if ((c3 & 0xC0) != 0x80)
			{
				return;
			}

			u32 value = (c1 << 12) + (c2 << 6) + c3 - 0xE2080;

			output.push_back(value);
			offset += 3;
		} 
		// 4-byte sequence 
		else if (c1 < 0xF5) 
		{
			u8 c2 = utf8[offset + 1];
			if ((c2 & 0xC0) != 0x80)
			{
				return;
			}

			if (c1 == 0xF0 && c2 < 0x90)
			{
				return;
			}

			if (c1 == 0xF4 && c2 >= 0x90)
			{
				return;
			}

			u8 c3 = utf8[offset + 2];
			if ((c3 & 0xC0) != 0x80)
			{
				return;
			}

			u8 c4 = utf8[offset + 3];
			if ((c4 & 0xC0) != 0x80) 
			{
				return;
			}

			u32 value = (c1 << 18) + (c2 << 12) + (c3 << 6) + c4 - 0x3C82080;

			output.push_back(value);
			offset += 4;
		} 
		// Invalid, > U+10FFFF
		else 
		{
			break;
		}
	}
}

std::vector<std::string> StringHelper::Crack_Command_Line(std::string cmdline)
{
	std::vector<std::string> result;

	std::string segment = "";
	bool bInQuotes = false;

	for (unsigned int i = 0; i < cmdline.size(); i++)
	{
		char chr = cmdline[i];
		if (chr == '\"')
		{
			bInQuotes = !bInQuotes;
		}
		else if (chr == ' ')
		{
			if (bInQuotes)
			{
				segment += chr;
			}
			else
			{
				if (segment != "")
				{
					result.push_back(segment);
					segment = "";
				}
			}
		}
		else
		{
			segment += chr;
		}
	}

	if (segment != "")
	{
		result.push_back(segment);
	}

	return result;
}