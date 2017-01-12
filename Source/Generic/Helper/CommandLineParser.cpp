// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Helper/CommandLineParser.h"

#include <cstdlib>

void CommandLineParser::Register_Option(std::string* var_ptr, const char* name, const char* description)
{
	CommandLineOption option;
	option.DataPtr = var_ptr;
	option.DataType = CommandLineOptionType::String;
	option.Name = name;
	option.Description = description;
	m_options.push_back(option);
}

void CommandLineParser::Register_Option(int* var_ptr, const char* name, const char* description)
{
	CommandLineOption option;
	option.DataPtr = var_ptr;
	option.DataType = CommandLineOptionType::Int;
	option.Name = name;
	option.Description = description;
	m_options.push_back(option);
}

void CommandLineParser::Register_Option(float* var_ptr, const char* name, const char* description)
{
	CommandLineOption option;
	option.DataPtr = var_ptr;
	option.DataType = CommandLineOptionType::Float;
	option.Name = name;
	option.Description = description;
	m_options.push_back(option);
}

void CommandLineParser::Register_Option(bool* var_ptr, const char* name, const char* description)
{
	CommandLineOption option;
	option.DataPtr = var_ptr;
	option.DataType = CommandLineOptionType::Bool;
	option.Name = name;
	option.Description = description;
	m_options.push_back(option);
}

CommandLineParser::CommandLineParser()
{
}

void CommandLineParser::Start_Parsing(const char* input)
{
	m_input = input;
	m_input_offset = 0;
	m_input_length = strlen(input);
}

bool CommandLineParser::End_Of_Tokens()
{
	return m_input_offset >= m_input_length;
}

std::string CommandLineParser::Read_Token()
{
	std::string result = "";
	bool in_quote = false;

	while (!End_Of_Tokens())
	{
		char next = m_input[m_input_offset];

		if (next == '\"' || next == '\'')
		{
			in_quote = !in_quote;
		}
		else if (next == ' ' && in_quote == false)
		{
			if (result != "")
			{
				break;
			}
		}
		else 
		{
			result += next;
		}

		m_input_offset++;
	}

	return result;
}

CommandLineOption* CommandLineParser::Get_Option(std::string name)
{
	for (std::vector<CommandLineOption>::iterator iter = m_options.begin(); iter != m_options.end(); iter++)
	{
		CommandLineOption& option = *iter;
		if (stricmp(option.Name, name.c_str()) == 0)
		{
			return &option;
		}
	}
	return NULL;
}

void CommandLineParser::Set_Option_Value(CommandLineOption* option, std::string value)
{
	switch (option->DataType)
	{
	case CommandLineOptionType::String:		*static_cast<std::string*>(option->DataPtr) = value;
	case CommandLineOptionType::Int:		*static_cast<int*>(option->DataPtr)			= atoi(value.c_str());
	case CommandLineOptionType::Float:		*static_cast<float*>(option->DataPtr)		= (float)atof(value.c_str());
	case CommandLineOptionType::Bool:		*static_cast<bool*>(option->DataPtr)		= (atoi(value.c_str()) != 0);
	}
}

void CommandLineParser::Parse(const char* command_line)
{
	DBG_LOG("Parsing command line:", command_line);
	DBG_LOG("	%s", command_line);

	Start_Parsing(command_line);
	
	while (!End_Of_Tokens())
	{
		std::string tok = Read_Token();
		CommandLineOption* option = Get_Option(tok);
		
		if (option != NULL)
		{
			std::string value = "1";

			if (option->DataType != CommandLineOptionType::Bool)
			{
				if (End_Of_Tokens())
				{
					DBG_LOG("Expected value for command line option '%s'", tok.c_str());
					break;
				}
				value = Read_Token();
			}

			Set_Option_Value(option, value);
			DBG_LOG("Accepted command line option '%s' with value '%s'", tok.c_str(), value.c_str());
		}
		else
		{
			DBG_LOG("Invalid command line option '%s'", tok.c_str());
		}
	}
}