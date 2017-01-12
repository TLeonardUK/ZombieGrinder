/* *****************************************************************

		CCaseStatementASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCASESTATEMENTASTNODE_H_
#define _CCASESTATEMENTASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"



class CExpressionASTNode;

// =================================================================
//	Stores information on an block statement.
// =================================================================
class CCaseStatementASTNode : public CASTNode
{
	

protected:	

public:
	CASTNode* BodyStatement;
	std::vector<CASTNode*> Expressions;

	CCaseStatementASTNode(CASTNode* parent, CToken token);
	
	virtual CASTNode* Clone(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual CASTNode* FindLoopScope(CSemanter* semanter);

	virtual bool AcceptBreakStatement();

};

#endif