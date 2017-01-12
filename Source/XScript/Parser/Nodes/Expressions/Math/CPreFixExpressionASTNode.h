/* *****************************************************************

		CPreFixExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CPREFIXEXPRESSIONASTNODE_H_
#define _CPREFIXEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



// =================================================================
//	Stores information on an expression.
// =================================================================
class CPreFixExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	
	CASTNode* PerformOperatorOverride(CSemanter* semanter, CExpressionBaseASTNode* lvalue);

public:
	CASTNode* LeftValue;

	CPreFixExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone		(CSemanter* semanter);
	virtual CASTNode* Semant	(CSemanter* semanter);	
	
	virtual	EvaluationResult Evaluate(CTranslationUnit* unit);

	virtual int TranslateExpr(CTranslator* translator);

};

#endif