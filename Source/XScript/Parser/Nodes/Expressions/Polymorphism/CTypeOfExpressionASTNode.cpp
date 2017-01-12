/* *****************************************************************

		CTypeOfExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CTypeOfExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CTypeOfExpressionASTNode::CTypeOfExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token)
	, Type(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CTypeOfExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CTypeOfExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Grab data type.
	Type = Type->Semant(semanter, this);

	CDeclarationIdentifier s_type_ident("Type");

	// Result type is type of "Type"
	ExpressionResultType = FindDataType(semanter, s_type_ident, std::vector<CDataType*>(), true, false);

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CTypeOfExpressionASTNode::Clone(CSemanter* semanter)
{
	CTypeOfExpressionASTNode* clone = new CTypeOfExpressionASTNode(NULL, Token);
	clone->Type = Type;

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CTypeOfExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateTypeOfExpression(this);
}