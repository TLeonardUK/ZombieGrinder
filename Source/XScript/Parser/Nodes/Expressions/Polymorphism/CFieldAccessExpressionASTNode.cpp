/* *****************************************************************

		CFieldAccessExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CIndexOfExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CClassRefExpressionASTNode.h"

#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Semanter/CSemanter.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CFieldAccessExpressionASTNode::CFieldAccessExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	LeftValue(NULL),
	RightValue(NULL),
	ExpressionResultClassMember(NULL),
	m_isSemantingRightValue(false),
	IsPropertyAccess(false),
	Property_GetMethod(NULL),
	Property_SetMethod(NULL),
	AllowNonStaticFieldReferences(false)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CFieldAccessExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CFieldAccessExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	// Get expression representations.
	CExpressionBaseASTNode* left_hand_expr	 = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	CExpressionBaseASTNode* right_hand_expr  = dynamic_cast<CExpressionBaseASTNode*>(RightValue);

	// Semant left hand node.
	LeftValue  = ReplaceChild(LeftValue,   LeftValue->Semant(semanter));
	
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

	// Make sure the identifier represents a valid field.
	CClassMemberASTNode* declaration = accessClass->FindClassField(semanter, identNode->Token.Literal, NULL, this);
	IsPropertyAccess = false;
	if (declaration == NULL)
	{
		// If identifier dosen't exist, see if a Get_Identifier or Set_Identifier methods exist
		// if they do then we treat this as a property access.
		CDeclarationIdentifier get_ident = CDeclarationIdentifier(StringHelper::Format("Get_%s", identNode->Token.Literal.c_str()).c_str());

		declaration = accessClass->FindClassMethod(semanter, get_ident, std::vector<CDataType*>(), true);

		if (declaration == NULL)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Undefined field '%s' in class '%s'.", identNode->Token.Literal.c_str(), accessClass->ToString().c_str()), Token);		
		}
		else
		{
		//	if (declaration->IsStatic == true)
		//	{
		//		semanter->GetContext()->FatalError(CStringHelper::FormatString("Invalid property-get method '%s' in class '%s'.", identNode->Token.Literal.c_str(), accessClass->ToString().c_str()), Token);		
		//	}
		//	else
		//	{
				std::vector<CDataType*> args;
				args.push_back(declaration->ReturnType);

				CDeclarationIdentifier set_ident = CDeclarationIdentifier(StringHelper::Format("Set_%s", identNode->Token.Literal.c_str()).c_str());

				IsPropertyAccess = true;
				Property_GetMethod = declaration;
				Property_SetMethod = accessClass->FindClassMethod(semanter, set_ident, args, true);
		//	}
		}
	}
	identNode->ResolvedDeclaration = declaration;

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

	// Is part of indexof expression?
	bool isIndexOf = (dynamic_cast<CIndexOfExpressionASTNode*>(Parent) != NULL);

	// If we are a class reference, we can only access static fields.
	bool isClassReference = (dynamic_cast<CClassReferenceDataType*>(left_hand_expr->ExpressionResultType) != NULL);
	if (isClassReference == true)
	{
		if (declaration->IsStatic == false && isIndexOf == false)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot access instance field '%s' through class reference '%s'.", declaration->Identifier.c_str(), accessClass->ToString().c_str()), Token);	
		}
	}

	// Resulting type is always our right hand type.
	ExpressionResultClassMember	 = declaration;
	ExpressionResultType		 = declaration->ReturnType;

	ExpressionResultType		 = ExpressionResultType->Semant(semanter, this);
	
	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CFieldAccessExpressionASTNode::Clone(CSemanter* semanter)
{
	CFieldAccessExpressionASTNode* clone = new CFieldAccessExpressionASTNode(NULL, Token);

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
int CFieldAccessExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateFieldAccessExpression(this);
}

// =================================================================
//	Statically evaluates an expression.
// =================================================================
EvaluationResult CFieldAccessExpressionASTNode::Evaluate(CTranslationUnit* unit)
{
	if (!Semanted)
	{
		Semant(unit->GetSemanter());
	}

	if (ExpressionResultClassMember->MemberType == MemberType::Field &&
		ExpressionResultClassMember->FindClassScope(unit->GetSemanter())->IsEnum == true)
	{
		return ExpressionResultClassMember->Assignment->Evaluate(unit);
	}
	else
	{
		return CASTNode::Evaluate(unit);
	}
}
