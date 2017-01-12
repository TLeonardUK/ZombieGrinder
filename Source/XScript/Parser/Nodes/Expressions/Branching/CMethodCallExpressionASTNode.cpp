/* *****************************************************************

		CMethodCallExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"

#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CClassRefExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CNewExpressionASTNode.h"

#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CMethodCallExpressionASTNode::CMethodCallExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL),
	ResolvedDeclaration(NULL),
	RPCTargetExpression(NULL),
	RPCExceptExpression(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CMethodCallExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CMethodCallExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Semant left hand node.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));
	
	// Get expression representations.
	CExpressionBaseASTNode* left_hand_expr	 = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* right_hand_expr  = dynamic_cast<CExpressionBaseASTNode*>(RightValue);

	// Make sure we can access class.
	CClassASTNode* accessClass = left_hand_expr->ExpressionResultType->GetClass(semanter);
	if (accessClass == NULL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Invalid use of scoping operator."), Token);		
	}

	// Check we can access this class from here.
	accessClass->CheckAccess(semanter, this);

	// NOTE: Do not r-value semant identifier, we want to process that ourselves.
	CIdentifierExpressionASTNode* identNode = dynamic_cast<CIdentifierExpressionASTNode*>(RightValue);

	// Semant arguments.
	std::vector<CDataType*> argument_types;
	String argument_types_string;
	for (std::vector<CASTNode*>::iterator iter = ArgumentExpressions.begin(); iter < ArgumentExpressions.end(); iter++)
	{
		CExpressionBaseASTNode* node = dynamic_cast<CExpressionBaseASTNode*>((*iter)->Semant(semanter));
		argument_types.push_back(node->ExpressionResultType);

		if (iter != ArgumentExpressions.begin())
		{
			argument_types_string += ", ";
		}
		argument_types_string += node->ExpressionResultType->ToString();

		(*iter) = node;
	}

	// Make sure the identifier represents a valid field.
	CClassMemberASTNode* declaration = accessClass->FindClassMethod(semanter, identNode->Token.Literal, argument_types, false, NULL, this);
	if (declaration == NULL)
	{
		// Check if this is a class constructor, if it is, then turn us into a 'new X' expression.
		// TODO: This is kinda silly atm as you can do things like this.Vec4(0, 0, 0, 0); But eh, works for now.
		CDataType* type = FindDataType(semanter, identNode->Token.Literal, std::vector<CDataType*>()); 
		if (type != NULL)
		{
			CNewExpressionASTNode* new_expr = new CNewExpressionASTNode(Parent, Token);
			new_expr->DataType = type;
			new_expr->IsArray = false;
			new_expr->StructSyntax = true;
			
			for (std::vector<CASTNode*>::iterator iter = ArgumentExpressions.begin(); iter != ArgumentExpressions.end(); iter++)
			{
				CASTNode* arg = *iter;

				RemoveChild(arg);
				new_expr->AddChild(arg);
				new_expr->ArgumentExpressions.push_back(arg);
			}

			new_expr->Semant(semanter);

			return new_expr;
		}
		else
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Undefined method '%s(%s)' in class '%s'.", identNode->Token.Literal.c_str(), argument_types_string.c_str(), accessClass->ToString().c_str()), Token);		
		}
	}

	// Cannot call events directly! Dat shit is native->script only!
	// UPDATE: Why its just a normal function?
	//if (declaration->IsEvent == true)
	//{
	//	semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot call method '%s(%s)' in class '%s', method is defined as an event. Events can only be called from native code.", identNode->Token.Literal.c_str(), argument_types_string.c_str(), accessClass->ToString().c_str()), Token);	
	//}
	
	// Cannot call RPC directly!
	if (declaration->IsRPC == true && (RPCTargetExpression == NULL && RPCExceptExpression == NULL))
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot call method '%s(%s)' in class '%s', method is defined as an rpc event. Use rpc(from,to) %s(%s) to invoke RPC call.", identNode->Token.Literal.c_str(), argument_types_string.c_str(), accessClass->ToString().c_str(), identNode->Token.Literal.c_str(), argument_types_string.c_str()), Token);	
	}

// UPDATE: Abstract method calling is fine. Remember we won't be able to instantiate classes that do not override all abstract methods.
//	if (declaration->IsAbstract == true)
//	{
//		semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot call method '%s(%s)' in class '%s', method is abstract.", identNode->Token.Literal.c_str(), argument_types_string.c_str(), accessClass->ToString().c_str()), Token);		
//	}

	// Semant RPC values.
	if (RPCExceptExpression != NULL ||
		RPCTargetExpression != NULL)
	{
		CDataType* to = CIntDataType::StaticInstance;

		RPCExceptExpression->Semant(semanter);
		RPCExceptExpression = dynamic_cast<CExpressionASTNode*>(ReplaceChild(RPCExceptExpression, RPCExceptExpression->CastTo(semanter, to, Token)));

		RPCTargetExpression->Semant(semanter);
		RPCTargetExpression = dynamic_cast<CExpressionASTNode*>(ReplaceChild(RPCTargetExpression, RPCTargetExpression->CastTo(semanter, to, Token)));
	}
	
	ResolvedDeclaration = declaration;

	// Check we can access this field from here.
	declaration->CheckAccess(semanter, this);

	// HACK: This is really hackish and needs fixing!
	if (dynamic_cast<CThisExpressionASTNode*>(LeftValue) != NULL &&
		declaration->IsStatic == true)
	{		
		LeftValue = ReplaceChild(LeftValue, new CClassRefExpressionASTNode(NULL, Token));
		LeftValue->Token.Literal = declaration->FindClassScope(semanter)->Identifier.Get_String();
		LeftValue->Semant(semanter);

		left_hand_expr	 = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	}

	// Add default arguments if we do not have enough args to call.
	if (declaration->Arguments.size() > ArgumentExpressions.size())
	{
		for (unsigned int i = ArgumentExpressions.size(); i < declaration->Arguments.size() ; i++)
		{
			CASTNode* expr = declaration->Arguments.at(i)->AssignmentExpression->Clone(semanter);
			AddChild(expr);
			ArgumentExpressions.push_back(expr);

			expr->Semant(semanter);
		}
	}
	
	// Cast all arguments to correct data types.
	int index = 0;
	for (std::vector<CASTNode*>::iterator iter = ArgumentExpressions.begin(); iter != ArgumentExpressions.end(); iter++)
	{
		CDataType* dataType = declaration->Arguments.at(index++)->Type;

		CExpressionBaseASTNode* subnode = dynamic_cast<CExpressionBaseASTNode*>(*iter);
		subnode = dynamic_cast<CExpressionBaseASTNode*>(ReplaceChild(subnode, subnode->CastTo(semanter, dataType, Token)));
		(*iter) = subnode;
	}

	// If we are a class reference, we can only access static fields.
	bool isClassReference = (dynamic_cast<CClassReferenceDataType*>(left_hand_expr->ExpressionResultType) != NULL);
	if (isClassReference == true)
	{
		if (declaration->IsStatic == false &&
			declaration->IsNative == false)		// Exception is made for native methods so we can do things like have native instance constructors that return new instances.
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot access instance method '%s' through class reference '%s'.", declaration->Identifier.c_str(), accessClass->ToString().c_str()), Token);	
		}
	}

	// If this is a constructor we are calling, make sure we are in a constructors scope, or its illegal!
	else
	{
		CClassMemberASTNode* methodScope = FindClassMethodScope(semanter);

		if (methodScope == NULL ||
			methodScope->IsConstructor == false)
		{
			if (declaration->IsConstructor == true)
			{
				semanter->GetContext()->FatalError("Calling constructors manually is only valid inside another constructors scope.", Token);	
			}
		}
	}

	// Resulting type is always our right hand type.
	ExpressionResultType = declaration->ReturnType;

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CMethodCallExpressionASTNode::Clone(CSemanter* semanter)
{
	CMethodCallExpressionASTNode* clone = new CMethodCallExpressionASTNode(NULL, Token);

	for (std::vector<CASTNode*>::iterator iter = ArgumentExpressions.begin(); iter != ArgumentExpressions.end(); iter++)
	{
		CASTNode* node = (*iter)->Clone(semanter);
		clone->ArgumentExpressions.push_back(node);
		clone->AddChild(node);
	}
	
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

	if (RPCTargetExpression != NULL)
	{
		clone->RPCTargetExpression = dynamic_cast<CExpressionASTNode*>(RPCTargetExpression->Clone(semanter));
		clone->AddChild(clone->RPCTargetExpression);
	}

	if (RPCExceptExpression != NULL)
	{
		clone->RPCExceptExpression = dynamic_cast<CExpressionASTNode*>(RPCExceptExpression->Clone(semanter));
		clone->AddChild(clone->RPCExceptExpression);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CMethodCallExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateMethodCallExpression(this);
}