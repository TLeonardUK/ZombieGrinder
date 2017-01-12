// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/OptionCommandLineParser.h"
#include "Engine/Options/OptionRegistry.h"

#include "Engine/Options/Types/BoolOption.h"

OptionCommandLineParser::OptionCommandLineParser()
{
}

void OptionCommandLineParser::Start_Parsing(const char* input)
{
	m_input = input;
	m_input_offset = 0;
	m_input_length = strlen(input);
}

bool OptionCommandLineParser::End_Of_Tokens()
{
	return m_input_offset >= m_input_length;
}

std::string OptionCommandLineParser::Read_Token()
{
	std::string result = "";
	bool in_quote = false;
	bool first_char = true;

	while (!End_Of_Tokens())
	{
		char next = m_input[m_input_offset];

		if ((next == '+' || next == '-') && result == "")
		{
			// Ignore these if first char.
		}
		else if (next == '\"' || next == '\'')
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

void OptionCommandLineParser::Parse(const char* command_line)
{
	//DBG_LOG("Parsing command line options:", command_line);
	//DBG_LOG("	%s", command_line);

	Start_Parsing(command_line);
	
	// We don't care about first token, it's the exe location.
	Read_Token();

	while (!End_Of_Tokens())
	{
		std::string tok = Read_Token();
		if (tok == "")
		{
			continue;
		}

		Option* option = OptionRegistry::Get()->Find(tok.c_str());
		
		if (option != NULL)
		{
			std::string value = "1";

			if (dynamic_cast<BoolOption*>(option) == NULL)
			{
				if (End_Of_Tokens())
				{
					DBG_LOG("Expected value for command line option '%s'", tok.c_str());
					break;
				}
				value = Read_Token();
				if (value == "")
				{
					DBG_LOG("Expected value for command line option '%s'", tok.c_str());
					break;
				}
			}
			
			if (option->Parse(value.c_str()))
			{
				option->Add_Flag(OptionFlags::SetByCommandLine);
				DBG_LOG("Accepted command line option '%s' with value '%s'", tok.c_str(), value.c_str());
			}
			else
			{
				DBG_LOG("Unable to parse command line option '%s' with value '%s'", tok.c_str(), value.c_str());
			}
		}
		else
		{
			DBG_LOG("Invalid command line option '%s'", tok.c_str());
		}
	}
}