/* *****************************************************************

		CIndexExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Types/CIndexExpressionASTNode.h"

#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Translator/CTranslator.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CIndexExpressionASTNode::CIndexExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	IndexExpression(NULL),
	LeftValue(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CIndexExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CIndexExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;
	
	// Semant expressions.
	LeftValue		= ReplaceChild(LeftValue, LeftValue->Semant(semanter));
	IndexExpression = ReplaceChild(IndexExpression, IndexExpression->Semant(semanter));

	// Get expression references.
	CExpressionBaseASTNode* lValueBase    = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* indexExprBase = dynamic_cast<CExpressionBaseASTNode*>(IndexExpression);

	// Cast index to integer.
//	IndexExpression = ReplaceChild(indexExprBase, indexExprBase->CastTo(semanter, new CIntDataType(Token), Token));

	// Valid object to index?
	std::vector<CDataType*> argumentTypes;
	argumentTypes.push_back(indexExprBase->ExpressionResultType);//new CIntDataType(Token));

	CDeclarationIdentifier s_operator_ident("__operator[]");

	CClassASTNode* classNode = lValueBase->ExpressionResultType->GetClass(semanter);
	CClassMemberASTNode* memberNode = classNode->FindClassMethod(semanter, s_operator_ident, argumentTypes, false, NULL, NULL);

	if (memberNode == NULL)
	{
		semanter->GetContext()->FatalError(StringHelper::Format("Data type does not support indexing, no operator[] method defined for key type '%s'.", indexExprBase->ExpressionResultType->ToString().c_str()).c_str(), Token);
	}
	// TODO: Remove this restriction.
//	else if (memberNode->MangledIdentifier != "GetIndex")
//	{
//		semanter->GetContext()->FatalError("Indexing using the GetIndex method is only supported on native members.", Token);
//	}
	else
	{
		ExpressionResultType = memberNode->ReturnType;
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CIndexExpressionASTNode::Clone(CSemanter* semanter)
{
	CIndexExpressionASTNode* clone = new CIndexExpressionASTNode(NULL, Token);
	
	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}

	if (IndexExpression != NULL)
	{
		clone->IndexExpression = dynamic_cast<CASTNode*>(IndexExpression->Clone(semanter));
		clone->AddChild(clone->IndexExpression);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CIndexExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateIndexExpression(this);
}