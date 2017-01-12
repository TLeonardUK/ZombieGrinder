/* *****************************************************************

		CIndexOfExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CIndexOfExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CIndexOfExpressionASTNode::CIndexOfExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token)
	, Expression(NULL)
	, ClassMember(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CIndexOfExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CIndexOfExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Semant expressions.
	Expression = (CExpressionASTNode*)ReplaceChild(Expression, Expression->Semant(semanter));

	// Get expression references.
	CFieldAccessExpressionASTNode* field_access_node = dynamic_cast<CFieldAccessExpressionASTNode*>(Expression);
	if (field_access_node == NULL)
	{
		semanter->GetContext()->FatalError("IndexOf expression must be fully qualified field name.", Token);
	}

	ClassMember = field_access_node->ExpressionResultClassMember;

	// Result type is type of "Type"
	ExpressionResultType = CIntDataType::StaticInstance;

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CIndexOfExpressionASTNode::Clone(CSemanter* semanter)
{
	CIndexOfExpressionASTNode* clone = new CIndexOfExpressionASTNode(NULL, Token);
	
	CASTNode* node = Expression->Clone(semanter);
	clone->AddChild(node);

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CIndexOfExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateIndexOfExpression(this);
}