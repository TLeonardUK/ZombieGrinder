/* *****************************************************************

		CLogicalExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Math/CLogicalExpressionASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CLogicalExpressionASTNode::CLogicalExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CLogicalExpressionASTNode::Clone(CSemanter* semanter)
{
	CLogicalExpressionASTNode* clone = new CLogicalExpressionASTNode(NULL, Token);

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
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CLogicalExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CLogicalExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Semant expressions.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));
	RightValue = ReplaceChild(RightValue, RightValue->Semant(semanter)); 

	// Get expression references.
	CExpressionBaseASTNode* leftValueBase  = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* rightValueBase = dynamic_cast<CExpressionBaseASTNode*>(RightValue);

	// Cast to resulting expression.
	ExpressionResultType = new CBoolDataType(Token);
	LeftValue  = ReplaceChild(LeftValue,  leftValueBase->CastTo(semanter, ExpressionResultType, Token, true));
	RightValue = ReplaceChild(RightValue, rightValueBase->CastTo(semanter, ExpressionResultType, Token, true)); 

	return this;
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CLogicalExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	EvaluationResult leftResult  = LeftValue->Evaluate(unit);
	EvaluationResult rightResult = RightValue->Evaluate(unit);

	switch (Token.Type)
	{		
		case TokenIdentifier::OP_LOGICAL_AND:		return EvaluationResult(leftResult.GetBool() && rightResult.GetBool()); 
		case TokenIdentifier::OP_LOGICAL_OR:		return EvaluationResult(leftResult.GetBool() && rightResult.GetBool());
	}
	
	unit->FatalError("Invalid constant operation.", Token);
	return EvaluationResult(false);
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CLogicalExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateLogicalExpression(this);
}