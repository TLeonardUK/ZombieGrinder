/* *****************************************************************

		CTypeOfExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CTYPEOFEXPRESSIONASTNODE_H_
#define _CTYPEOFEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

// =================================================================
//	Stores information on an expression.
// =================================================================
class CTypeOfExpressionASTNode : public CExpressionBaseASTNode
{
public:
	CDataType* Type;

protected:	

public:
	CTypeOfExpressionASTNode	(CASTNode* parent, CToken token);

	virtual CASTNode* Clone		(CSemanter* semanter);
	virtual CASTNode* Semant	(CSemanter* semanter);

	virtual int TranslateExpr(CTranslator* translator);

};

#endif