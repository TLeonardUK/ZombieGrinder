/* *****************************************************************

		CAssignmentExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Assignment/CAssignmentExpressionASTNode.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIndexExpressionASTNode.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CAssignmentExpressionASTNode::CAssignmentExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL),
	IgnoreConst(false)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CAssignmentExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CAssignmentExpressionASTNode");

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
		args.push_back(rightValueBase->ExpressionResultType);

		CDeclarationIdentifier s_operator_ident("__operator[]");

		CClassASTNode* arrayClass = leftLeftValueBase->ExpressionResultType->GetClass(semanter);
		CClassMemberASTNode* memberNode = arrayClass->FindClassMethod(semanter, s_operator_ident, args, false, NULL, NULL);
		
		if (memberNode == NULL)// || (dynamic_cast<CStringDataType*>(leftLeftValueBase->ExpressionResultType) == NULL && dynamic_cast<CArrayDataType*>(leftLeftValueBase->ExpressionResultType) == NULL))
		{
			index_node = NULL;
		}
	}
	else if (field_access_node != NULL)
	{
		field_node = field_access_node->ExpressionResultClassMember;
		if (field_access_node->IsPropertyAccess == true)
		{
			if (field_access_node->Property_SetMethod == NULL)
			{
				semanter->GetContext()->FatalError("Property is read only and cannot be assigned.", Token);
			}
		}
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
	if (field_node != NULL && field_node->IsConst == true && IgnoreConst == false)
	{		
		semanter->GetContext()->FatalError("Illegal l-value for assignment expression, l-value was declared constant.", Token);
	}

	// Balance types.
	ExpressionResultType = leftValueBase->ExpressionResultType;
				
	switch (Token.Type)
	{
		// Anything goes :3
		case TokenIdentifier::OP_ASSIGN:
		{
			break;
		}

		// Integer only operators.		
		case TokenIdentifier::OP_ASSIGN_AND:
		case TokenIdentifier::OP_ASSIGN_OR:
		case TokenIdentifier::OP_ASSIGN_XOR:
		case TokenIdentifier::OP_ASSIGN_SHL:
		case TokenIdentifier::OP_ASSIGN_SHR:
		case TokenIdentifier::OP_ASSIGN_MOD:
		{
			if (dynamic_cast<CIntDataType*>(leftValueBase->ExpressionResultType) == NULL)
			{	
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Assignment operator '%s' cannot be used on types '%s' and '%s'.", Token.Literal.c_str(), leftValueBase->ExpressionResultType->ToString().c_str(), rightValueBase->ExpressionResultType->ToString().c_str()), Token);							
			}
			break;
		}

		// Applicable to any type operators.
		case TokenIdentifier::OP_ASSIGN_ADD:
		case TokenIdentifier::OP_ASSIGN_SUB:
		case TokenIdentifier::OP_ASSIGN_MUL:
		case TokenIdentifier::OP_ASSIGN_DIV:
			{
				if (dynamic_cast<CStringDataType*>(ExpressionResultType) != NULL)
				{
					if (Token.Type != TokenIdentifier::OP_ASSIGN_ADD)
					{
						semanter->GetContext()->FatalError("Invalid operator, strings only supports concatination.", Token);			
					}
				}
				else if (dynamic_cast<CNumericDataType*>(ExpressionResultType) == NULL)
				{
					semanter->GetContext()->FatalError(CStringHelper::FormatString("Assignment operator '%s' cannot be used on types '%s' and '%s'.", Token.Literal.c_str(), leftValueBase->ExpressionResultType->ToString().c_str(), rightValueBase->ExpressionResultType->ToString().c_str()), Token);			
				}

				break;
			}

		// dafuq
		default:
		{
			semanter->GetContext()->FatalError("Internal error. Invalid assignment operator.", Token);
			break;
		}
	}

	// Cast R-Value to correct type.
	RightValue = ReplaceChild(RightValue, rightValueBase->CastTo(semanter, ExpressionResultType, Token)); 

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CAssignmentExpressionASTNode::Clone(CSemanter* semanter)
{
	CAssignmentExpressionASTNode* clone = new CAssignmentExpressionASTNode(NULL, Token);

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
//	Causes this node to be translated.
// =================================================================
int CAssignmentExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateAssignmentExpression(this);
}