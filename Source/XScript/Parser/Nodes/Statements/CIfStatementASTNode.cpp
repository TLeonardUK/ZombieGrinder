/* *****************************************************************

		CIfStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CIfStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Translator/CTranslator.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CIfStatementASTNode::CIfStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token),
	BodyStatement(NULL),
	ElseStatement(NULL),
	ExpressionStatement(NULL)
{
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CIfStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CIfStatementASTNode");

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

	// Semant else statement.
	if (ElseStatement != NULL)
	{
		ElseStatement = ReplaceChild(ElseStatement, ElseStatement->Semant(semanter));
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CIfStatementASTNode::Clone(CSemanter* semanter)
{
	CIfStatementASTNode* clone = new CIfStatementASTNode(NULL, Token);
	
	if (ExpressionStatement != NULL)
	{
		clone->ExpressionStatement = dynamic_cast<CExpressionASTNode*>(ExpressionStatement->Clone(semanter));
		clone->AddChild(clone->ExpressionStatement);
	}
	if (BodyStatement != NULL)
	{
		clone->BodyStatement = BodyStatement->Clone(semanter);
		clone->AddChild(clone->BodyStatement);
	}
	if (ElseStatement != NULL)
	{
		clone->ElseStatement = ElseStatement->Clone(semanter);
		clone->AddChild(clone->ElseStatement);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CIfStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateIfStatement(this);
}