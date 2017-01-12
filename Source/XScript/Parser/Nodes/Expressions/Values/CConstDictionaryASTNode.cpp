/* *****************************************************************

		CConstDictionaryASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Values/CConstDictionaryASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CConstDictionaryASTNode::CConstDictionaryASTNode(CASTNode* parent, CToken token) 
	: CExpressionBaseASTNode(parent, token)
{
}

// =================================================================
//	Destructor.
// =================================================================
CConstDictionaryASTNode::~CConstDictionaryASTNode()
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CConstDictionaryASTNode::Clone(CSemanter* semanter)
{
	CConstDictionaryASTNode* clone = new CConstDictionaryASTNode(NULL, Token);
	
	for (std::vector<CConstDictionaryValue>::iterator iter = Values.begin(); iter != Values.end(); iter++)
	{
		CConstDictionaryValue& var = *iter;
		CConstDictionaryValue clone_var;
		clone_var.Identifier		 = var.Identifier;
		clone_var.ValueExpr			 = dynamic_cast<CExpressionBaseASTNode*>(var.ValueExpr->Clone(semanter));

		clone->AddChild(clone_var.ValueExpr);

		clone->Values.push_back(var);
	}

	return clone;
}

// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CConstDictionaryASTNode::ToString()
{
	return "Const Dictionary";
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CConstDictionaryASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CConstDictionaryASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		//m_semanting = false;
		return this;
	}
	Semanted = true;

	// Semant all values.
	SemantChildren(semanter);

	return this;
}

// =================================================================
//	Performs finalization on this class.
// =================================================================
CASTNode* CConstDictionaryASTNode::Finalize(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CConstDictionaryASTNode::Translate(CTranslator* translator)
{
	// No translation is performed for this, as it just holds values for something else.
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CConstDictionaryASTNode::TranslateExpr(CTranslator* translator)
{
	DBG_ASSERT(false); // Should never be called!
	return 0;
}