// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_STRINGHELPER_
#define _GENERIC_HELPER_STRINGHELPER_

#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <map>

class StringHelper
{
private:
	StringHelper();

public:
	static int			Split(const char* value, char deliminator, std::vector<std::string>& segments);
	static void			Find_Line_And_Column(const char* text, int offset, int& line, int& column);
	static std::string	Trim(const char* value);
	static std::string	Remove_Whitespace(const char* value);
	static std::string	Lowercase(const char* value);
	static std::string	Uppercase(const char* value);
	static std::string	Replace(const char* value, const char* from, const char* to);
	static std::string	Format(const char* format, ...);
	static std::string	Format_Time(const char* format, int time);
	static std::string	Format_Number(float value, bool bWithSign = false);
	static int			Hash(const char* value);
	static bool			IsHex(char value);

	static bool			Match(std::string value, std::string filter);

	static std::vector<std::string>  Crack_Command_Line(std::string cmdline);

	static std::string  Join(const std::vector<std::string>::iterator& begin, const std::vector<std::string>::iterator& end, std::string glue = " ");

	static u64 To_U64(std::string value)
	{
		if (value == "")
		{
			return 0;
		}

#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
		u64 id = std::stoull(value.c_str());
#else
		u64 id = strtoull(value.c_str(), NULL, 10);
#endif
		return id;
	}

	static int Parse_Hex(std::string value)
	{
		if (value == "")
		{
			return 0;
		}

#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
		int id = std::stoul(value.c_str(), NULL, 16);
#else
		int id = strtoul(value.c_str(), NULL, 16);
#endif
		return id;
	}

	static int Parse_Int(std::string value)
	{
		if (value == "")
		{
			return 0;
		}

#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
		int id = std::stoul(value.c_str(), NULL, 10);
#else
		int id = strtoul(value.c_str(), NULL, 10);
#endif
		return id;
	}

	template<typename T>
	static std::string	To_String(T value)
	{		
		std::stringstream ss;
		ss << value;
		return ss.str();
	} 

	static void Split_UTF8(const char* utf8, std::vector<u32>& output);


};

#endif