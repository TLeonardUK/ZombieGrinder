/* *****************************************************************

		CForStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CForStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Translator/CTranslator.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CForStatementASTNode::CForStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token),
	InitialStatement(NULL),
	ConditionExpression(NULL),
	IncrementExpression(NULL),
	BodyStatement(NULL)
{
}

// =================================================================
//	Returns true if this node can accept break statements inside
//	of it.
// =================================================================
bool CForStatementASTNode::AcceptBreakStatement()
{
	return true;
}

// =================================================================
//	Returns true if this node can accept continue statements inside
//	of it.
// =================================================================
bool CForStatementASTNode::AcceptContinueStatement()
{
	return true;
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CForStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CForStatementASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	if (InitialStatement != NULL)
	{		
		InitialStatement = dynamic_cast<CASTNode*>(InitialStatement->Semant(semanter));
	}

	if (ConditionExpression != NULL)
	{	
		ConditionExpression = dynamic_cast<CExpressionBaseASTNode*>(ConditionExpression->Semant(semanter));
		ConditionExpression = dynamic_cast<CExpressionBaseASTNode*>(ConditionExpression->CastTo(semanter, new CBoolDataType(Token), Token));
	}

	if (IncrementExpression != NULL)
	{
		IncrementExpression = dynamic_cast<CExpressionBaseASTNode*>(IncrementExpression->Semant(semanter));
	}

	if (BodyStatement != NULL)
	{
		BodyStatement = BodyStatement->Semant(semanter);
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CForStatementASTNode::Clone(CSemanter* semanter)
{
	CForStatementASTNode* clone = new CForStatementASTNode(NULL, Token);
	
	if (InitialStatement != NULL)
	{
		clone->InitialStatement = dynamic_cast<CASTNode*>(InitialStatement->Clone(semanter));
		clone->AddChild(clone->InitialStatement);
	}
	if (ConditionExpression != NULL)
	{
		clone->ConditionExpression = dynamic_cast<CExpressionASTNode*>(ConditionExpression->Clone(semanter));
		clone->AddChild(clone->ConditionExpression);
	}
	if (IncrementExpression != NULL)
	{
		clone->IncrementExpression = dynamic_cast<CExpressionASTNode*>(IncrementExpression->Clone(semanter));
		clone->AddChild(clone->IncrementExpression);
	}
	if (BodyStatement != NULL)
	{
		clone->BodyStatement = dynamic_cast<CASTNode*>(BodyStatement->Clone(semanter));
		clone->AddChild(clone->BodyStatement);
	}

	return clone;
}

// =================================================================
//	Finds the scope the looping statement this node is contained by.
// =================================================================
CASTNode* CForStatementASTNode::FindLoopScope(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CForStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateForStatement(this);
}