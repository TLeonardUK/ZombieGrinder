/* *****************************************************************

		CSliceExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Types/CSliceExpressionASTNode.h"

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
CSliceExpressionASTNode::CSliceExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token), 
	StartExpression(NULL), 
	EndExpression(NULL),
	LeftValue(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CSliceExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CSliceExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;
	
	// Semant expressions.
	LeftValue = ReplaceChild(LeftValue, LeftValue->Semant(semanter));
	if (StartExpression != NULL)
	{
		StartExpression  = ReplaceChild(StartExpression, StartExpression->Semant(semanter));
	}
	if (EndExpression != NULL)
	{
		EndExpression    = ReplaceChild(EndExpression, EndExpression->Semant(semanter));
	}

	// Get expression references.
	CExpressionBaseASTNode* lValueBase    = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* startExprBase = dynamic_cast<CExpressionBaseASTNode*>(StartExpression);
	CExpressionBaseASTNode* endExprBase   = dynamic_cast<CExpressionBaseASTNode*>(EndExpression);
	
	// Cast index to integer.
	if (startExprBase != NULL)
	{
		StartExpression = ReplaceChild(startExprBase, startExprBase->CastTo(semanter, new CIntDataType(Token), Token));
	}
	if (endExprBase != NULL)
	{
		EndExpression   = ReplaceChild(endExprBase,   endExprBase->CastTo  (semanter, new CIntDataType(Token), Token));
	}

	// Valid object to slice?
	std::vector<CDataType*> argumentTypes;
	argumentTypes.push_back(new CIntDataType(Token));
	argumentTypes.push_back(new CIntDataType(Token));

	CDeclarationIdentifier s_getslice_ident("GetSlice");

	CClassASTNode* classNode = lValueBase->ExpressionResultType->GetClass(semanter);
	CClassMemberASTNode* memberNode = classNode->FindClassMethod(semanter, s_getslice_ident, argumentTypes, true, NULL, NULL);

	if (memberNode == NULL)
	{
		semanter->GetContext()->FatalError("Data type does not support slicing, no GetSlice method defined.", Token);
	}
	else
	{
		ExpressionResultType = memberNode->ReturnType;
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CSliceExpressionASTNode::Clone(CSemanter* semanter)
{
	CSliceExpressionASTNode* clone = new CSliceExpressionASTNode(NULL, Token);
	
	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}

	if (StartExpression != NULL)
	{
		clone->StartExpression = dynamic_cast<CASTNode*>(StartExpression->Clone(semanter));
		clone->AddChild(clone->StartExpression);
	}

	if (EndExpression != NULL)
	{
		clone->EndExpression = dynamic_cast<CASTNode*>(EndExpression->Clone(semanter));
		clone->AddChild(clone->EndExpression);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CSliceExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateSliceExpression(this);
}