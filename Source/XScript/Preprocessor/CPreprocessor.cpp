/* *****************************************************************

		CPreprocessor.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "Generic/Types/String.h"
#include <stdio.h>
#include <assert.h>
#include <stdexcept>

#include "XScript/Compiler/CCompiler.h"
#include "CPreprocessor.h"
#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Helpers/CPathHelper.h"

#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Nodes/EvaluationResult.h"

// =================================================================
//	Processes input and performs the actions requested.
// =================================================================
bool CPreprocessor::Process(CTranslationUnit* context)
{	
	// TODO: Optimize the preprocessor, to slow atm, we don't use it either, so lets just skip it for now.
	return true;
	
	m_lines			= CStringHelper::Split(context->GetSource(), '\n');
	m_result		= "";
	m_defines		= &context->GetDefines();
	m_line_index	= 0;
	m_context		= context;

	// Quick check for any #'s in context source, if non then we don't care about 
	// parsing (results in skipping the {Define} tags, but seeing as we never use them anyway ...)
	if (!context->GetSource().Contains("#"))
	{
		return true;
	}

	while (!EndOfLines())
	{
		ParseLine(ReadLine());
	}

	context->GetSource() = m_result;

	return true;
}

// =================================================================
//	Returns true if we are at the end of lines.
// =================================================================
bool CPreprocessor::EndOfLines()
{
	return m_line_index >= m_lines.size();
}

// =================================================================
//	Splits a line into command and value.
// =================================================================
std::vector<String> CPreprocessor::SplitLine(String line)
{
	std::vector<String> result;

	// Find starting hash.
	char starting_char		  = '\0';
	int	 starting_char_offset = 0;
	for (unsigned int j = 0; j < line.size(); j++)
	{
		char start = line[j];
		if (start != '\t' &&
			start != '\n' &&
			start != '\r' &&
			start != ' ')
		{
			starting_char = start;
			starting_char_offset = j;
			break;
		}
	}

	// Is this a command?
	if (starting_char == '#')
	{
		String expr		= line.substr(starting_char_offset);
		int			space_index = expr.find(' ');
		String cmd			= expr;
		String value		= "";

		if (space_index == String::npos)
		{
			space_index = expr.find('\t');
		}

		if (space_index != String::npos)
		{
			cmd   = CStringHelper::StripWhitespace(expr.substr(0, space_index));
			value = CStringHelper::StripWhitespace(expr.substr(space_index + 1));
		}

		result.push_back(cmd);
		result.push_back(value);
		return result;
	}

	return result;
}

// =================================================================
//	Parses the given line of text.
// =================================================================
void CPreprocessor::ParseLine(String line)
{
	// Split line.
	std::vector<String> split = SplitLine(line);

	// Is this a command?
	if (split.size() > 0)
	{
		String cmd			= split.at(0);
		String value		= split.size() > 1 ? split.at(1) : "";

		if (cmd == "#if" || cmd == "#ifdef")
		{
			ParseIf(value);
		}
		else if (cmd == "#define" || cmd == "#def")
		{
			ParseDefine(value);
		}
		else if (cmd == "#undefine" || cmd == "#undef")
		{
			ParseUndefine(value);
		}
		else if (cmd == "#error" || cmd == "#err")
		{
			ParseError(value);
		}
		else if (cmd == "#warning" || cmd == "#warn")
		{
			ParseWarning(value);
		}
		else if (cmd == "#info")
		{
			ParseInfo(value);
		}
		else
		{
			ParseError(CStringHelper::FormatString("Unknown preprocessor command '%s'.", cmd.c_str()));
		}
	}
	else
	{		
		Output(line + "\n");
	}
}

// =================================================================
//	Reads the next line of text in the input.
// =================================================================
String CPreprocessor::ReadLine()
{
	m_lineToken.Column		= 1;
	m_lineToken.Row			= m_line_index + 1;
	m_lineToken.Literal		= "";
	m_lineToken.Type		= TokenIdentifier::PreProcessor;
	m_lineToken.SourceFile	= m_context->GetFilePath();

	if (m_line_index >= m_lines.size())
	{
		return "";
	}

	String line = m_lines.at(m_line_index++);

	//line = ReplaceDefineTags(line);
	m_currentLine = line;

	return line;
}

// =================================================================
//	Replaces all define tags in a string.
// =================================================================
String CPreprocessor::ReplaceDefineTags(String line)
{
	// Replace defines on this line.
	if (line.find('{') != String::npos &&
		line.find('}') != String::npos)
	{
		for (std::vector<CDefine>::iterator  iter = m_defines->begin(); iter != m_defines->end(); iter++)
		{
			CDefine& define = *iter;
			line = CStringHelper::Replace(line, "{" + define.Name + "}", define.Value);
		}
	}

	return line;
}

// =================================================================
//	Reads the next line of text in the input without advancing
//  the read pointer.
// =================================================================
String CPreprocessor::LookAheadLine()
{
	if (m_line_index >= m_lines.size())
	{
		return "";
	}

	String line = m_lines.at(m_line_index);

	//line = ReplaceDefineTags(line);

	return line;
}

// =================================================================
//	Reads the current line.
// =================================================================
String CPreprocessor::CurrentLine()
{
	return m_currentLine;
}

// =================================================================
//	Appends the given text to the resulting preprocessed text.
// =================================================================
void CPreprocessor::Output(String output)
{
	m_result += output;
}

// =================================================================
//	Parses an if block.
// =================================================================
void CPreprocessor::ParseIfBlock()
{
	while (!EndOfLines())
	{		
		std::vector<String> split = SplitLine(LookAheadLine());
		if (split.size() > 1)
		{
			String cmd = split.at(0);
			if (cmd == "#endif" || cmd == "#end")
			{
				ReadLine();
				Output("\n");
				break;
			}
			else if (cmd == "#else" || cmd == "#elif" || cmd == "#elseif")
			{
				ReadLine();
				Output("\n");
				break;
			}
			else
			{
				ParseLine(ReadLine());
			}
		}
		else
		{
			ParseLine(ReadLine());
		}
	}
}

// =================================================================
//	Skips an if block.
// =================================================================
void CPreprocessor::SkipIfBlock()
{
	int depth = 1;

	while (!EndOfLines())
	{		
		std::vector<String> split = SplitLine(LookAheadLine());
		if (split.size() >= 1)
		{
			String cmd = split.at(0);
			if (cmd == "#if")
			{
				ReadLine();
				Output("\n");
				depth++;
			}
			else if (cmd == "#endif" || cmd == "#end")
			{
				ReadLine();
				Output("\n");
				depth--;

				if (depth <= 0)
				{
					break;
				}
			}
			else if (cmd == "#else" || cmd == "#elseif" || cmd == "#elif")
			{
				ReadLine();
				Output("\n");

				if (depth <= 1)
				{
					break;
				}
			}
			else
			{
				ReadLine();
				Output("\n");
			}
		}
		else
		{
			ReadLine();
			Output("\n");
		}
	}
}

// =================================================================
//	Parses an if statement.
// =================================================================
void CPreprocessor::ParseIf(String line)
{
	Output("\n");

	// Accept if block.
	bool accept = Evaluate(m_context, m_lineToken, line, m_defines).GetBool();
	if (accept == true)
	{
		ParseIfBlock();
	}
	else
	{
		SkipIfBlock();
	}	

	// Keep parsing else blocks.
	while (true)
	{
		std::vector<String> split = SplitLine(CurrentLine());
		String cmd = split.at(0);
		String val = split.size() > 1 ? split.at(1) : "";

		if (cmd == "#endif" || cmd == "#end")
		{
			break;
		}
		else if (cmd == "#elif" || cmd == "#elseif")
		{	
			if (accept == true)
			{
				SkipIfBlock();
			}
			else
			{
				accept = Evaluate(m_context, m_lineToken, val, m_defines).GetBool();
				if (accept == true)
				{
					ParseIfBlock();
				}
				else
				{
					SkipIfBlock();
				}
			}	
		}
		else if (cmd == "#else")
		{	
			if (accept == true)
			{
				SkipIfBlock();
			}
			else
			{
				ParseIfBlock();
			}	
			break;
		}
	}
}

// =================================================================
//	Parses an define block.
// =================================================================
void CPreprocessor::ParseDefine(String value)
{
	Output("\n");

	value = CStringHelper::StripWhitespace(value);
				
	int space_index = value.find(' ');
	if (space_index == String::npos)
	{
		space_index = value.find('\t');
	}

	String		 def_name  = CStringHelper::StripWhitespace(value);
	String		 def_value = "1";
	DefineType::Type def_type  = DefineType::Bool;

	if (space_index != String::npos)
	{
		def_name  = CStringHelper::StripWhitespace(value.substr(0, space_index));
		def_value = CStringHelper::StripWhitespace(value.substr(space_index + 1));

		EvaluationResult result = Evaluate(m_context, m_lineToken, def_value, m_defines);
		switch (result.GetType())
		{
			case EvaluationResult::BOOL:		def_type = DefineType::Bool;
			case EvaluationResult::FLOAT:		def_type = DefineType::Float;
			case EvaluationResult::INT:			def_type = DefineType::Int;
			case EvaluationResult::STRING:		def_type = DefineType::String;
		}
		def_value = result.GetString();
	}

	CDefine define = CDefine(def_type, def_name, def_value);
	m_defines->push_back(define);
}

// =================================================================
//	Parses an undefine block.
// =================================================================
void CPreprocessor::ParseUndefine(String line)
{
	Output("\n");

	String name = CStringHelper::StripWhitespace(line);
				
	for (std::vector<CDefine>::iterator  iter = m_defines->begin(); iter != m_defines->end(); iter++)
	{
		CDefine& def = *iter;
		if (def.Name == name)
		{
			iter = m_defines->erase(iter);
			break;
		}
	}
}

// =================================================================
//	Parses an error block.
// =================================================================
void CPreprocessor::ParseError(String line)
{
	Output("\n");

	CToken token;
	token.Column		= 1;
	token.Row			= m_line_index + 1;
	token.Literal		= "";
	token.Type			= TokenIdentifier::PreProcessor;
	token.SourceFile	= m_context->GetFilePath();
	
	m_context->FatalError(Evaluate(m_context, m_lineToken, line, m_defines).GetString(), m_lineToken);		
}

// =================================================================
//	Parses an warning block.
// =================================================================
void CPreprocessor::ParseWarning(String line)
{
	Output("\n");

	CToken token;
	token.Column		= 1;
	token.Row			= m_line_index + 1;
	token.Literal		= "";
	token.Type			= TokenIdentifier::PreProcessor;
	token.SourceFile	= m_context->GetFilePath();
	
	m_context->Warning(Evaluate(m_context, m_lineToken, line, m_defines).GetString(), m_lineToken);		
}

// =================================================================
//	Parses an info block.
// =================================================================
void CPreprocessor::ParseInfo(String line)
{	
	Output("\n");

	m_context->Info(Evaluate(m_context, m_lineToken, line, m_defines).GetString(), m_lineToken);		
}

// =================================================================
//	Evaluates a string expression.
// =================================================================
EvaluationResult CPreprocessor::Evaluate(CTranslationUnit* context, CToken& token, String expr, std::vector<CDefine>* defines)
{
	CTranslationUnit unit(context->GetCompiler(), "<eval>", *defines);
	bool result = unit.Evaluate(expr);	
	if (result == true)
	{
		try
		{
			EvaluationResult res = unit.GetASTRoot()->Evaluate(&unit);
			return res;
		}
		catch (std::runtime_error ex)
		{
			return EvaluationResult(false);
		}
	}
	else
	{
		context->FatalError("Invalid preprocessor expression.", token);
	}
	return EvaluationResult(result);
}
