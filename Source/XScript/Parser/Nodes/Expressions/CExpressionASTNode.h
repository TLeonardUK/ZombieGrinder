/* *****************************************************************

		CExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CEXPRESSIONASTNODE_H_
#define _CEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"



#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

// =================================================================
//	Stores information on an expression.
// =================================================================
class CExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	bool		IsConstant;
	CASTNode*	LeftValue;

	CExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone					(CSemanter* semanter);
	virtual CASTNode* Semant				(CSemanter* semanter);	
	
	virtual	EvaluationResult Evaluate		(CTranslationUnit* unit);
	
	virtual void			Translate		(CTranslator* translator);
	virtual int TranslateExpr(CTranslator* translator);

};

#endif