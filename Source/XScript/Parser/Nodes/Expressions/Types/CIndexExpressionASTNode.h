/* *****************************************************************

		CIndexExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CINDEXEXPRESSIONASTNODE_H_
#define _CINDEXEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CClassMemberASTNode;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CIndexExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	CASTNode* LeftValue;
	CASTNode* IndexExpression;

	CIndexExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone	(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual int TranslateExpr(CTranslator* translator);
};

#endif