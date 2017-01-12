/* *****************************************************************

		CStringHelper.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h>
#include "Generic/Types/String.h"
#include <assert.h>
#include <algorithm>

#ifdef _WIN32
#include <cstdarg>
#include <stdarg.h>
#include <Windows.h>
#else
#include <cstdarg>
#endif

#include <map>

#include "CStringHelper.h"

// =================================================================
//	Converts a string to an int.
// =================================================================
int	CStringHelper::ToInt(const String& str)
{
	return atoi(str.c_str());
}

// =================================================================
//	Converts a string to a float.
// =================================================================
float CStringHelper::ToFloat(const String& str)
{
	return (float)atof(str.c_str());
}

// =================================================================
//	Pads the right side of a string with the given character until
//  it is greater or equal to the length given.
// =================================================================
String CStringHelper::PadRight(String value, int length, String padding)
{
	assert(padding.length() > 0);
	
	if (length <= 0)
	{
		return value;
	}

	while (value.length() < (unsigned int)length)
	{
		value += padding;
	}

	return value;
}

// =================================================================
//	Pads the left side of a string with the given character until
//  it is greater or equal to the length given.
// =================================================================
String CStringHelper::PadLeft(String value, int length, String padding)
{
	assert(padding.length() > 0);

	if (length <= 0)
	{
		return value;
	}

	while (value.length() < (unsigned int)length)
	{
		value = padding + value;
	}

	return value;
}

// =================================================================
//	Splits a string up into multiple strings based on the given 
//	deliminator.
// =================================================================
std::vector<String> CStringHelper::Split(String value, char deliminator)
{
	std::vector<String> splits;

	int startIndex = 0;

	while (true)
	{
		int offset = value.find(deliminator, startIndex);
		if (offset < 0)
		{
			break;
		}
	
		splits.push_back(value.substr(startIndex, offset - startIndex));

		startIndex = offset + 1;
	}

	splits.push_back(value.substr(startIndex, value.size() - startIndex));

	return splits;
}

// =================================================================
//	Glus an array of strings together.
// =================================================================
String CStringHelper::Join(std::vector<String> values, String glue)
{
	String result = "";

	for (std::vector<String>::iterator iter = values.begin(); iter != values.end(); iter++)
	{
		if (iter != values.begin())
		{
			result += glue;
		}
		result += *iter;
	}

	return result;
}

// =================================================================
//	Replaces a string within another string.
// =================================================================
String CStringHelper::Replace(String value, String from, String to)
{
	int offset = 0;
	
	while (true)
	{
		int pos = value.find(from, offset);
		if (pos == String::npos)
		{
			break;
		}

		String left = value.substr(0, pos);
		String right = value.substr(pos + from.size());
		value = left + to + right;
		
		offset = pos + to.size();
	}

	return value;
}

// =================================================================
//	Returns true if the given character is hexidecimal.
// =================================================================
bool CStringHelper::IsHex(char x)
{
	return ((x >= '0' && x <= '9') ||
			(x >= 'A' && x <= 'F') ||
			(x >= 'a' && x <= 'f'));
}

// =================================================================
//  Converts a number to a hex string. 
// =================================================================
String CStringHelper::ToHexString(int code)
{
	std::stringstream sstream;
	sstream << std::hex << code;
	return sstream.str();
}

// =================================================================
//	Uses variable arguments to format a string.
// =================================================================
String	CStringHelper::FormatString(String value, ...)
{
	va_list va;
	va_start(va, value);
	value = CStringHelper::FormatStringVarArgs(value, va);
	va_end(va);

	return value;
}

// =================================================================
//	Uses variable arguments to format a string.
// =================================================================
String	CStringHelper::FormatStringVarArgs(String value, va_list& va)
{
	va_list start_list;

// Fucking microsoft.
#ifdef _WIN32
	start_list = va;
#else
	va_copy(start_list, va);
#endif

	int size = vsnprintf(NULL, NULL, value.c_str(), va);

	char* buffer = new char[size + 1];
	vsnprintf(buffer, size + 1, value.c_str(), start_list);

	String result = buffer;

	delete[] buffer;

	return result;
}

// =================================================================
//  Get the specific line in the string.
// =================================================================
String CStringHelper::GetLineInString(String value, int lineIndex)
{
	String line;
	int lineOffset = 0;
	int startIndex = 0;

	while (true)
	{
		int offset = value.find('\n', startIndex);
		if (offset <= 0)
		{
			break;
		}
	
		line = value.substr(startIndex, offset - startIndex);
		if (lineOffset == lineIndex)
		{
			return line;
		}
		lineOffset++;

		startIndex = offset + 1;
	}

	line = value.substr(startIndex, value.size() - startIndex);

	if (lineOffset == lineIndex)
	{
		return line;
	}
	else
	{
		return "";
	}
}

// =================================================================
//  Converts a string to a lowercase representation.
// =================================================================
String CStringHelper::ToLower(String value)
{
//	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
//	return value;
	return value.To_Lower();
}

// =================================================================
//  Converts a string to a uppercase representation.
// =================================================================
String CStringHelper::ToUpper(String value)
{
	//std::transform(value.begin(), value.end(), value.begin(), ::toupper);
	//return value;
	return value.To_Upper();
}

// =================================================================
//  Strips whitespace from string.
// =================================================================
String	CStringHelper::StripWhitespace(String value)
{
	while (value.size() > 0 &&
  		   (value[0] == ' ' || 
		    value[0] == '\r' || 
			value[0] == '\n' || 
			value[0] == '\t' || 
			value[0] == '\v'))
	{
		value = value.substr(1, value.size() - 1);
	}

	while (value.size() > 0 &&
  		   (value[value.size() - 1] == ' ' || 
		    value[value.size() - 1] == '\r' || 
			value[value.size() - 1] == '\n' || 
			value[value.size() - 1] == '\t' || 
			value[value.size() - 1] == '\v'))
	{
		value = value.substr(0, value.size() - 1);
	}

	return value;
}

// =================================================================
//  Strips a given character from a string.
// =================================================================
String	CStringHelper::StripChar(String value, char chr)
{
	while (value.size() > 0 &&
  		   (value[0] == chr))
	{
		value = value.substr(1, value.size() - 1);
	}

	while (value.size() > 0 &&
  		   (value[value.size() - 1] == chr))
	{
		value = value.substr(0, value.size() - 1);
	}

	return value;
}

// =================================================================
//  MReplaces all character in the string except alpha numeric.
// =================================================================
String CStringHelper::CleanExceptAlphaNum(String value, char chr)
{
	String result = "";
	for (unsigned int i = 0; i < value.size(); i++)
	{
		char rchr = value.at(i);
		if ((rchr >= '0' && rchr <= '9') ||
			(rchr >= 'a' && rchr <= 'z') ||
			(rchr >= 'A' && rchr <= 'Z'))
		{
			result += rchr;
		}
		else
		{			
			result += chr;
		}
	}
	return result;
}

// =================================================================
//  Multiplies the string the given number of times.
// =================================================================
String CStringHelper::MultiplyString(String value, int counter)
{
	String result = "";

	for (int i = 0; i < counter; i++)
	{
		result += value;
	}

	return result;
}

// =================================================================
//  Multiplies the string the given number of times.
// =================================================================
String CStringHelper::GetDateTimeStamp()
{
	char buffer[1024];
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 1024, "%d/%m/%Y %H:%M", timeinfo);

	return String(buffer);
}

// =================================================================
//  Gets all available environment variables.
// =================================================================
std::map<String, String> CStringHelper::GetEnvironmentVariables()
{
	std::map<String, String> vars;

#ifdef _WIN32
	
	LPTCH str = GetEnvironmentStrings();
	
	String newvar = "";
	unsigned int offset = 0;
	while (true)
	{
		char chr = (char)str[offset];
		if (chr == '\0')
		{
			// Should be in the format of name=value.
			unsigned int idx = newvar.find('=');
			if (idx > 0) // Ignore envvars that start with an = sign 
							// (some wierd variables we don't care are reported at the start like this).
			{
				vars.insert(std::pair<String, String>(newvar.substr(0, idx), newvar.substr(idx + 1)));
			}

			// End of values?
			if (str[offset + 1] == '\0')
			{
				break;
			}

			newvar = "";
		}
		else
		{
			newvar += chr;
		}

		offset++;
	}
			
	FreeEnvironmentStrings(str);

#elif defined(__linux__)  || defined(__APPLE__) 

	extern char** environ;

	String newvar = "";
	char* var = NULL;
	int i = 0;
	
	while (true)
	{
		var = *(environ + (i++));
		if (var == NULL)
		{
			break;
		}
		
		newvar = String(var);
	 
		// Should be in the format of name=value.
		unsigned int idx = newvar.find('=');
		if (idx > 0) // Ignore envvars that start with an = sign 
					 // (some wierd variables we don't care are reported at the start like this).
		{
			vars.insert(std::pair<String, String>(newvar.substr(0, idx), newvar.substr(idx + 1)));
		}
	}

#else
	
	assert(0);
	
#endif
	
	return vars;
}
