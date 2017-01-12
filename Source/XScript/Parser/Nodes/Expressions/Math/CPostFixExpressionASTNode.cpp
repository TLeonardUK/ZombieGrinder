/* *****************************************************************

		CPostFixExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Math/CPostFixExpressionASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIndexExpressionASTNode.h"

#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CPostFixExpressionASTNode::CPostFixExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CPostFixExpressionASTNode::Clone(CSemanter* semanter)
{
	CPostFixExpressionASTNode* clone = new CPostFixExpressionASTNode(NULL, Token);

	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}

	return clone;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CPostFixExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CPostFixExpressionASTNode");

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

	// Balance types.
	switch (Token.Type)
	{	
		case TokenIdentifier::OP_INCREMENT:
		case TokenIdentifier::OP_DECREMENT:	
		{
			if (dynamic_cast<CIntDataType*>(ExpressionResultType) == NULL)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Postfix operator '%s' only supports integer l-value's.", Token.Literal.c_str()), Token);
			}
			break;
		}
		default:
		{
			semanter->GetContext()->FatalError("Internal error. Invalid postfix operator.", Token);
			break;
		}
	}

	return this;
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CPostFixExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	EvaluationResult leftResult  = LeftValue->Evaluate(unit);

	switch (Token.Type)
	{	
		case TokenIdentifier::OP_INCREMENT:
		case TokenIdentifier::OP_DECREMENT:	
		{
			break;
		}
	}

	unit->FatalError("Invalid postfix operation, increment and decrement operators cannot be evaluated.", Token);
	return EvaluationResult(false);
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CPostFixExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslatePostFixExpression(this);
}