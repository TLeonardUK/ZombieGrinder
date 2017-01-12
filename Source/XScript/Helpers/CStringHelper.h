/* *****************************************************************

		CStringHelper.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CSTRINGHELPER_H_
#define _CSTRINGHELPER_H_

#include "Generic/Types/String.h"
#include <sstream>
#include <vector>
#include <ctime>
#include <map>

#ifdef _WIN32
#include <stdarg.h>
#else
#include <cstdarg>
#endif

// =================================================================
//	Class contains several string related helper functions.
// =================================================================
class CStringHelper
{
private:
	CStringHelper();

public:
	static String				PadRight		(String value, int length, String padding = " ");
	static String				PadLeft			(String value, int length, String padding = " ");
	static std::vector<String> Split			(String value, char deliminator);
	static String				Join			(std::vector<String> value, String glue);

	static String				Replace			(String value, String from, String to);
	static bool						IsHex			(char x);
	static String				ToHexString		(int code);
	
	static int						ToInt			(const String& str);
	static float					ToFloat			(const String& str);

	template<typename T>
	static String				ToString		(const T& value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}

	static String				GetLineInString		(String value, int lineIndex);

	static String				FormatString		(String value, ...);
	static String				FormatStringVarArgs	(String value, va_list& va);

	static String				ToLower				(String value);
	static String				ToUpper				(String value);
	static String				StripWhitespace		(String value);
	static String				StripChar			(String value, char chr);
	static String				MultiplyString		(String value, int counter);

	static String				CleanExceptAlphaNum	(String value, char chr);

	static String				GetDateTimeStamp	();
	
	static std::map<String, String> GetEnvironmentVariables();

};

#endif