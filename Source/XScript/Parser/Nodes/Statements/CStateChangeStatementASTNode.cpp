/* *****************************************************************

		CStateChangeStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CStateChangeStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CStateChangeStatementASTNode::CStateChangeStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
	, StateIdentifier("")
	, State(NULL)
{
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CStateChangeStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CStateChangeStatementASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Check state exists.
	if (Token.Type != TokenIdentifier::KEYWORD_POP_STATE)
	{
		State = FindClassState(semanter, StateIdentifier, NULL, NULL);
		if (State == NULL)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Unidentified state name '%s'.", StateIdentifier.c_str()), Token);
		}
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CStateChangeStatementASTNode::Clone(CSemanter* semanter)
{
	CStateChangeStatementASTNode* clone = new CStateChangeStatementASTNode(NULL, Token);
	clone->StateIdentifier = clone->StateIdentifier;

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CStateChangeStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateStateChangeStatement(this);
}