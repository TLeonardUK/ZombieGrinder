/* *****************************************************************

		CForStatementASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CFORSTATEMENTASTNODE_H_
#define _CFORSTATEMENTASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"



class CExpressionBaseASTNode;

// =================================================================
//	Stores information on an block statement.
// =================================================================
class CForStatementASTNode : public CASTNode
{
	

protected:	

public:
	CASTNode*				InitialStatement;	
	CExpressionBaseASTNode*	ConditionExpression;
	CExpressionBaseASTNode*	IncrementExpression;

	CASTNode* BodyStatement;

	CForStatementASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual CASTNode* FindLoopScope(CSemanter* semanter);
	
	virtual bool AcceptBreakStatement();
	virtual bool AcceptContinueStatement();

	virtual void Translate(CTranslator* translator);

};

#endif