/* *****************************************************************

		CBaseExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CBaseExpressionASTNode.h"
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
CBaseExpressionASTNode::CBaseExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CBaseExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CBaseExpressionASTNode");

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
		semanter->GetContext()->FatalError("base keyword can only be used in class methods.", Token);		
	}
	if (method_scope->IsStatic == true)
	{
		semanter->GetContext()->FatalError("base keyword cannot be used in static methods.", Token);
	}
	if (class_scope->SuperClass == NULL)
	{
		semanter->GetContext()->FatalError("base keyword cannot be used in class without super class.", Token);
	}

	ExpressionResultType = class_scope->SuperClass->ObjectDataType;

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CBaseExpressionASTNode::Clone(CSemanter* semanter)
{
	CBaseExpressionASTNode* clone = new CBaseExpressionASTNode(NULL, Token);

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CBaseExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateBaseExpression(this);
}