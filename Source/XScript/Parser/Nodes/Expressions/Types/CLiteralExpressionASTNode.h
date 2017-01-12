/* *****************************************************************

		CLiteralExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CLITERALEXPRESSIONASTNODE_H_
#define _CLITERALEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CDataType;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CLiteralExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	CDataType*			  Type;
	String			  Literal;

	int					  StringTableIndex;
	
	CLiteralExpressionASTNode(CASTNode* parent, CToken token, CDataType* type, String lit);
	
	virtual CASTNode* Clone					(CSemanter* semanter);
	virtual CASTNode* Semant				(CSemanter* semanter);
	
	virtual	EvaluationResult Evaluate(CTranslationUnit* unit);

	virtual int TranslateExpr(CTranslator* translator);
};

#endif