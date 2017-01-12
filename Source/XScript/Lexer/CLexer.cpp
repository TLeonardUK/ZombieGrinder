/* *****************************************************************

		CLexer.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "Generic/Types/String.h"
#include <stdio.h>
#include <assert.h>
#include <cstdlib>

#include "XScript/Compiler/CCompiler.h"
#include "XScript/Lexer/CLexer.h"
#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Helpers/CPathHelper.h"

#include "XScript/Compiler/CTranslationUnit.h"

// =================================================================
//	Processes input and performs the actions requested.
// =================================================================
bool CLexer::Process(CTranslationUnit* context)
{	
	unsigned int token_start	= 0;
	bool		 token_avail	= false;
	CToken		 token;

	int			 token_column	= 1;
	int			 token_row		= 1;

	String& source					= context->GetSource();
	String& file_path				= context->GetFilePath();
	std::vector<CToken>& token_list		= context->GetTokenList();

	// Guestimate how many tokens we will probably have. 
	// 1 token every 5 characters?
	token_list.reserve(source.length() / 5);

	while (token_start < source.length())
	{
		if (LexToken(context, source, file_path, token_start, token, token_column, token_row, token_avail))
		{
			if (token_avail == true)
			{
				token_list.push_back(token);
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

// =================================================================
//	Trys to find token mnemonic entry based on a string.
// =================================================================
bool CLexer::FindTokenMnemonicEntry(const String& name, TokenMnemonicTableEntry& output)
{
	int hash = StringHelper::Hash(name.c_str());

	int offset = 0;
	while (true)
	{
		TokenMnemonicTableEntry& entry = TOKEN_MNEMONIC_TABLE[offset];
		if (entry.Final == true)
		{
			return false;
		}

		if (entry.LiteralHash == hash)
		{
			output = entry; 
			return true;
		}

		offset++;
	}

	return false;
}

// =================================================================
//	Reads the next token at the given position.
// =================================================================
bool CLexer::LexToken(CTranslationUnit* context, 
						const String& source, 
						const String& path, 
						unsigned int& offset, 
						CToken& token, 
						int& token_column, 
						int& token_row, 
						bool& token_avail)
{
	char next_char		= source.at(offset);
	char la_char		= offset + 1 < source.size() ? source.at(offset + 1) : '\0';
	char la_la_char		= offset + 2 < source.size() ? source.at(offset + 2) : '\0';
	
	// ====================================================================================
	// Whitespace.
	// ====================================================================================
	if (next_char == ' ' ||
		next_char == '\t' || 
		next_char == '\n' ||
		next_char == '\r')
	{	
		offset++;	
		token_column++;

		if (next_char == '\n')
		{
			token_row++;
			token_column = 1;
		}	

		if (next_char == '\t')
		{
			token_column += 3;
		}

		token_avail = false;
		return true;
	}
	
	// ====================================================================================
	// Line Comment Start.
	// ====================================================================================
	else if (next_char == '/' && la_char == '/')				// // test
	{
		offset += 2;			
		token_column += 2;

		// Keep skipping till we get to the end of the file, or the next line.
		while (offset < source.size())
		{
			next_char = source.at(offset);
			if (next_char == '\n')
			{
				break;
			}			
			
			token_column++;
			offset++;
		}

		token_avail = false;
		return true;
	}

	// ====================================================================================
	// Block Comment Start.
	// ====================================================================================
	else if (next_char == '/' && la_char == '*')				// /* */
	{
		token_column += 2;
		offset += 2;
		
		int block_depth = 1;

		// Keep skipping till we get to the end of the file, or closing block.
		while (offset < source.size())
		{
			next_char = source.at(offset);
			la_char   = offset + 1 < source.size() ? source.at(offset + 1) : '\0';
			
			if (next_char == '/' &&
				la_char   == '*')
			{
				block_depth++;
				token_column += 2;
				offset += 2;
				continue;
			}

			else if (next_char == '*' &&
					 la_char   == '/')
			{
				block_depth--;
				token_column += 2;
				offset += 2;

				if (block_depth <= 0)
				{
					break;
				}
			}
			else if (next_char == '\n')
			{
				token_row++;
				token_column = 1;
				offset++;
			}	
			else if (next_char == '\t')
			{
				token_column += 3;
				offset++;
			}
			else
			{
				token_column++;
				offset++;
			}
		}

		token_avail = false;
		return true;
	}
	
	// ====================================================================================
	// String start.
	// ====================================================================================
	else if (next_char == '"' ||								// "herp derp"
			 next_char == '\'' ||								// 'herp derp'
			 (next_char == '@' && la_char == '"') ||			// @"herp derp"
			 (next_char == '@' && la_char == '\''))				// @'herp derp'
	{
		bool		escapable	= true;
		char		start_char	= next_char;
		String result		= "";

		if (next_char == '@')
		{			
			token_column++;	
			offset++;

			escapable = false;
			start_char = la_char;
		}
		
		token_column++;	
		offset++;
		
		// Keep skipping till we get to the end of the file, or closing block.
		while (offset < source.size())
		{
			next_char = source.at(offset);
			la_char   = offset + 1 < source.size() ? source.at(offset + 1) : '\0';
		
			if (next_char == start_char)
			{		
				token_column++;
				offset++;			
				break;
			}
			else if (next_char == '\\' && escapable == true)
			{
				token_column += 2;
				offset += 2;

				// All our delicious escape strings!
				switch (la_char)
				{
					case '\\': result += "\\"; break;
					case '\'': result += "'"; break;
					case '"': result += "\""; break;
					//case '0': result += "\0"; break;
					case 'a': result += "\a"; break;
					case 'b': result += "\b"; break;
					case 'f': result += "\f"; break;
					case 'n': result += "\n"; break;
					case 'r': result += "\r"; break;
					case 't': result += "\t"; break;
					case 'v': result += "\v"; break;
					case '?': result += "?"; break;

					// Hex escape strings. Format \Xnn
					case 'x': 
					case 'X': 
						{
							if (offset + 2 >= source.size())
							{
								context->FatalError("Invalid escape sequence, hexidecimal characters must be in the format \\Xnn.", path, token_row, token_column);
								return false;
							}
							else
							{
								next_char = source.at(offset);
								la_char   = source.at(offset + 1);

								token_column += 2;
								offset += 2;

								if (CStringHelper::IsHex(next_char) &&
									CStringHelper::IsHex(la_char))
								{
									String hexStr = (String("0x") + next_char) + la_char;
									result += (char)strtol(hexStr.c_str(), NULL, 16);
								}
								else
								{
									context->FatalError("Invalid hexidecimal escape sequence, hexidecimal characters must be 0-9 or A-F.", path, token_row, token_column);
									return false;
								}
							}
							break;
						}

					// ???
					default:
						{
							context->FatalError(String("Unexpected escape character in string '") + la_char + "'.", path, token_row, token_column);
							return false;
						}
				}
			}
			else
			{
				result += next_char;
	
				token_column++;
				offset++;			
			}
		}
		
		token.Type			= TokenIdentifier::STRING_LITERAL;
		token.SourceFile	= path;
		token.Row			= token_row;
		token.Column		= token_column;
		token.Literal		= result;

		token_avail = true;
		return true;
	}
	
	// ====================================================================================
	// Numeric value.
	// ====================================================================================
	else if (isdigit(next_char) || 
			 (next_char == '-' && (isdigit(la_char) || la_char == '.')) ||											// 0.123
			 (next_char == '.' && (isdigit(la_char) || (la_char == 'e' && (la_la_char == '-' || la_la_char == '+')))))	// .1231 .e-12
	{
		TokenIdentifier::Type numberType= TokenIdentifier::INT_LITERAL;
		String		result			= "";
		bool			isHex			= false;
		bool			isFloat			= false;
		bool			foundRadix		= false;
		bool			foundExp		= false;
		int				expPosition		= 0;
		int				numberCount		= 0;
		int				numberOffset    = 0;
		char			startChar		= source.at(offset);
		int				startOffset		= offset;
		bool			hasPostfix		= false;

		while (offset < source.size())
		{			
			next_char = source.at(offset);
			
			// Hex prefix 0X or 0x
			if ((next_char == 'x' || next_char == 'X') && numberOffset == 1 && startChar == '0')
			{
				isHex	 = true;		
			}

			// Floating point radix.
			else if (next_char == '.' && isHex == false && foundRadix == false && foundExp == false)
			{
				isFloat		 = true;			
				foundRadix	 = true;
				numberType	 = TokenIdentifier::FLOAT_LITERAL;
			}	

			// Exponent
			else if (next_char == 'e' && numberCount > 0 && isHex == false && foundExp == false)
			{
				isFloat		 = true;			
				foundExp 	 = true;
				expPosition  = numberOffset;
				numberType	 = TokenIdentifier::FLOAT_LITERAL;
			}	
			
			// Sign +/-
			else if ((next_char == '-' || next_char == '+') && (numberOffset == 0))
			{
				result += next_char;
			}	

			// Exponent sign +/-
			else if ((next_char == '-' || next_char == '+') && (foundExp == true && expPosition == numberOffset - 1))
			{
				result += next_char;
			}		

			// Hex digit.
			else if (((next_char >= 'A' && next_char <= 'F') || (next_char >= 'a' && next_char <= 'f')) && isHex == true)
			{
				numberCount++;
			}

			// Standard digit.
			else if (next_char >= '0' && next_char <= '9')
			{
				numberCount++;
			}
			
			// Character! Baaaad
			else if ((next_char >= 'A' && next_char <= 'Z') || (next_char >= 'a' && next_char <= 'z'))
			{
				if (next_char == 'f')
				{
					numberType	 = TokenIdentifier::FLOAT_LITERAL;
					isFloat		 = true;	
					hasPostfix   = true;
					break;
				}
				else
				{
					context->FatalError("Identifiers cannot start with numbers.", path, token_row, token_column);
					return false;
				}
			}

			// lolwut
			else
			{
				break;
			}

			numberOffset++;
			offset++;
		}

		token.Type			= numberType;
		token.SourceFile	= path;
		token.Row			= token_row;
		token.Column		= token_column;
		token.Literal		= source.substr(startOffset, offset-startOffset);

		token_column += (offset-startOffset);

		if (hasPostfix == true)
			offset++;

		token_avail = true;
		return true;
	}
	
	// ====================================================================================
	// Identifier/Keyword
	// ====================================================================================
	else if ((next_char >= 'A' && next_char <= 'Z') ||
			 (next_char >= 'a' && next_char <= 'z') ||
			 next_char == '_' ||
			 next_char == '@') 	
	{
		bool notKeyword = false;

		// Ignore the @, just stops us acting as a keyword.
		if (next_char == '@')
		{
			notKeyword = true;
			token_column++;	
			offset++;
		}

		int startOffset = offset;

		while (offset < source.size())
		{			
			next_char = source.at(offset);
			
			// A-Z, a-z, 0-9
			if ((next_char >= 'A' && next_char <= 'Z') ||
				(next_char >= 'a' && next_char <= 'z') ||
				(next_char >= '0' && next_char <= '9') ||
				next_char == '_')
			{
				// Ok!
			}

			// lolwut
			else
			{
				break;
			}

			offset++;
			token_column++;
		}
		
		String	result = source.substr(startOffset, offset-startOffset);

		TokenMnemonicTableEntry entry;
		if (notKeyword == false && FindTokenMnemonicEntry(result, entry))
		{
			token.Type		= entry.TokenType;
		}
		else
		{
			token.Type		= TokenIdentifier::IDENTIFIER;
		}
		token.SourceFile	= path;
		token.Row			= token_row;
		token.Column		= token_column;
		token.Literal		= result;

		token_avail = true;
		return true;
	}

	// ====================================================================================
	// Operator
	// ====================================================================================
	else
	{
		String				result = "";
		TokenMnemonicTableEntry mnemonic;
		int						startOffset = offset;


		while (offset < source.size())
		{
			next_char = source.at(offset);

			if (!FindTokenMnemonicEntry(result + next_char, mnemonic))
			{
				break;
			}

			result += next_char;

			offset++;			
			token_column++;
		}
		
		if (result != "")
		{
			token.Type			= mnemonic.TokenType;
			token.SourceFile	= path;
			token.Row			= token_row;
			token.Column		= token_column;
			token.Literal		= result;

			token_avail = true;
			return true;
		}
	}

	context->FatalError(String("Unexpected character in input '") + next_char + "'.", path, token_row, token_column);
	return false;
}