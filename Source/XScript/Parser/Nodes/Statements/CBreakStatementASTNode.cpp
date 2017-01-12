/* *****************************************************************

		CBreakStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CBreakStatementASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CBreakStatementASTNode::CBreakStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CBreakStatementASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CBreakStatementASTNode");

	CASTNode* node = FindLoopScope(semanter);
	if (node == NULL || node->AcceptBreakStatement() == false)
	{
		semanter->GetContext()->FatalError("Break statements can only be used inside loops and switch statements.", Token);
	}
	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CBreakStatementASTNode::Clone(CSemanter* semanter)
{
	CBreakStatementASTNode* clone = new CBreakStatementASTNode(NULL, Token);

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CBreakStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateBreakStatement(this);
}