// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_OPTIONCOMMANDLINEPARSER_
#define _ENGINE_OPTIONS_OPTIONCOMMANDLINEPARSER_

#include <string>

class OptionCommandLineParser
{
	MEMORY_ALLOCATOR(OptionCommandLineParser, "Engine");

private:
	const char* m_input;
	int m_input_offset;
	int m_input_length;

	void Start_Parsing(const char* input);
	bool End_Of_Tokens();
	std::string Read_Token();

public:
	OptionCommandLineParser();

	void Parse(const char* command_line);

};

#endif

