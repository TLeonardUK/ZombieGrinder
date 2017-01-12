/* *****************************************************************

		CBinaryMathExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Math/CBinaryMathExpressionASTNode.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CBinaryMathExpressionASTNode::CBinaryMathExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CBinaryMathExpressionASTNode::Clone(CSemanter* semanter)
{
	CBinaryMathExpressionASTNode* clone = new CBinaryMathExpressionASTNode(NULL, Token);

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
CASTNode* CBinaryMathExpressionASTNode::PerformOperatorOverride(CSemanter* semanter, CExpressionBaseASTNode* lvalue, CExpressionBaseASTNode* rvalue)
{
	CClassASTNode* lvalue_class = lvalue->ExpressionResultType->GetClass(semanter);

	switch (Token.Type)
	{		
		case TokenIdentifier::OP_ADD:
		case TokenIdentifier::OP_SUB:
		case TokenIdentifier::OP_MUL:
		case TokenIdentifier::OP_DIV:
			{
				String operator_method_name = "__operator" + Token.Literal;

				std::vector<CDataType*> arguments;
				arguments.push_back(rvalue->ExpressionResultType);

				CClassMemberASTNode* overload = lvalue_class->FindClassMethod(semanter, operator_method_name, arguments, false);
				if (overload != NULL)
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

				break;
			}
	}

	return NULL;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CBinaryMathExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CBinaryMathExpressionASTNode");

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
	switch (Token.Type)
	{
		// Integer only operators.		
		case TokenIdentifier::OP_AND:
		case TokenIdentifier::OP_OR:
		case TokenIdentifier::OP_XOR:
		case TokenIdentifier::OP_SHL:
		case TokenIdentifier::OP_SHR:
		case TokenIdentifier::OP_MOD:
		{
			ExpressionResultType = new CIntDataType(Token);
			break;
		}

		// Applicable to any type operators.
		case TokenIdentifier::OP_ADD:
		case TokenIdentifier::OP_SUB:
		case TokenIdentifier::OP_MUL:
		case TokenIdentifier::OP_DIV:
			{
				ExpressionResultType = semanter->BalanceDataTypes(leftValueBase->ExpressionResultType, 
																  rightValueBase->ExpressionResultType,
																  Token);
				
				if (dynamic_cast<CStringDataType*>(ExpressionResultType) != NULL)
				{
					if (Token.Type != TokenIdentifier::OP_ADD)
					{
						semanter->GetContext()->FatalError("Invalid operator, strings only supports concatination.", Token);			
					}
				}
				else if (dynamic_cast<CNumericDataType*>(ExpressionResultType) == NULL)
				{
					semanter->GetContext()->FatalError(CStringHelper::FormatString("Invalid expression. Operator '%s' cannot be used on types '%s' and '%s'.", Token.Literal.c_str(), leftValueBase->ExpressionResultType->ToString().c_str(), rightValueBase->ExpressionResultType->ToString().c_str()), Token);			
				}

				break;
			}
		default:
		{
			semanter->GetContext()->FatalError("Internal error. Invalid binary math operator.", Token);
			break;
		}
	}

	// Cast to resulting expression.
	LeftValue  = ReplaceChild(LeftValue,  leftValueBase->CastTo(semanter, ExpressionResultType, Token));
	RightValue = ReplaceChild(RightValue, rightValueBase->CastTo(semanter, ExpressionResultType, Token)); 

	return this;
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CBinaryMathExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	EvaluationResult leftResult  = LeftValue->Evaluate(unit);
	EvaluationResult rightResult = RightValue->Evaluate(unit);

	if (dynamic_cast<CBoolDataType*>(ExpressionResultType) != NULL)
	{
	}
	else if (dynamic_cast<CIntDataType*>(ExpressionResultType) != NULL)
	{
		switch (Token.Type)
		{		
			case TokenIdentifier::OP_AND:	return EvaluationResult(leftResult.GetInt() & rightResult.GetInt()); 
			case TokenIdentifier::OP_OR:	return EvaluationResult(leftResult.GetInt() | rightResult.GetInt());  
			case TokenIdentifier::OP_XOR:	return EvaluationResult(leftResult.GetInt() ^ rightResult.GetInt());
			case TokenIdentifier::OP_SHL:	return EvaluationResult(leftResult.GetInt() << rightResult.GetInt());  
			case TokenIdentifier::OP_SHR:	return EvaluationResult(leftResult.GetInt() >> rightResult.GetInt());  
			case TokenIdentifier::OP_MOD:	return EvaluationResult(leftResult.GetInt() % rightResult.GetInt());  
			case TokenIdentifier::OP_ADD:	return EvaluationResult(leftResult.GetInt() + rightResult.GetInt());
			case TokenIdentifier::OP_SUB:	return EvaluationResult(leftResult.GetInt() - rightResult.GetInt());  
			case TokenIdentifier::OP_MUL:	return EvaluationResult(leftResult.GetInt() * rightResult.GetInt()); 
			case TokenIdentifier::OP_DIV:	
				{
					if (rightResult.GetInt() == 0)
					{
						unit->FatalError("Attempt to divide by zero.", Token);
					}
					return EvaluationResult(leftResult.GetInt() / rightResult.GetInt()); 
				}
		}
	}
	else if (dynamic_cast<CFloatDataType*>(ExpressionResultType) != NULL)
	{
		switch (Token.Type)
		{		 
			case TokenIdentifier::OP_ADD:	return EvaluationResult(leftResult.GetFloat() + rightResult.GetFloat());
			case TokenIdentifier::OP_SUB:	return EvaluationResult(leftResult.GetFloat() - rightResult.GetFloat());  
			case TokenIdentifier::OP_MUL:	return EvaluationResult(leftResult.GetFloat() * rightResult.GetFloat()); 
			case TokenIdentifier::OP_DIV:	
				{
					if (rightResult.GetFloat() == 0)
					{
						unit->FatalError("Attempt to divide by zero.", Token);
					}
					return EvaluationResult(leftResult.GetFloat() / rightResult.GetFloat()); 
				}
		}
	}
	else if (dynamic_cast<CStringDataType*>(ExpressionResultType) != NULL)
	{
		switch (Token.Type)
		{		 
			case TokenIdentifier::OP_ADD:	return EvaluationResult(leftResult.GetString() + rightResult.GetString());
		}
	}
	
	unit->FatalError("Invalid constant operation.", Token);
	return EvaluationResult(false);
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CBinaryMathExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateBinaryMathExpression(this);
}