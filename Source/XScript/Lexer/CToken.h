/* *****************************************************************

		CToken.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CTOKEN_H_
#define _CTOKEN_H_

#include "Generic/Types/String.h"
#include "Generic/Helper//StringHelper.h"
#include <vector>

// =================================================================
//	Enumeration of different token types.
// =================================================================
namespace TokenIdentifier
{
	enum Type
	{
		KEYWORD_ABSTRACT,
		KEYWORD_EVENT,
		KEYWORD_NEW,
		KEYWORD_STRUCT,
		KEYWORD_AS,
		KEYWORD_EXPLICIT,
		KEYWORD_SWITCH,
		KEYWORD_BASE,
		KEYWORD_EXTERN,
		KEYWORD_OPERATOR,
		KEYWORD_THROW,
		KEYWORD_BREAK,
		KEYWORD_FINALLY,
		KEYWORD_OUT,
		KEYWORD_IN,
		KEYWORD_FIXED,
		KEYWORD_OVERRIDE,
		KEYWORD_TRY,
		KEYWORD_CASE,
		KEYWORD_PARAMS,
		KEYWORD_TYPEOF,
		KEYWORD_CATCH,
		KEYWORD_FOR,
		KEYWORD_PRIVATE,
		KEYWORD_PUBLIC,
		KEYWORD_FOREACH,
		KEYWORD_PROTECTED,
		KEYWORD_CHECKED,
		KEYWORD_GOTO,
		KEYWORD_UNCHECKED,
		KEYWORD_CLASS,
		KEYWORD_IF,
		KEYWORD_READONLY,
		KEYWORD_UNSAFE,
		KEYWORD_CONST,
		KEYWORD_IMPLICIT,
		KEYWORD_REF,
		KEYWORD_CONTINUE,
		KEYWORD_RETURN,
		KEYWORD_USING,
		KEYWORD_VIRTUAL,
		KEYWORD_DEFAULT,
		KEYWORD_INTERFACE,
		KEYWORD_SEALED,
		KEYWORD_VOLATILE,
		KEYWORD_DELEGATE,
		KEYWORD_INTERNAL,
		KEYWORD_DO,
		KEYWORD_IS,
		KEYWORD_SIZEOF,
		KEYWORD_INDEXOF,
		KEYWORD_LOCK,
		KEYWORD_STACKALLOC,
		KEYWORD_ELSE,
		KEYWORD_STATIC,
		KEYWORD_ENUM,
		KEYWORD_NAMESPACE,
		KEYWORD_MODULE,
		KEYWORD_PACKAGE,
		KEYWORD_IMPORT,
		KEYWORD_INCLUDE,
		KEYWORD_END,
		KEYWORD_GENERATOR,
		KEYWORD_WHILE,
		KEYWORD_NATIVE,
		KEYWORD_BOX,
		KEYWORD_FORCEOFFSET,

		KEYWORD_SERIALIZED,		
		KEYWORD_PROPERTY,

		KEYWORD_STATE,
		KEYWORD_CHANGE_STATE,
		KEYWORD_PUSH_STATE,
		KEYWORD_POP_STATE,

		KEYWORD_RPC,

		KEYWORD_REPLICATE,
		KEYWORD_COMPONENTS,

		KEYWORD_COPY,
		KEYWORD_LIBRARY,

		KEYWORD_THIS,
		KEYWORD_TRUE,
		KEYWORD_FALSE,
		KEYWORD_NULL,

		//KEYWORD_OBJECT,
		KEYWORD_BOOL,
		KEYWORD_VOID,
		//KEYWORD_BYTE,
		//KEYWORD_SHORT,
		KEYWORD_INT,
		//KEYWORD_LONG,
		KEYWORD_FLOAT,
		//KEYWORD_DOUBLE,
		KEYWORD_STRING,

		OP_LOGICAL_NOT,		// !
		OP_LOGICAL_OR,		// ||
		OP_LOGICAL_AND,		// &&
		
		OP_AT,				// @
		
		OP_SHL,				// <<
		OP_SHR,				// >>
		OP_AND,				// &
		OP_OR,				// |
		OP_XOR,				// ^
		OP_NOT,				// ~
		OP_MOD,				// %
		OP_ADD,				// +
		OP_SUB,				// -
		OP_DIV,				// /
		OP_MUL,				// *
		OP_TERNARY,			// ?

		OP_EQUAL,			// ==
		OP_NOT_EQUAL,		// !=
		OP_GREATER_EQUAL,	// >=
		OP_LESS_EQUAL,		// <=
		OP_GREATER,			// >
		OP_LESS,			// <

		OP_ASSIGN,			// ==
		OP_ASSIGN_SUB,		// -=
		OP_ASSIGN_ADD,		// +=
		OP_ASSIGN_MUL,		// *=
		OP_ASSIGN_DIV,		// /=
		OP_ASSIGN_MOD,		// %=
		OP_ASSIGN_AND,		// &=
		OP_ASSIGN_OR,		// |=
		OP_ASSIGN_XOR,		// ^=
		OP_ASSIGN_SHL,		// <<=
		OP_ASSIGN_SHR,		// >>=

		OP_INCREMENT,		// ++
		OP_DECREMENT,		// --

		OPEN_BRACKET,		// [
		CLOSE_BRACKET,		// ]
		OPEN_BRACE,			// {
		CLOSE_BRACE,		// }
		OPEN_PARENT,		// (
		CLOSE_PARENT,		// )

		SINGLE_COMMA,		// ,
		COLON,				// :
		SEMICOLON,			// ;
		PERIOD,				// .
		SLICE,				// ..
		SCOPE,				// ::

		IDENTIFIER,			// herpderp
		STRING_LITERAL,		// "avalue"
		FLOAT_LITERAL,		// 0.1337
		INT_LITERAL,		// 1337

		EndOfFile,			// <eof>
		StartOfFile,		// <sof>
		PreProcessor,		// #asdasd
	};
};

// =================================================================
//	Enumeration of different token types.
// =================================================================
struct TokenMnemonicTableEntry
{
public:
	bool					Final;
	String					 Literal;
	int						LiteralHash;
	TokenIdentifier::Type	TokenType;

	TokenMnemonicTableEntry() 
	{
	}
	TokenMnemonicTableEntry(String lit, TokenIdentifier::Type type)
	{
		Literal = lit;
		Final = (lit == "");
		LiteralHash = StringHelper::Hash(lit.c_str());
		TokenType = type;
	}
};

extern TokenMnemonicTableEntry TOKEN_MNEMONIC_TABLE[];

// =================================================================
//	Struct stores information on an individual token.
// =================================================================
struct CToken
{
public:
	String				Literal;
	TokenIdentifier::Type	Type;

	String		SourceFile;
	int				Row;
	int				Column;
};

#endif