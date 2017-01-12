/* *****************************************************************

		CArrayInitializerASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/Expressions/Types/CArrayInitializerASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Nodes/Expressions/Polymorphism/CNewExpressionASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CArrayInitializerASTNode::CArrayInitializerASTNode(CASTNode* parent, CToken token) :
	CExpressionBaseASTNode(parent, token) ,
	ExpectedDataType(NULL)
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CArrayInitializerASTNode::Clone(CSemanter* semanter)
{
	CArrayInitializerASTNode* clone = new CArrayInitializerASTNode(NULL, Token);

	for (std::vector<CASTNode*>::iterator iter = Expressions.begin(); iter != Expressions.end(); iter++)
	{
		CASTNode* node = (*iter)->Clone(semanter);
		clone->Expressions.push_back(node);
		clone->AddChild(node);
	}

	return clone;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CArrayInitializerASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CArrayInitializerASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	ExpressionResultType = ExpectedDataType;

	// Semant all expressions.
	for (std::vector<CASTNode*>::iterator iter = Expressions.begin(); iter != Expressions.end(); iter++)
	{
		CExpressionBaseASTNode* node = dynamic_cast<CExpressionBaseASTNode*>(*iter);
		node->Semant(semanter);

		if (ExpressionResultType == NULL)
		{
			ExpressionResultType = node->ExpressionResultType;
		}
		else
		{
			if (!ExpressionResultType->IsEqualTo(semanter, node->ExpressionResultType))
			{
				// Cast to correct type.
				if (ExpressionResultType->CanCastTo(semanter, node->ExpressionResultType))
				{
					CASTNode* casted_node = node->CastTo(semanter, ExpressionResultType, Token, false);
					casted_node = ReplaceChild(node, casted_node);
					*iter = casted_node;
				}
				else
				{
					if (ExpectedDataType != NULL)
						semanter->GetContext()->FatalError(StringHelper::Format("Cannot cast all expressions in array initializer to infered type '%s'.", ExpressionResultType->ToString().c_str()).c_str(), node->Token);
					else
						semanter->GetContext()->FatalError(StringHelper::Format("Cannot cast all expressions in array initializer to expected type '%s'.", ExpressionResultType->ToString().c_str()).c_str(), node->Token);
				}
			}
		}
	}

	ExpressionResultType = ExpressionResultType->ArrayOf();

	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
int CArrayInitializerASTNode::TranslateExpr(CTranslator* translator)
{
	return translator->TranslateArrayInitializerExpression(this);
}