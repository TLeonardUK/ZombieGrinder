/* *****************************************************************

		CContinueStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CContinueStatementASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CContinueStatementASTNode::CContinueStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CContinueStatementASTNode::Semant(CSemanter* semanter)
{ 
	CASTNode* node = FindLoopScope(semanter);
	if (node == NULL || node->AcceptContinueStatement() == false)
	{
		semanter->GetContext()->FatalError("Continue statements can only be used inside loops.", Token);
	}
	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CContinueStatementASTNode::Clone(CSemanter* semanter)
{
	CContinueStatementASTNode* clone = new CContinueStatementASTNode(NULL, Token);

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CContinueStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateContinueStatement(this);
}