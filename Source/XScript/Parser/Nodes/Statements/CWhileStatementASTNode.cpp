/* *****************************************************************

		CWhileStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CWhileStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Translator/CTranslator.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CWhileStatementASTNode::CWhileStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token),
	BodyStatement(NULL),
	ExpressionStatement(NULL)
{
}

// =================================================================
//	Returns true if this node can accept break statements inside
//	of it.
// =================================================================
bool CWhileStatementASTNode::AcceptBreakStatement()
{
	return true;
}

// =================================================================
//	Returns true if this node can accept continue statements inside
//	of it.
// =================================================================
bool CWhileStatementASTNode::AcceptContinueStatement()
{
	return true;
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CWhileStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CWhileStatementASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Semant the expression.
	ExpressionStatement = dynamic_cast<CExpressionBaseASTNode*>(ReplaceChild(ExpressionStatement, ExpressionStatement->Semant(semanter)));
	ExpressionStatement = dynamic_cast<CExpressionBaseASTNode*>(ReplaceChild(ExpressionStatement, ExpressionStatement->CastTo(semanter, new CBoolDataType(Token), Token)));

	// Semant Body statement.
	if (BodyStatement != NULL)
	{
		BodyStatement = ReplaceChild(BodyStatement, BodyStatement->Semant(semanter));
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CWhileStatementASTNode::Clone(CSemanter* semanter)
{
	CWhileStatementASTNode* clone = new CWhileStatementASTNode(NULL, Token);
	
	if (ExpressionStatement != NULL)
	{
		clone->ExpressionStatement = dynamic_cast<CExpressionASTNode*>(ExpressionStatement->Clone(semanter));
		clone->AddChild(clone->ExpressionStatement);
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
CASTNode* CWhileStatementASTNode::FindLoopScope(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CWhileStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateWhileStatement(this);
}