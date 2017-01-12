/* *****************************************************************

		CClassBodyASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/TopLevel/CClassBodyASTNode.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassBodyASTNode::CClassBodyASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{

}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassBodyASTNode::Semant(CSemanter* semanter)
{
	SEMANT_TRACE("CClassBodyASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	SemantChildren(semanter);
	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassBodyASTNode::Clone(CSemanter* semanter)
{
	CClassBodyASTNode* clone = new CClassBodyASTNode(NULL, Token);
	
	CloneChildren(semanter, clone);

	return clone;
}
