/* *****************************************************************

		CBlockStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CBlockStatementASTNode.h"

#include "XScript/Translator/CTranslator.h"

#include "XScript/Semanter/CSemanter.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CBlockStatementASTNode::CBlockStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CBlockStatementASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CBlockStatementASTNode");

	SemantChildren(semanter);
	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CBlockStatementASTNode::Clone(CSemanter* semanter)
{
	CBlockStatementASTNode* clone = new CBlockStatementASTNode(NULL, Token);

	CloneChildren(semanter, clone);

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CBlockStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateBlockStatement(this);
}