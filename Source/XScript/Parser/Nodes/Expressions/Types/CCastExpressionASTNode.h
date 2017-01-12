/* *****************************************************************

		CCastExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCASTEXPRESSIONASTNODE_H_
#define _CCASTEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CDataType;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CCastExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	bool		Explicit;	
	bool		ExceptionOnFail;
	CDataType*	Type;
	CASTNode*	RightValue;

	CCastExpressionASTNode(CASTNode* parent, CToken token, bool explicitCast);

	virtual CASTNode* Clone				(CSemanter* semanter);
	virtual CASTNode* Semant			(CSemanter* semanter);	
	
	virtual	EvaluationResult Evaluate	(CTranslationUnit* unit);

	static bool IsValidCast				(CSemanter* semanter, CDataType* from, CDataType* to, bool explicit_cast);	

	virtual int TranslateExpr(CTranslator* translator);

};

#endif