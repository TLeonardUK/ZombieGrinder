/* *****************************************************************

		CTypeOfExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CINDEXOFEXPRESSIONASTNODE_H_
#define _CINDEXOFEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

class CClassMemberASTNode;
class CExpressionASTNode;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CIndexOfExpressionASTNode : public CExpressionBaseASTNode
{
public:
	CExpressionBaseASTNode* Expression;
	CClassMemberASTNode* ClassMember;

protected:	

public:
	CIndexOfExpressionASTNode	(CASTNode* parent, CToken token);

	virtual CASTNode* Clone		(CSemanter* semanter);
	virtual CASTNode* Semant	(CSemanter* semanter);

	virtual int TranslateExpr(CTranslator* translator);

};

#endif