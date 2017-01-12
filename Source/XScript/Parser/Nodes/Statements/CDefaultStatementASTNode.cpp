/* *****************************************************************

		CDefaultStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CDefaultStatementASTNode.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CDefaultStatementASTNode::CDefaultStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token),
	BodyStatement(NULL)
{
}

// =================================================================
//	Returns true if this node can accept break statements inside
//	of it.
// =================================================================
bool CDefaultStatementASTNode::AcceptBreakStatement()
{
	return true;
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CDefaultStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CDefaultStatementASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;
	
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
CASTNode* CDefaultStatementASTNode::Clone(CSemanter* semanter)
{
	CDefaultStatementASTNode* clone = new CDefaultStatementASTNode(NULL, Token);
	
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
CASTNode* CDefaultStatementASTNode::FindLoopScope(CSemanter* semanter)
{
	return this;
}