/* *****************************************************************

		CToken.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Lexer/CToken.h"

TokenMnemonicTableEntry TOKEN_MNEMONIC_TABLE[] = {
	TokenMnemonicTableEntry("abstract",		TokenIdentifier::KEYWORD_ABSTRACT),
	TokenMnemonicTableEntry("event",		TokenIdentifier::KEYWORD_EVENT),
	TokenMnemonicTableEntry("new",			TokenIdentifier::KEYWORD_NEW),
	TokenMnemonicTableEntry("struct",		TokenIdentifier::KEYWORD_STRUCT),
	TokenMnemonicTableEntry("as",			TokenIdentifier::KEYWORD_AS),
	TokenMnemonicTableEntry("explicit",		TokenIdentifier::KEYWORD_EXPLICIT),
	TokenMnemonicTableEntry("switch",		TokenIdentifier::KEYWORD_SWITCH),
	TokenMnemonicTableEntry("base",			TokenIdentifier::KEYWORD_BASE),
	TokenMnemonicTableEntry("extern",		TokenIdentifier::KEYWORD_EXTERN),
	TokenMnemonicTableEntry("operator",		TokenIdentifier::KEYWORD_OPERATOR),
	TokenMnemonicTableEntry("throw",		TokenIdentifier::KEYWORD_THROW),
	TokenMnemonicTableEntry("break",		TokenIdentifier::KEYWORD_BREAK),
	TokenMnemonicTableEntry("finally",		TokenIdentifier::KEYWORD_FINALLY),
	TokenMnemonicTableEntry("out",			TokenIdentifier::KEYWORD_OUT),
	TokenMnemonicTableEntry("in",			TokenIdentifier::KEYWORD_IN),
	TokenMnemonicTableEntry("fixed",		TokenIdentifier::KEYWORD_FIXED),
	TokenMnemonicTableEntry("override",		TokenIdentifier::KEYWORD_OVERRIDE),
	TokenMnemonicTableEntry("try",			TokenIdentifier::KEYWORD_TRY),
	TokenMnemonicTableEntry("case",			TokenIdentifier::KEYWORD_CASE),
	TokenMnemonicTableEntry("params",		TokenIdentifier::KEYWORD_PARAMS),
	TokenMnemonicTableEntry("typeof",		TokenIdentifier::KEYWORD_TYPEOF),
	TokenMnemonicTableEntry("indexof",		TokenIdentifier::KEYWORD_INDEXOF),
	TokenMnemonicTableEntry("catch",		TokenIdentifier::KEYWORD_CATCH),
	TokenMnemonicTableEntry("for",			TokenIdentifier::KEYWORD_FOR),
	TokenMnemonicTableEntry("private",		TokenIdentifier::KEYWORD_PRIVATE),
	TokenMnemonicTableEntry("public",		TokenIdentifier::KEYWORD_PUBLIC),
	TokenMnemonicTableEntry("foreach",		TokenIdentifier::KEYWORD_FOREACH),
	TokenMnemonicTableEntry("protected",	TokenIdentifier::KEYWORD_PROTECTED),
	TokenMnemonicTableEntry("checked",		TokenIdentifier::KEYWORD_CHECKED),
	TokenMnemonicTableEntry("goto",			TokenIdentifier::KEYWORD_GOTO),
	TokenMnemonicTableEntry("forceoffset",	TokenIdentifier::KEYWORD_FORCEOFFSET),
	TokenMnemonicTableEntry("unchecked",	TokenIdentifier::KEYWORD_UNCHECKED),
	TokenMnemonicTableEntry("class",		TokenIdentifier::KEYWORD_CLASS),
	TokenMnemonicTableEntry("if",			TokenIdentifier::KEYWORD_IF),
	TokenMnemonicTableEntry("readonly",		TokenIdentifier::KEYWORD_READONLY),
	TokenMnemonicTableEntry("unsafe",		TokenIdentifier::KEYWORD_UNSAFE),
	TokenMnemonicTableEntry("const",		TokenIdentifier::KEYWORD_CONST),
	TokenMnemonicTableEntry("implicit",		TokenIdentifier::KEYWORD_IMPLICIT),
	TokenMnemonicTableEntry("ref",			TokenIdentifier::KEYWORD_REF),
	TokenMnemonicTableEntry("continue",		TokenIdentifier::KEYWORD_CONTINUE),
	TokenMnemonicTableEntry("return",		TokenIdentifier::KEYWORD_RETURN),
	TokenMnemonicTableEntry("using",		TokenIdentifier::KEYWORD_USING),
	TokenMnemonicTableEntry("virtual",		TokenIdentifier::KEYWORD_VIRTUAL),
	TokenMnemonicTableEntry("default",		TokenIdentifier::KEYWORD_DEFAULT),
	TokenMnemonicTableEntry("interface",	TokenIdentifier::KEYWORD_INTERFACE),
	TokenMnemonicTableEntry("sealed",		TokenIdentifier::KEYWORD_SEALED),
	TokenMnemonicTableEntry("volatile",		TokenIdentifier::KEYWORD_VOLATILE),
	TokenMnemonicTableEntry("delegate",		TokenIdentifier::KEYWORD_DELEGATE),
	TokenMnemonicTableEntry("internal",		TokenIdentifier::KEYWORD_INTERNAL),
	TokenMnemonicTableEntry("do",			TokenIdentifier::KEYWORD_DO),
	TokenMnemonicTableEntry("is",			TokenIdentifier::KEYWORD_IS),
	TokenMnemonicTableEntry("sizeof",		TokenIdentifier::KEYWORD_SIZEOF),
	TokenMnemonicTableEntry("lock",			TokenIdentifier::KEYWORD_LOCK),
	TokenMnemonicTableEntry("stackalloc",	TokenIdentifier::KEYWORD_STACKALLOC),
	TokenMnemonicTableEntry("else",			TokenIdentifier::KEYWORD_ELSE),
	TokenMnemonicTableEntry("static",		TokenIdentifier::KEYWORD_STATIC),
	TokenMnemonicTableEntry("enum",			TokenIdentifier::KEYWORD_ENUM),
	TokenMnemonicTableEntry("namespace",	TokenIdentifier::KEYWORD_NAMESPACE),
	TokenMnemonicTableEntry("module",		TokenIdentifier::KEYWORD_MODULE),
	TokenMnemonicTableEntry("package",		TokenIdentifier::KEYWORD_PACKAGE),
	TokenMnemonicTableEntry("import",		TokenIdentifier::KEYWORD_IMPORT),
	TokenMnemonicTableEntry("include",		TokenIdentifier::KEYWORD_INCLUDE),
	TokenMnemonicTableEntry("end",			TokenIdentifier::KEYWORD_END),
	TokenMnemonicTableEntry("generator",	TokenIdentifier::KEYWORD_GENERATOR),
	TokenMnemonicTableEntry("while",		TokenIdentifier::KEYWORD_WHILE),
	TokenMnemonicTableEntry("native",		TokenIdentifier::KEYWORD_NATIVE),
	TokenMnemonicTableEntry("box",			TokenIdentifier::KEYWORD_BOX),
	TokenMnemonicTableEntry("copy",			TokenIdentifier::KEYWORD_COPY),
	TokenMnemonicTableEntry("library",		TokenIdentifier::KEYWORD_LIBRARY),

	TokenMnemonicTableEntry("property",		TokenIdentifier::KEYWORD_PROPERTY),
	TokenMnemonicTableEntry("serialized",	TokenIdentifier::KEYWORD_SERIALIZED),
	
	TokenMnemonicTableEntry("state",		TokenIdentifier::KEYWORD_STATE),
	TokenMnemonicTableEntry("change_state",	TokenIdentifier::KEYWORD_CHANGE_STATE),
	TokenMnemonicTableEntry("push_state",	TokenIdentifier::KEYWORD_PUSH_STATE),
	TokenMnemonicTableEntry("pop_state",	TokenIdentifier::KEYWORD_POP_STATE),

	TokenMnemonicTableEntry("rpc",			TokenIdentifier::KEYWORD_RPC),

	TokenMnemonicTableEntry("replicate",	TokenIdentifier::KEYWORD_REPLICATE),
	TokenMnemonicTableEntry("components",	TokenIdentifier::KEYWORD_COMPONENTS),

	TokenMnemonicTableEntry("this",			TokenIdentifier::KEYWORD_THIS),
	TokenMnemonicTableEntry("true",			TokenIdentifier::KEYWORD_TRUE),
	TokenMnemonicTableEntry("false",		TokenIdentifier::KEYWORD_FALSE),
	TokenMnemonicTableEntry("null",			TokenIdentifier::KEYWORD_NULL),

//	TokenMnemonicTableEntry("object",		TokenIdentifier::KEYWORD_OBJECT),
	TokenMnemonicTableEntry("bool",			TokenIdentifier::KEYWORD_BOOL),
	TokenMnemonicTableEntry("void",			TokenIdentifier::KEYWORD_VOID),
	//TokenMnemonicTableEntry("byte",			TokenIdentifier::KEYWORD_BYTE),
	//TokenMnemonicTableEntry("short",		TokenIdentifier::KEYWORD_SHORT),
	TokenMnemonicTableEntry("int",			TokenIdentifier::KEYWORD_INT),
	//TokenMnemonicTableEntry("long",			TokenIdentifier::KEYWORD_LONG),
	TokenMnemonicTableEntry("float",		TokenIdentifier::KEYWORD_FLOAT),
	//TokenMnemonicTableEntry("double",		TokenIdentifier::KEYWORD_DOUBLE),
	TokenMnemonicTableEntry("string",		TokenIdentifier::KEYWORD_STRING),

	TokenMnemonicTableEntry("!",			TokenIdentifier::OP_LOGICAL_NOT),		// !
	TokenMnemonicTableEntry("||",			TokenIdentifier::OP_LOGICAL_OR),		// ||
	TokenMnemonicTableEntry("&&",			TokenIdentifier::OP_LOGICAL_AND),		// &&
	
	TokenMnemonicTableEntry("@",			TokenIdentifier::OP_AT),				// @

	TokenMnemonicTableEntry("<<",			TokenIdentifier::OP_SHL),				// <<
	TokenMnemonicTableEntry(">>",			TokenIdentifier::OP_SHR),				// >>
	TokenMnemonicTableEntry("&",			TokenIdentifier::OP_AND),				// &
	TokenMnemonicTableEntry("|",			TokenIdentifier::OP_OR),				// |
	TokenMnemonicTableEntry("^",			TokenIdentifier::OP_XOR),				// ^
	TokenMnemonicTableEntry("~",			TokenIdentifier::OP_NOT),				// ~
	TokenMnemonicTableEntry("%",			TokenIdentifier::OP_MOD),				// %
	TokenMnemonicTableEntry("+",			TokenIdentifier::OP_ADD),				// +
	TokenMnemonicTableEntry("-",			TokenIdentifier::OP_SUB),				// -
	TokenMnemonicTableEntry("/",			TokenIdentifier::OP_DIV),				// /
	TokenMnemonicTableEntry("*",			TokenIdentifier::OP_MUL),				// *
	TokenMnemonicTableEntry("?",			TokenIdentifier::OP_TERNARY),				// ?

	TokenMnemonicTableEntry("==",			TokenIdentifier::OP_EQUAL),			// ==
	TokenMnemonicTableEntry("!=",			TokenIdentifier::OP_NOT_EQUAL),		// !=
	TokenMnemonicTableEntry(">=",			TokenIdentifier::OP_GREATER_EQUAL),	// >=
	TokenMnemonicTableEntry("<=",			TokenIdentifier::OP_LESS_EQUAL),		// <=
	TokenMnemonicTableEntry(">",			TokenIdentifier::OP_GREATER),			// >
	TokenMnemonicTableEntry("<",			TokenIdentifier::OP_LESS),			// <

	TokenMnemonicTableEntry("=",			TokenIdentifier::OP_ASSIGN),			// ==
	TokenMnemonicTableEntry("-=",			TokenIdentifier::OP_ASSIGN_SUB),		// -=
	TokenMnemonicTableEntry("+=",			TokenIdentifier::OP_ASSIGN_ADD),		// +=
	TokenMnemonicTableEntry("*=",			TokenIdentifier::OP_ASSIGN_MUL),		// *=
	TokenMnemonicTableEntry("/=",			TokenIdentifier::OP_ASSIGN_DIV),		// /=
	TokenMnemonicTableEntry("%=",			TokenIdentifier::OP_ASSIGN_MOD),		// %=
	TokenMnemonicTableEntry("&=",			TokenIdentifier::OP_ASSIGN_AND),		// &=
	TokenMnemonicTableEntry("|=",			TokenIdentifier::OP_ASSIGN_OR),		// |=
	TokenMnemonicTableEntry("^=",			TokenIdentifier::OP_ASSIGN_XOR),		// ^=
	TokenMnemonicTableEntry("<<=",			TokenIdentifier::OP_ASSIGN_SHL),		// <<=
	TokenMnemonicTableEntry(">>=",			TokenIdentifier::OP_ASSIGN_SHR),		// >>=

	TokenMnemonicTableEntry("++",			TokenIdentifier::OP_INCREMENT),		// ++
	TokenMnemonicTableEntry("--",			TokenIdentifier::OP_DECREMENT),		// --

	TokenMnemonicTableEntry("[",			TokenIdentifier::OPEN_BRACKET),		// [
	TokenMnemonicTableEntry("]",			TokenIdentifier::CLOSE_BRACKET),	// ]
	TokenMnemonicTableEntry("{",			TokenIdentifier::OPEN_BRACE),		// {
	TokenMnemonicTableEntry("}",			TokenIdentifier::CLOSE_BRACE),		// }
	TokenMnemonicTableEntry("(",			TokenIdentifier::OPEN_PARENT),		// (
	TokenMnemonicTableEntry(")",			TokenIdentifier::CLOSE_PARENT),		// )

	TokenMnemonicTableEntry(",",			TokenIdentifier::SINGLE_COMMA),			// ,
	TokenMnemonicTableEntry(":",			TokenIdentifier::COLON),			// :
	TokenMnemonicTableEntry(";",			TokenIdentifier::SEMICOLON),		// ;
	TokenMnemonicTableEntry(".",			TokenIdentifier::PERIOD),			// .
	TokenMnemonicTableEntry("..",			TokenIdentifier::SLICE),			// ..
	TokenMnemonicTableEntry("::",			TokenIdentifier::SCOPE),			// ::

	// End of table.
	TokenMnemonicTableEntry("",				(TokenIdentifier::Type)0)
};