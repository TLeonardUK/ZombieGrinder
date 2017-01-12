/* *****************************************************************

		CThisExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CThisExpressionASTNode::CThisExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CThisExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CThisExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Make sure we are inside a method and 
	CClassASTNode*		 class_scope = this->FindClassScope(semanter);
	CClassMemberASTNode* method_scope = this->FindClassMethodScope(semanter);

	if (method_scope == NULL ||
		class_scope  == NULL)
	{
		semanter->GetContext()->FatalError("this keyword can only be used in class methods.", Token);		
	}
	if (method_scope->IsStatic == true)
	{
		semanter->GetContext()->FatalError("this keyword cannot be used in static methods.", Token);
	}

	CDeclarationIdentifier s_string_declaration("string");

	if (class_scope->Identifier == s_string_declaration)
	{
		ExpressionResultType = new CStringDataType(Token);
	}
	else
	{
		ExpressionResultType = class_scope->ObjectDataType;
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CThisExpressionASTNode::Clone(CSemanter* semanter)
{
	CThisExpressionASTNode* clone = new CThisExpressionASTNode(NULL, Token);
	

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CThisExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateThisExpression(this);
}