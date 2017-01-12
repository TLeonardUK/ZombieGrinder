/* *****************************************************************

		CMethodBodyASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/TopLevel/CMethodBodyASTNode.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CMethodBodyASTNode::CMethodBodyASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{

}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CMethodBodyASTNode::Clone(CSemanter* semanter)
{
	CMethodBodyASTNode* clone = new CMethodBodyASTNode(NULL, Token);

	CloneChildren(semanter, clone);

	return clone;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CMethodBodyASTNode::Semant(CSemanter* semanter)
{
	SEMANT_TRACE("CMethodBodyASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	SemantChildren(semanter);

	return this;
}