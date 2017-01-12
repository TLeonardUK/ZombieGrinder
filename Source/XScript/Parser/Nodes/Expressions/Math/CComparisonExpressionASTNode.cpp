/* *****************************************************************

		CComparisonExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Math/CComparisonExpressionASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CComparisonExpressionASTNode::CComparisonExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL),
	CompareResultType(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CComparisonExpressionASTNode::Clone(CSemanter* semanter)
{
	CComparisonExpressionASTNode* clone = new CComparisonExpressionASTNode(NULL, Token);

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

	return clone;
}

// =================================================================
//	Attempts to perform an operator override if one is declared
//  for the lvalue.
// =================================================================
CASTNode* CComparisonExpressionASTNode::PerformOperatorOverride(CSemanter* semanter, CExpressionBaseASTNode* lvalue, CExpressionBaseASTNode* rvalue)
{
	CClassASTNode* lvalue_class = lvalue->ExpressionResultType->GetClass(semanter);

	switch (Token.Type)
	{		
	case TokenIdentifier::OP_EQUAL:
	case TokenIdentifier::OP_NOT_EQUAL:
	case TokenIdentifier::OP_GREATER:
	case TokenIdentifier::OP_GREATER_EQUAL:
	case TokenIdentifier::OP_LESS:
	case TokenIdentifier::OP_LESS_EQUAL:
			{
				String operator_method_name = "__operator" + Token.Literal;

				std::vector<CDataType*> arguments;
				arguments.push_back(rvalue->ExpressionResultType);

				CClassMemberASTNode* overload = lvalue_class->FindClassMethod(semanter, operator_method_name, arguments, false);
				if (overload != NULL)
				{
					if (dynamic_cast<CBoolDataType*>(overload->ReturnType) != NULL)
					{
						CToken ident_token = Token;
						ident_token.Literal = operator_method_name;

						CIdentifierExpressionASTNode* method_call_ident = new CIdentifierExpressionASTNode(NULL, ident_token);						
						
						CMethodCallExpressionASTNode* method_call = new CMethodCallExpressionASTNode(Parent, Token);
						method_call->RightValue = method_call_ident;
						method_call->LeftValue = lvalue;
						method_call->ArgumentExpressions.push_back(rvalue);
						method_call->AddChild(method_call_ident);
					
						RemoveChild(lvalue);
						method_call->AddChild(lvalue);	
						RemoveChild(rvalue);
						method_call->AddChild(rvalue);	
						method_call->AddChild(method_call_ident);

						ExpressionResultType = overload->ReturnType;

						return method_call->Semant(semanter);
					}
					else
					{
						semanter->GetContext()->FatalError(StringHelper::Format("Operator overload for '%s' must return bool.", Token.Literal.c_str()), Token);
					}
				}

				break;
			}
	}

	return NULL;
}
// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CComparisonExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CComparisonExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Semant expressions.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));
	RightValue = ReplaceChild(RightValue, RightValue->Semant(semanter)); 

	// Get expression references.
	CExpressionBaseASTNode* leftValueBase  = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* rightValueBase = dynamic_cast<CExpressionBaseASTNode*>(RightValue);

	// First check for operator overrides before doing a normal op.
	CASTNode* override_node = PerformOperatorOverride(semanter, leftValueBase, rightValueBase);
	if (override_node != NULL)
	{
		return override_node;
	}

	// Balance types.
	ExpressionResultType = semanter->BalanceDataTypes(leftValueBase->ExpressionResultType, 
		rightValueBase->ExpressionResultType,
		Token);
	
	// Objects only permit equality operations.
	if (dynamic_cast<CObjectDataType*>(ExpressionResultType) != NULL &&
		Token.Type != TokenIdentifier::OP_EQUAL &&
		Token.Type != TokenIdentifier::OP_NOT_EQUAL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("%s operator cannot be used on objects.", Token.Literal.c_str()), Token);
	}

	// Cast to resulting expression.
	LeftValue  = ReplaceChild(LeftValue,  leftValueBase->CastTo(semanter, ExpressionResultType, Token));
	RightValue = ReplaceChild(RightValue, rightValueBase->CastTo(semanter, ExpressionResultType, Token)); 

	CompareResultType = ExpressionResultType;
	ExpressionResultType = new CBoolDataType(Token);

	return this;
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CComparisonExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	EvaluationResult leftResult  = LeftValue->Evaluate(unit);
	EvaluationResult rightResult = RightValue->Evaluate(unit);

	if (dynamic_cast<CBoolDataType*>(CompareResultType) != NULL)
	{
	}
	else if (dynamic_cast<CIntDataType*>(CompareResultType) != NULL)
	{
		switch (Token.Type)
		{		
			case TokenIdentifier::OP_EQUAL:			return EvaluationResult(leftResult.GetInt() == rightResult.GetInt()); 
			case TokenIdentifier::OP_NOT_EQUAL:		return EvaluationResult(leftResult.GetInt() != rightResult.GetInt());  
			case TokenIdentifier::OP_GREATER:		return EvaluationResult(leftResult.GetInt() >  rightResult.GetInt());
			case TokenIdentifier::OP_LESS:			return EvaluationResult(leftResult.GetInt() <  rightResult.GetInt());   
			case TokenIdentifier::OP_GREATER_EQUAL:	return EvaluationResult(leftResult.GetInt() >= rightResult.GetInt()); 
			case TokenIdentifier::OP_LESS_EQUAL:	return EvaluationResult(leftResult.GetInt() <= rightResult.GetInt());  
		}
	}
	else if (dynamic_cast<CFloatDataType*>(CompareResultType) != NULL)
	{
		switch (Token.Type)
		{		
			case TokenIdentifier::OP_EQUAL:			return EvaluationResult(leftResult.GetFloat() == rightResult.GetFloat()); 
			case TokenIdentifier::OP_NOT_EQUAL:		return EvaluationResult(leftResult.GetFloat() != rightResult.GetFloat());  
			case TokenIdentifier::OP_GREATER:		return EvaluationResult(leftResult.GetFloat() >  rightResult.GetFloat());
			case TokenIdentifier::OP_LESS:			return EvaluationResult(leftResult.GetFloat() <  rightResult.GetFloat());   
			case TokenIdentifier::OP_GREATER_EQUAL:	return EvaluationResult(leftResult.GetFloat() >= rightResult.GetFloat()); 
			case TokenIdentifier::OP_LESS_EQUAL:	return EvaluationResult(leftResult.GetFloat() <= rightResult.GetFloat());  
		}
	}
	else if (dynamic_cast<CStringDataType*>(CompareResultType) != NULL)
	{
		switch (Token.Type)
		{		
			case TokenIdentifier::OP_EQUAL:			return EvaluationResult(leftResult.GetString() == rightResult.GetString()); 
			case TokenIdentifier::OP_NOT_EQUAL:		return EvaluationResult(leftResult.GetString() != rightResult.GetString());  
			case TokenIdentifier::OP_GREATER:		return EvaluationResult(leftResult.GetString() >  rightResult.GetString());
			case TokenIdentifier::OP_LESS:			return EvaluationResult(leftResult.GetString() <  rightResult.GetString());   
			case TokenIdentifier::OP_GREATER_EQUAL:	return EvaluationResult(leftResult.GetString() >= rightResult.GetString()); 
			case TokenIdentifier::OP_LESS_EQUAL:	return EvaluationResult(leftResult.GetString() <= rightResult.GetString());  
		}
	}
	
	unit->FatalError("Invalid constant operation.", Token);
	return EvaluationResult(false);
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CComparisonExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateComparisonExpression(this);
}