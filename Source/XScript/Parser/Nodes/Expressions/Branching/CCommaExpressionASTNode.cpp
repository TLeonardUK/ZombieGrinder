/* *****************************************************************

		CCommaExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Branching/CCommaExpressionASTNode.h"

#include "XScript/Translator/CTranslator.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CCommaExpressionASTNode::CCommaExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CCommaExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CCommaExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;
	
	// Semant expressions.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));
	RightValue = ReplaceChild(RightValue, RightValue->Semant(semanter)); 

	// Resulting type is always our right hand type.
	ExpressionResultType = dynamic_cast<CExpressionBaseASTNode*>(RightValue)->ExpressionResultType;

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CCommaExpressionASTNode::Clone(CSemanter* semanter)
{
	CCommaExpressionASTNode* clone = new CCommaExpressionASTNode(NULL, Token);

	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}
	if (RightValue != NULL)
	{
		clone->RightValue = dynamic_cast<CASTNode*>(RightValue->Clone(semanter));
		clone->AddChild(clone->RightValue);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CCommaExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateCommaExpression(this);
}