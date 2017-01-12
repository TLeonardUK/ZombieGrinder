/* *****************************************************************

		CLexer.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CLEXER_H_
#define _CLEXER_H_

#include "XScript/Lexer/CToken.h"

class CCompiler;
class CTranslationUnit;

// =================================================================
//	Class deals with process input and compiling the correct
//	files as requested.
// =================================================================
class CLexer
{
private:
	inline bool FindTokenMnemonicEntry(const String& name, TokenMnemonicTableEntry& output);
	inline bool LexToken(CTranslationUnit* context, 
					      const String& source, 
						  const String& path, 
						  unsigned int& offset, 
						  CToken& token, 
						  int& token_column, 
						  int& token_row, 
						  bool& token_avail);

public:
	bool Process(CTranslationUnit* context);

};

#endif

