/* *****************************************************************

		CFieldAccessExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CFIELDACCESSEXPRESSIONASTNODE_H_
#define _CFIELDACCESSEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CClassMemberASTNode;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CFieldAccessExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	
	bool m_isSemantingRightValue;

public:
	CASTNode*				LeftValue;
	CASTNode*				RightValue;
	CClassMemberASTNode*	ExpressionResultClassMember;

	CClassMemberASTNode*	Property_GetMethod;
	CClassMemberASTNode*	Property_SetMethod;

	bool					IsPropertyAccess;

	bool					AllowNonStaticFieldReferences;

	CFieldAccessExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone									(CSemanter* semanter);
	virtual CASTNode* Semant								(CSemanter* semanter);
	
	virtual	EvaluationResult Evaluate						(CTranslationUnit* unit);

	virtual int TranslateExpr(CTranslator* translator);

};

#endif