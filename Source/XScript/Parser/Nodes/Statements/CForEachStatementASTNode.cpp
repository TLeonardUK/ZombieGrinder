/* *****************************************************************

		CForEachStatementASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Statements/CForEachStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIndexExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"
#include "XScript/Parser/Nodes/Statements/CWhileStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CBlockStatementASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Assignment/CAssignmentExpressionASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CForEachStatementASTNode::CForEachStatementASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token),
	VariableStatement(NULL),
	ExpressionStatement(NULL),
	BodyStatement(NULL),
	OriginalBodyStatement(NULL)
{
}

// =================================================================
//	Returns true if this node can accept break statements inside
//	of it.
// =================================================================
bool CForEachStatementASTNode::AcceptBreakStatement()
{
	return true;
}

// =================================================================
//	Returns true if this node can accept continue statements inside
//	of it.
// =================================================================
bool CForEachStatementASTNode::AcceptContinueStatement()
{
	return true;
}

// =================================================================
// Performs semantic analysis of this node.
// =================================================================
CASTNode* CForEachStatementASTNode::Semant(CSemanter* semanter)
{	
	SEMANT_TRACE("CForEachStatementASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;
	
	CDeclarationIdentifier s_ienumerable_ident("IEnumerable");

	// Find enumerable base.
	CDataType* enumerable_base = FindDataType(semanter, s_ienumerable_ident, std::vector<CDataType*>());
	if (enumerable_base == NULL ||
		enumerable_base->GetClass(semanter) == NULL)
	{
		semanter->GetContext()->FatalError("Internal error, could not find base 'IEnumerable' class.");
	}

	// Check expression is valid.
	if (ExpressionStatement != NULL)
	{
		ExpressionStatement = dynamic_cast<CExpressionBaseASTNode*>(ExpressionStatement->Semant(semanter));

		CDataType* dataType = ExpressionStatement->ExpressionResultType->GetClass(semanter)->ObjectDataType;

		if (!dataType->CanCastTo(semanter, enumerable_base))
		{
			semanter->GetContext()->FatalError("ForEach expressions must implement 'IEnumerable' interface.", Token);	
		}
	}

	// Check variable declaration is a valid l-value.
	VariableStatement = VariableStatement->Semant(semanter);
	CExpressionASTNode* varExpr = dynamic_cast<CExpressionASTNode*>(VariableStatement);
	if (varExpr != NULL)
	{
		// Try and find field the l-value is refering to.
		CClassMemberASTNode*		   field_node		 = NULL;
		CVariableStatementASTNode*	   var_node			 = NULL;
		CFieldAccessExpressionASTNode* field_access_node = dynamic_cast<CFieldAccessExpressionASTNode*>(varExpr->LeftValue);
		CIdentifierExpressionASTNode*  identifier_node   = dynamic_cast<CIdentifierExpressionASTNode*>(varExpr->LeftValue);
		CIndexExpressionASTNode*	   index_node		 = dynamic_cast<CIndexExpressionASTNode*>(varExpr->LeftValue);

		if (index_node != NULL)
		{
			// Should call CanAssignIndex or something
			CExpressionBaseASTNode* leftLeftValueBase  = dynamic_cast<CExpressionBaseASTNode*>(index_node->LeftValue);
			CExpressionBaseASTNode* indexIndexValueBase = dynamic_cast<CExpressionBaseASTNode*>(index_node->IndexExpression);

			std::vector<CDataType*> args;
			args.push_back(indexIndexValueBase->ExpressionResultType);
			args.push_back(varExpr->ExpressionResultType);

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
	}

	// Right nows the fun bit! We're going to construct part of the AST tree in-place. The result should be
	// the AST equivilent of this;
	//
	//	ls_IEnumerator* internal = (expr)->GetEnumerator();
	//	while (internal->Next())
	//	{
	//		object value = internal->Current();
	//		(body)
	//	}
	//

	// If we are doing an array we actually do this:
	// Array[] internal = (expr);
	// int index = 0;
	// while (index < internal->Length())
	// {
	//		object value = internal[index++];
	//		(body)
	// }

	OriginalBodyStatement = BodyStatement;

	CBlockStatementASTNode*			new_body									= new CBlockStatementASTNode(this, Token);
	CVariableStatementASTNode*		enumerator_var								= new CVariableStatementASTNode(new_body, Token);
	CMethodCallExpressionASTNode*	get_enum_method_call_expr					= new CMethodCallExpressionASTNode(enumerator_var, Token);
	CIdentifierExpressionASTNode*	get_enum_identifier_expr					= new CIdentifierExpressionASTNode(get_enum_method_call_expr, Token);
	CWhileStatementASTNode*			while_loop									= new CWhileStatementASTNode(new_body, Token);
	CMethodCallExpressionASTNode*	while_loop_method_call_expr					= new CMethodCallExpressionASTNode(while_loop, Token);
	CIdentifierExpressionASTNode*	while_loop_identifier_expr					= new CIdentifierExpressionASTNode(while_loop_method_call_expr, Token);
	CIdentifierExpressionASTNode*	while_loop_var_ref_expr						= new CIdentifierExpressionASTNode(while_loop_method_call_expr, Token);
	CBlockStatementASTNode*			while_body									= new CBlockStatementASTNode(while_loop, Token);

	//	CVariableStatementASTNode*		value_var				= new CVariableStatementASTNode(while_body, Token);

	RemoveChild(VariableStatement);
	RemoveChild(ExpressionStatement);
	RemoveChild(BodyStatement);
//	AddChild(new_body);
//	new_body->AddChild(enumerator_var);
//	new_body->AddChild(while_loop);

//	enumerator_var->AddChild(get_enum_method_call_expr);
	get_enum_method_call_expr->AddChild(ExpressionStatement);
//	get_enum_method_call_expr->AddChild(get_enum_identifier_expr);
//	while_loop->AddChild(while_body);
//	while_loop->AddChild(while_loop_method_call_expr);
//	while_loop_method_call_expr->AddChild(while_loop_identifier_expr);
//	while_loop_method_call_expr->AddChild(while_loop_var_ref_expr);

	// ls_IEnumerator* internal = (expr)->GetEnumerator();
	get_enum_identifier_expr->Token.Literal = "GetEnumerator"; 

	get_enum_method_call_expr->LeftValue = ExpressionStatement;
	get_enum_method_call_expr->RightValue = get_enum_identifier_expr;

	CDeclarationIdentifier s_ienumerator_ident("IEnumerator");

	enumerator_var->Type = FindDataType(semanter, s_ienumerator_ident, std::vector<CDataType*>());
	enumerator_var->AssignmentExpression = get_enum_method_call_expr;
	enumerator_var->Identifier = semanter->NewInternalVariableName();
	enumerator_var->MangledIdentifier = enumerator_var->Identifier.Get_String();

	// while (internal->Next())
	while_loop_var_ref_expr->Token.Literal		= enumerator_var->Identifier.Get_String(); 
	while_loop_identifier_expr->Token.Literal	= "Next"; 

	while_loop_method_call_expr->LeftValue = while_loop_var_ref_expr;
	while_loop_method_call_expr->RightValue = while_loop_identifier_expr;

	while_loop->BodyStatement = while_body;
	while_loop->ExpressionStatement = while_loop_method_call_expr;

	// object value = internal->Current();
	if (varExpr != NULL)
	{	
		CExpressionASTNode*				value_assign_expr						= new CExpressionASTNode(while_body, Token);
		CAssignmentExpressionASTNode*	value_assign							= new CAssignmentExpressionASTNode(value_assign_expr, Token);
		CMethodCallExpressionASTNode*	value_assign_method_call_expr			= new CMethodCallExpressionASTNode(value_assign, Token);
		CIdentifierExpressionASTNode*	value_assign_method_call_lvalue_expr	= new CIdentifierExpressionASTNode(value_assign_method_call_expr, Token);
		CIdentifierExpressionASTNode*	value_assign_method_call_rvalue_expr	= new CIdentifierExpressionASTNode(value_assign_method_call_expr, Token);

		value_assign_expr->LeftValue = value_assign;

		value_assign_method_call_lvalue_expr->Token.Literal	= enumerator_var->Identifier.Get_String(); 
		value_assign_method_call_rvalue_expr->Token.Literal	= "Current"; 

		value_assign_method_call_expr->LeftValue = value_assign_method_call_lvalue_expr;
		value_assign_method_call_expr->RightValue = value_assign_method_call_rvalue_expr;

		value_assign->LeftValue = varExpr->LeftValue;
		value_assign->RightValue = value_assign_method_call_expr;
		value_assign->Token.Type = TokenIdentifier::OP_ASSIGN;

		value_assign->AddChild(varExpr->LeftValue);

		CExpressionBaseASTNode* left_base = dynamic_cast<CExpressionBaseASTNode*>(value_assign->LeftValue);
		CExpressionBaseASTNode* right_base = dynamic_cast<CExpressionBaseASTNode*>(value_assign->RightValue);
		left_base->Semant(semanter);
		right_base->Semant(semanter);
		value_assign->RightValue = value_assign->ReplaceChild(value_assign->RightValue, right_base->CastTo(semanter, left_base->ExpressionResultType, Token, true, true));
	}
	else
	{		
		CVariableStatementASTNode* var_node = dynamic_cast<CVariableStatementASTNode*>(VariableStatement);

		CMethodCallExpressionASTNode*	value_assign_method_call_expr			= new CMethodCallExpressionASTNode(VariableStatement, Token);
		CIdentifierExpressionASTNode*	value_assign_method_call_lvalue_expr	= new CIdentifierExpressionASTNode(value_assign_method_call_expr, Token);
		CIdentifierExpressionASTNode*	value_assign_method_call_rvalue_expr	= new CIdentifierExpressionASTNode(value_assign_method_call_expr, Token);

		value_assign_method_call_lvalue_expr->Token.Literal	= enumerator_var->Identifier.Get_String(); 
		value_assign_method_call_rvalue_expr->Token.Literal	= "Current"; 

		value_assign_method_call_expr->LeftValue = value_assign_method_call_lvalue_expr;
		value_assign_method_call_expr->RightValue = value_assign_method_call_rvalue_expr;

		var_node->RemoveChild(var_node->AssignmentExpression);
		while_body->AddChild(var_node);

		var_node->AssignmentExpression = value_assign_method_call_expr;

		value_assign_method_call_expr->Semant(semanter);
		CASTNode* casted_node = value_assign_method_call_expr->CastTo(semanter, var_node->Type, Token, true, true);
		VariableStatement->ReplaceChild(value_assign_method_call_expr, casted_node);
		var_node->AssignmentExpression = dynamic_cast<CExpressionBaseASTNode*>(casted_node);
	}

	while_body->AddChild(BodyStatement);

	// Replace body statement.
	BodyStatement = new_body;

	// Parse dat body.
	if (BodyStatement != NULL)
	{
		BodyStatement = BodyStatement->Semant(semanter);
	}
	if (OriginalBodyStatement != NULL)
	{
		OriginalBodyStatement = OriginalBodyStatement->Semant(semanter);
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CForEachStatementASTNode::Clone(CSemanter* semanter)
{
	CForEachStatementASTNode* clone = new CForEachStatementASTNode(NULL, Token);
	
	if (VariableStatement != NULL)
	{
		clone->VariableStatement = dynamic_cast<CASTNode*>(VariableStatement->Clone(semanter));
		clone->AddChild(clone->VariableStatement);
	}
	if (ExpressionStatement != NULL)
	{
		clone->ExpressionStatement = dynamic_cast<CExpressionASTNode*>(ExpressionStatement->Clone(semanter));
		clone->AddChild(clone->ExpressionStatement);
	}
	if (BodyStatement != NULL)
	{
		clone->BodyStatement = dynamic_cast<CASTNode*>(BodyStatement->Clone(semanter));
		clone->AddChild(clone->BodyStatement);
	}

	return clone;
}

// =================================================================
//	Finds the scope the looping statement this node is contained by.
// =================================================================
CASTNode* CForEachStatementASTNode::FindLoopScope(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CForEachStatementASTNode::Translate(CTranslator* translator)
{
	translator->TranslateForEachStatement(this);
}