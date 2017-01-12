/* *****************************************************************

		CLogicalExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CLOGICALEXPRESSIONASTNODE_H_
#define _CLOGICALEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CExpressionASTNode;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CLogicalExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	CASTNode* LeftValue;
	CASTNode* RightValue;

	CLogicalExpressionASTNode(CASTNode* parent, CToken token);
	
	virtual CASTNode* Clone		(CSemanter* semanter);
	virtual CASTNode* Semant	(CSemanter* semanter);	
	
	virtual	EvaluationResult Evaluate(CTranslationUnit* unit);

	virtual int TranslateExpr(CTranslator* translator);

};

#endif