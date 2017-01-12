// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_COMMANDLINEPARSER_
#define _GENERIC_HELPER_COMMANDLINEPARSER_

#include <string>
#include <vector>
#include <cstring>

struct CommandLineOptionType
{
	enum Type
	{
		String,
		Int,
		Float,
		Bool
	};
};

struct CommandLineOption
{
	MEMORY_ALLOCATOR(CommandLineOption, "Generic");

public:
	void* DataPtr;
	CommandLineOptionType::Type DataType;
	const char* Name;
	const char* Description;
};

class CommandLineParser
{
	MEMORY_ALLOCATOR(CommandLineParser, "Generic");

private:
	std::vector<CommandLineOption> m_options;

	const char* m_input;
	int m_input_offset;
	int m_input_length;

	void Start_Parsing(const char* input);
	bool End_Of_Tokens();
	std::string Read_Token();
	CommandLineOption* Get_Option(std::string name);
	void Set_Option_Value(CommandLineOption* option, std::string value);

protected:	
	void Register_Option(std::string* var_ptr,	const char* name, const char* description);
	void Register_Option(int* var_ptr,			const char* name, const char* description);
	void Register_Option(float* var_ptr,		const char* name, const char* description);
	void Register_Option(bool* var_ptr,			const char* name, const char* description);

public:
	CommandLineParser();

	void Parse(const char* command_line);

};

#endif