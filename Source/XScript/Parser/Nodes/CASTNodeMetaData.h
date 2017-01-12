/* *****************************************************************

		CASTNodeMetaData.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CASTNODEMETADATA_H_
#define _CASTNODEMETADATA_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"

#include "XScript/Parser/Nodes/EvaluationResult.h"

#include "XScript/VirtualMachine/CSymbol.h"

class CSemanter;
class CTranslationUnit;
class CExpressionBaseASTNode;
class CDataType;

class CAliasASTNode;
class CDeclarationASTNode;
class CPackageASTNode;
class CClassASTNode;
class CClassMemberASTNode;

class CTranslator;

class CSymbol;

// =================================================================
//	Access level type.
// =================================================================
namespace CASTNodeMetaDataType
{
	enum Type
	{
		String,
		Int,
		Float
	};
};

// =================================================================
//	Base class used to store a representation of individual nodes
//	in an Abstract Syntax Tree.
// =================================================================
struct CASTNodeMetaData
{
public:
	CASTNodeMetaDataType::Type	type;
	String					name;

	String					string_value;
	int							int_value;
	float						float_value;
	
};

#endif