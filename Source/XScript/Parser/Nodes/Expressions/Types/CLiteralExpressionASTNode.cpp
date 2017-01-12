/* *****************************************************************

		CLiteralExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Types/CLiteralExpressionASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/Helper/CNullDataType.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CLiteralExpressionASTNode::CLiteralExpressionASTNode(CASTNode* parent, CToken token, CDataType* type, String lit) :
	CExpressionBaseASTNode(parent, token), 
	Type(type), 
	Literal(lit)
{

}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CLiteralExpressionASTNode::Clone(CSemanter* semanter)
{
	CLiteralExpressionASTNode* clone = new CLiteralExpressionASTNode(NULL, Token, Type, Literal);

	return clone;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CLiteralExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CLiteralExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	//DBG_LOG("LIT:0x%08x", Type);
	ExpressionResultType = Type->Semant(semanter, this);
	//DBG_LOG("LIT RT:0x%08x", ExpressionResultType);

	return this;
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CLiteralExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	if (dynamic_cast<CBoolDataType*>(ExpressionResultType) != NULL)
	{
		return EvaluationResult(Literal == "0" || CStringHelper::ToLower(Literal) == "false" || Literal == "" ? false : true);
	}
	else if (dynamic_cast<CIntDataType*>(ExpressionResultType) != NULL)
	{
		return EvaluationResult(CStringHelper::ToInt(Literal));
	}
	else if (dynamic_cast<CFloatDataType*>(ExpressionResultType) != NULL)
	{
		return EvaluationResult(CStringHelper::ToFloat(Literal));
	}
	else if (dynamic_cast<CStringDataType*>(ExpressionResultType) != NULL)
	{
		return EvaluationResult(Literal);
	}
	else if (dynamic_cast<CNullDataType*>(ExpressionResultType) != NULL)
	{
		return EvaluationResult(0);
	}
	
	unit->FatalError("Invalid constant operation.", Token);
	return EvaluationResult(false);
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CLiteralExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateLiteralExpression(this);
}