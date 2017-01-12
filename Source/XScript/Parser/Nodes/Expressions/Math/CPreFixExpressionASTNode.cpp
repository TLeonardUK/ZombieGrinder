/* *****************************************************************

		CPreFixExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Math/CPreFixExpressionASTNode.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIndexExpressionASTNode.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"

#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/Helper/CNumericDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CPreFixExpressionASTNode::CPreFixExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CPreFixExpressionASTNode::Clone(CSemanter* semanter)
{
	CPreFixExpressionASTNode* clone = new CPreFixExpressionASTNode(NULL, Token);

	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}

	return clone;
}

// =================================================================
//	Attempts to perform an operator override if one is declared
//  for the lvalue.
// =================================================================
CASTNode* CPreFixExpressionASTNode::PerformOperatorOverride(CSemanter* semanter, CExpressionBaseASTNode* lvalue)
{
	CClassASTNode* lvalue_class = lvalue->ExpressionResultType->GetClass(semanter);

	switch (Token.Type)
	{		
		case TokenIdentifier::OP_ADD:
		case TokenIdentifier::OP_SUB:
			{
				String operator_method_name = "__operator" + Token.Literal;

				std::vector<CDataType*> arguments;

				CClassMemberASTNode* overload = lvalue_class->FindClassMethod(semanter, operator_method_name, arguments, false);
				if (overload != NULL)
				{
					CToken ident_token = Token;
					ident_token.Literal = operator_method_name;

					CIdentifierExpressionASTNode* method_call_ident = new CIdentifierExpressionASTNode(NULL, ident_token);						

					CMethodCallExpressionASTNode* method_call = new CMethodCallExpressionASTNode(Parent, Token);
					method_call->RightValue = method_call_ident;
					method_call->LeftValue = lvalue;
					method_call->AddChild(method_call_ident);
					
					RemoveChild(lvalue);
					method_call->AddChild(lvalue);	
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
CASTNode* CPreFixExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CPreFixExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Semant expressions.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));
	
	// Get expression references.
	CExpressionBaseASTNode* leftValueBase  = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	
	// First check for operator overrides before doing a normal op.
	CASTNode* override_node = PerformOperatorOverride(semanter, leftValueBase);
	if (override_node != NULL)
	{
		return override_node;
	}

	// Balance types.
	switch (Token.Type)
	{	
		case TokenIdentifier::OP_INCREMENT:
		case TokenIdentifier::OP_DECREMENT:
		{
			// Try and find field the l-value is refering to.
			CClassMemberASTNode*		   field_node		 = NULL;
			CVariableStatementASTNode*	   var_node			 = NULL;
			CFieldAccessExpressionASTNode* field_access_node = dynamic_cast<CFieldAccessExpressionASTNode*>(LeftValue);
			CIdentifierExpressionASTNode*  identifier_node   = dynamic_cast<CIdentifierExpressionASTNode*>(LeftValue);
			CIndexExpressionASTNode*	   index_node		 = dynamic_cast<CIndexExpressionASTNode*>(LeftValue);

			if (index_node != NULL)
			{
				// Should call CanAssignIndex or something
				CExpressionBaseASTNode* leftLeftValueBase  = dynamic_cast<CExpressionBaseASTNode*>(index_node->LeftValue);
				CExpressionBaseASTNode* indexIndexValueBase = dynamic_cast<CExpressionBaseASTNode*>(index_node->IndexExpression);

				std::vector<CDataType*> args;
				args.push_back(indexIndexValueBase->ExpressionResultType);
				args.push_back(leftValueBase->ExpressionResultType);

				CDeclarationIdentifier s_operator_ident("__operator[]");

				CClassASTNode* arrayClass = leftLeftValueBase->ExpressionResultType->GetClass(semanter);
				CClassMemberASTNode* memberNode = arrayClass->FindClassMethod(semanter, s_operator_ident, args, false, NULL, NULL);
		
				if (memberNode == NULL || (dynamic_cast<CStringDataType*>(leftLeftValueBase->ExpressionResultType) == NULL && dynamic_cast<CArrayDataType*>(leftLeftValueBase->ExpressionResultType) == NULL))
				{
					index_node = NULL;
				}
			}
			else if (field_access_node != NULL)
			{
				field_node = field_access_node->ExpressionResultClassMember;
			}
			else if (identifier_node != NULL)
			{
				field_node = identifier_node->ExpressionResultClassMember;
				var_node = identifier_node->ExpressionResultVariable;
			}

			// Is the l-value a valid assignment target?
			if (field_node == NULL && var_node == NULL && index_node == NULL)
			{		
				semanter->GetContext()->FatalError("Illegal l-value for assignment expression.", Token);
			}
			if (field_node != NULL && field_node->IsConst == true)
			{		
				semanter->GetContext()->FatalError("Illegal l-value for assignment expression, l-value was declared constant.", Token);
			}

			// Work out result!
			ExpressionResultType = leftValueBase->ExpressionResultType;

			// Check expression type is integer.
			if (dynamic_cast<CIntDataType*>(ExpressionResultType) == NULL)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Prefix operator '%s' only supports integer r-value's.", Token.Literal.c_str()), Token);
			}

			break;
		}

		case TokenIdentifier::OP_ADD:
		case TokenIdentifier::OP_SUB:
		{
			if (dynamic_cast<CNumericDataType*>(leftValueBase->ExpressionResultType) == NULL)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Unary %s operator is only supported on numeric types.", Token.Literal.c_str()), Token);
			}

			ExpressionResultType = leftValueBase->ExpressionResultType;
			LeftValue = ReplaceChild(LeftValue,  leftValueBase->CastTo(semanter, ExpressionResultType, Token, false));
			break;
		}

		case TokenIdentifier::OP_NOT:
		{
			ExpressionResultType = new CIntDataType(Token);
			LeftValue = ReplaceChild(LeftValue,  leftValueBase->CastTo(semanter, ExpressionResultType, Token, false));
			break;
		}

		case TokenIdentifier::OP_LOGICAL_NOT:
		{
			ExpressionResultType = new CBoolDataType(Token);
			LeftValue  = ReplaceChild(LeftValue,  leftValueBase->CastTo(semanter, ExpressionResultType, Token, true));
		
			break;
		}

		default:
		{
			semanter->GetContext()->FatalError("Internal error. Invalid prefix math operator.", Token);
			break;
		}
	}

	return this;
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CPreFixExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	EvaluationResult leftResult  = LeftValue->Evaluate(unit);

	switch (Token.Type)
	{	
		case TokenIdentifier::OP_INCREMENT:
		case TokenIdentifier::OP_DECREMENT:	
		{
			break;
		}
		case TokenIdentifier::OP_ADD:	
		{
			if (dynamic_cast<CIntDataType*>(ExpressionResultType) != NULL)
			{
				return EvaluationResult(+leftResult.GetInt());
			}
			else if (dynamic_cast<CFloatDataType*>(ExpressionResultType) != NULL)
			{
				return EvaluationResult(+leftResult.GetFloat());
			}
			break;
		}
		case TokenIdentifier::OP_SUB:
		{
			if (dynamic_cast<CIntDataType*>(ExpressionResultType) != NULL)
			{
				return EvaluationResult(-leftResult.GetInt());
			}
			else if (dynamic_cast<CFloatDataType*>(ExpressionResultType) != NULL)
			{
				return EvaluationResult(-leftResult.GetFloat());
			}
			break;
		}
		case TokenIdentifier::OP_NOT:
		{
			if (dynamic_cast<CIntDataType*>(ExpressionResultType) != NULL)
			{
				return EvaluationResult(~leftResult.GetInt());
			}
			break;
		}
		case TokenIdentifier::OP_LOGICAL_NOT:
		{
			if (dynamic_cast<CBoolDataType*>(ExpressionResultType) != NULL)
			{
				return EvaluationResult(!leftResult.GetBool());
			}
			break;
		}
	}
	
	unit->FatalError("Invalid postfix operation, increment and decrement operators cannot be evaluated.", Token);
	return EvaluationResult(false);
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CPreFixExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslatePreFixExpression(this);
}