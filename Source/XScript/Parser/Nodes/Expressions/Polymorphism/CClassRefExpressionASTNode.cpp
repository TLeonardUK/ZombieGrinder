/* *****************************************************************

		CClassRefExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CClassRefExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassRefExpressionASTNode::CClassRefExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassRefExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CClassRefExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Make sure we are inside a method and 
	CClassASTNode* class_scope = this->FindClassScope(semanter);
	ExpressionResultType = new CClassReferenceDataType(Token, class_scope);

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassRefExpressionASTNode::Clone(CSemanter* semanter)
{
	CClassRefExpressionASTNode* clone = new CClassRefExpressionASTNode(NULL, Token);
	
	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CClassRefExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateClassRefExpression(this);
}