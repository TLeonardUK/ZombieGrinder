/* *****************************************************************

		CTernaryExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Branching/CTernaryExpressionASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CTernaryExpressionASTNode::CTernaryExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	Expression(NULL),
	LeftValue(NULL),
	RightValue(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CTernaryExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CTernaryExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Get expression representations.
	CExpressionBaseASTNode* left_hand_expr	 = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* right_hand_expr  = dynamic_cast<CExpressionBaseASTNode*>(RightValue);
	CExpressionBaseASTNode* expr_expr		 = dynamic_cast<CExpressionBaseASTNode*>(Expression);

	// Semant left hand node.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));

	// Semant right hand node.
	RightValue = ReplaceChild(RightValue, RightValue->Semant(semanter)); 

	// Semant expression node.
	Expression = ReplaceChild(Expression, Expression->Semant(semanter)); 

	// Make sure both l and r value are same DT.
	if (!left_hand_expr->ExpressionResultType->IsEqualTo(semanter, right_hand_expr->ExpressionResultType))
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Both expressions of a ternary operator must result in the same data type."), Token);			
	}

	// Cast expression to bool.
	ExpressionResultType = new CBoolDataType(Token);
	Expression			 = ReplaceChild(Expression,  expr_expr->CastTo(semanter, ExpressionResultType, Token, true));

	// Resulting type is our left hand type.
	ExpressionResultType = left_hand_expr->ExpressionResultType;

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CTernaryExpressionASTNode::Clone(CSemanter* semanter)
{
	CTernaryExpressionASTNode* clone = new CTernaryExpressionASTNode(NULL, Token);

	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}

	if (RightValue != NULL)
	{
		clone->RightValue = dynamic_cast<CASTNode*>(RightValue->Clone(semanter));
		clone->AddChild(clone->RightValue);
	}

	if (Expression != NULL)
	{
		clone->Expression = dynamic_cast<CASTNode*>(Expression->Clone(semanter));
		clone->AddChild(clone->Expression);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CTernaryExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateTernaryExpression(this);
}