/* *****************************************************************

		CTypeExpressionASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Types/CTypeExpressionASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Types/CCastExpressionASTNode.h"

#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CTypeExpressionASTNode::CTypeExpressionASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token),
	Type(NULL),
	LeftValue(NULL)
{
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CTypeExpressionASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CTypeExpressionASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;
	
	// Semant expressions.
	LeftValue = ReplaceChild(LeftValue, LeftValue->Semant(semanter));
	Type	  = Type->Semant(semanter, this);

	// Get expression references.
	CExpressionBaseASTNode* lValueBase    = dynamic_cast<CExpressionBaseASTNode*>(LeftValue);
	
	// What operator.
	switch (Token.Type)
	{
		// herp as int
		case TokenIdentifier::KEYWORD_AS:
		{
			LeftValue			 = ReplaceChild(LeftValue, lValueBase->CastTo(semanter, Type, Token, true, false));
			ExpressionResultType = Type;

		//	if (dynamic_cast<CCastExpressionASTNode*>(LeftValue) != NULL)
		//	{
		//		dynamic_cast<CCastExpressionASTNode*>(LeftValue)->ExceptionOnFail = false;
		//	}

			return LeftValue;
		}

		// expr is int
		case TokenIdentifier::KEYWORD_IS:
		{
			// Left side must be object.
			if (dynamic_cast<CObjectDataType*>(lValueBase->ExpressionResultType) == NULL)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("L-Value of is keyword must be of type 'object'.", lValueBase->ExpressionResultType->ToString().c_str(), Type->ToString().c_str()), Token);
			}

			// Is this cast valid?
			if (!CCastExpressionASTNode::IsValidCast(semanter, lValueBase->ExpressionResultType, Type, true))
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot check cast for '%s' to '%s'.", lValueBase->ExpressionResultType->ToString().c_str(), Type->ToString().c_str()), Token);
			}

			ExpressionResultType = new CBoolDataType(Token);
			break;
		}

		// Wut O_o
		default:
		{
			semanter->GetContext()->FatalError("Internal error. Invalid type operator.", Token);
			break;
		}
	}

	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CTypeExpressionASTNode::Clone(CSemanter* semanter)
{
	CTypeExpressionASTNode* clone = new CTypeExpressionASTNode(NULL, Token);
	clone->Type = Type;
	
	if (LeftValue != NULL)
	{
		clone->LeftValue = dynamic_cast<CASTNode*>(LeftValue->Clone(semanter));
		clone->AddChild(clone->LeftValue);
	}

	return clone;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CTypeExpressionASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateTypeExpression(this);
}