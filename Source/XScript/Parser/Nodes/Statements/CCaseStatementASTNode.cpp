/* *****************************************************************

		CCaseStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CSwitchStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CCaseStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CCaseStatementASTNode::CCaseStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token),
	BodyStatement(NULL)
{
}

// =================================================================
//	Returns true if this node can accept break statements inside
//	of it.
// =================================================================
bool CCaseStatementASTNode::AcceptBreakStatement()
{
	return true;
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CCaseStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CCaseStatementASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Find the switch statement we are part of.
	CASTNode* scope = this;
	CSwitchStatementASTNode* switchScope = NULL;

	while (scope != NULL && switchScope == NULL)
	{
		switchScope = dynamic_cast<CSwitchStatementASTNode*>(scope);
		scope = scope->Parent;
	}

	// Semant the expression.
	for (std::vector<CASTNode*>::iterator iter = Expressions.begin(); iter != Expressions.end(); iter++)
	{
		CASTNode* node = *iter;
		node = ReplaceChild(node, node->Semant(semanter));
		node = ReplaceChild(node, dynamic_cast<CExpressionBaseASTNode*>(node)->CastTo(semanter, switchScope->ExpressionStatement->ExpressionResultType, Token));
		(*iter) = node;
	}
	
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
CASTNode* CCaseStatementASTNode::Clone(CSemanter* semanter)
{
	CCaseStatementASTNode* clone = new CCaseStatementASTNode(NULL, Token);
	
	if (BodyStatement != NULL)
	{
		clone->BodyStatement = dynamic_cast<CASTNode*>(BodyStatement->Clone(semanter));
		clone->AddChild(clone->BodyStatement);
	}
	
	for (std::vector<CASTNode*>::iterator iter = Expressions.begin(); iter != Expressions.end(); iter++)
	{
		CASTNode* node = (*iter)->Clone(semanter);
		clone->Expressions.push_back(node);
		clone->AddChild(clone);
	}

	return clone;
}

// =================================================================
//	Finds the scope the looping statement this node is contained by.
// =================================================================
CASTNode* CCaseStatementASTNode::FindLoopScope(CSemanter* semanter)
{
	return this;
}