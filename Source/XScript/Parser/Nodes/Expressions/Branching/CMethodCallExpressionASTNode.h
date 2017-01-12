/* *****************************************************************

		CMethodCallExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CMETHODCALLEXPRESSIONASTNODE_H_
#define _CMETHODCALLEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



// =================================================================
//	Stores information on an expression.
// =================================================================
class CMethodCallExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	std::vector<CASTNode*> ArgumentExpressions;
	CASTNode* RightValue;
	CASTNode* LeftValue;
	CDeclarationASTNode* ResolvedDeclaration;
	CExpressionBaseASTNode* RPCTargetExpression;
	CExpressionBaseASTNode* RPCExceptExpression;

	CMethodCallExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone		(CSemanter* semanter);
	virtual CASTNode* Semant	(CSemanter* semanter);	

	virtual int TranslateExpr(CTranslator* translator);

};

#endif