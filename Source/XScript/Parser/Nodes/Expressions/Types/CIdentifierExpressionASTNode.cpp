/* *****************************************************************

		CIdentifierExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassPropertyASTNode.h"
#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CIdentifierExpressionASTNode::CIdentifierExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	ExpressionResultClassMember(NULL),
	ExpressionResultVariable(NULL),
	ResolvedDeclaration(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CIdentifierExpressionASTNode::Clone(CSemanter* semanter)
{
	CIdentifierExpressionASTNode* clone = new CIdentifierExpressionASTNode(NULL, Token);

	return clone;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CIdentifierExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CIdentifierExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Declaration priority:
	//		Field
	//		Method
	//		Class
	//		Last two get swapped if our parent is a field access operator.

	// Find the declaration specified.
	CClassASTNode*		 classScope	 = this->FindClassScope(semanter);
	CDeclarationASTNode* declaration = NULL;

	CFieldAccessExpressionASTNode* parentFieldAccess = dynamic_cast<CFieldAccessExpressionASTNode*>(Parent);
	CMethodCallExpressionASTNode*  parentMethodCall  = dynamic_cast<CMethodCallExpressionASTNode*>(Parent);

	// Generic class reference?
	if (GenericTypes.size() > 0)
	{
		CDataType* type = this->FindDataType(semanter, Token.Literal, GenericTypes);
		declaration = type->GetClass(semanter);
	}

	// We are the left side of a field access?
	else if ((parentFieldAccess != NULL && parentFieldAccess->LeftValue == this))
	{
		declaration = this->FindClassField(semanter, Token.Literal, NULL);
		if (declaration == NULL)
		{
			declaration = this->FindDataTypeDeclaration(semanter, Token.Literal);
		}
		if (declaration == NULL)
		{
			declaration = this->FindDeclaration(semanter, Token.Literal);
		}
	}
	
	// We are the left side of a method call?
	else if ((parentMethodCall != NULL && parentMethodCall->LeftValue == this))
	{
		declaration = this->FindClassField(semanter, Token.Literal, NULL);
		if (declaration == NULL)
		{
			declaration = this->FindDataTypeDeclaration(semanter, Token.Literal);
		}
		if (declaration == NULL)
		{
			declaration = this->FindDeclaration(semanter, Token.Literal);		

			if (declaration == NULL || dynamic_cast<CClassPropertyASTNode*>(declaration) != NULL)
			{
				// See if its a property.
				String get_name = StringHelper::Format("Get_%s", Token.Literal.c_str());
				declaration = this->FindClassMethod(semanter, get_name, std::vector<CDataType*>(), true, NULL, this);

				if (declaration)
				{
					// Ok as its a property we need to do some tricky shit to convert it into a field access.
					CFieldAccessExpressionASTNode* fa = new CFieldAccessExpressionASTNode(NULL, Token);
					fa->LeftValue = new CThisExpressionASTNode(NULL, Token);
					fa->RightValue = this;

					CASTNode* original_parent = Parent;

					fa->AddChild(this);
					fa->AddChild(fa->LeftValue);

					//
					original_parent->ReplaceChild(this, fa);
					Semanted = false;

					return fa->Semant(semanter);
				}
			}
		}
	}

	// Just a general identifier.
	else
	{
		declaration = this->FindClassField(semanter, Token.Literal, NULL);
		if (declaration == NULL)
		{
			declaration = this->FindDeclaration(semanter, Token.Literal);			

			if (declaration == NULL || dynamic_cast<CClassPropertyASTNode*>(declaration) != NULL)
			{
				// See if its a property.
				String get_name = StringHelper::Format("Get_%s", Token.Literal.c_str());
				declaration = this->FindClassMethod(semanter, get_name, std::vector<CDataType*>(), true, NULL, this);

				if (declaration)
				{
					// Ok as its a property we need to do some tricky shit to convert it into a field access.
					CFieldAccessExpressionASTNode* fa = new CFieldAccessExpressionASTNode(NULL, Token);
					fa->LeftValue = new CThisExpressionASTNode(NULL, Token);
					fa->RightValue = this;

					CASTNode* original_parent = Parent;

					fa->AddChild(this);
					fa->AddChild(fa->LeftValue);

					//
					original_parent->ReplaceChild(this, fa);
					Semanted = false;

					return fa->Semant(semanter);
				}
			}
		}
	}

	ResolvedDeclaration = declaration; 

	if (declaration == NULL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Undefined identifier '%s'.", Token.Literal.c_str()), Token);		
	}

	// Check access.
	declaration->CheckAccess(semanter, this);

	// Work out result type.
	CClassASTNode*				classNode		= dynamic_cast<CClassASTNode*>(declaration);
	CClassMemberASTNode*		memberNode		= dynamic_cast<CClassMemberASTNode*>(declaration);
	CVariableStatementASTNode*	localNode		= dynamic_cast<CVariableStatementASTNode*>(declaration);
	
	// Check we are not accessing an instance
	// variable from a static method.	
	if (memberNode != NULL)
	{
		CClassMemberASTNode* methodScope = FindClassMethodScope(semanter);
		if (methodScope->IsStatic == true &&
			memberNode->IsStatic  == false)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot acccess instance field '%s' in static method.", memberNode->Identifier.c_str()), Token);		
		}

		ExpressionResultType = memberNode->ReturnType;
		ExpressionResultClassMember = memberNode;
	}

	// Is this a local variable?
	else if (localNode != NULL)
	{
		ExpressionResultType	 = localNode->Type;
		ExpressionResultVariable = localNode;
	}

	// If this is a class reference, make sure we are the left-value of a scope expression
	// you should not be able to have a class reference as an r-value.
	else if (classNode != NULL)
	{
		if (classNode->IsGeneric == true && GenericTypes.size() <= 0)
		{
		//	semanter->GetContext()->FatalError(CStringHelper::FormatString("Referencing static members of generic classes is currently unsupported.", classNode->Identifier.c_str()), Token);			
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Reference to class '%s' expects declaration of generic types.", classNode->Identifier.c_str()), Token);			
		}
		else if (classNode->IsGeneric == false && GenericTypes.size() > 0)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Reference to class '%s' does not expect declaration of generic types.", classNode->Identifier.c_str()), Token);			
		}
		ExpressionResultType = classNode->ClassReferenceDataType;
	}

	ExpressionResultType = ExpressionResultType->Semant(semanter, this);

	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CIdentifierExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateIdentifierExpression(this);
}