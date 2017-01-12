/* *****************************************************************

		CPackageASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/TopLevel/CPackageASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CPackageASTNode::CPackageASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CPackageASTNode::Semant(CSemanter* semanter)
{
	SEMANT_TRACE("CPackageASTNode");

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
CASTNode* CPackageASTNode::Clone(CSemanter* semanter)
{
	CPackageASTNode* clone = new CPackageASTNode(NULL, Token);

	CloneChildren(semanter, clone);

	return clone;
}
