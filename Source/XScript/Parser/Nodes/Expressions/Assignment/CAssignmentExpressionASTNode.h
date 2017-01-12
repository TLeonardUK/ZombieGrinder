/* *****************************************************************

		CAssignmentExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CASSIGNMENTEXPRESSIONASTNODE_H_
#define _CASSIGNMENTEXPRESSIONASTNODE_H_

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
class CAssignmentExpressionASTNode : public CExpressionBaseASTNode
{
protected:	

public:
	CASTNode* LeftValue;
	CASTNode* RightValue;
	bool IgnoreConst;

	CAssignmentExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone		(CSemanter* semanter);
	virtual CASTNode* Semant	(CSemanter* semanter);	
	
	virtual int TranslateExpr(CTranslator* translator);

};

#endif