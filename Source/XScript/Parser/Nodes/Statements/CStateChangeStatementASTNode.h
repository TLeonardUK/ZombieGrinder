/* *****************************************************************

		CStateChangeStatementASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CSTATECHANGESTATEMENTASTNODE_H_
#define _CSTATECHANGESTATEMENTASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"



class CExpressionASTNode;
class CClassStateASTNode;

// =================================================================
//	Stores information on an block statement.
// =================================================================
class CStateChangeStatementASTNode : public CASTNode
{
	

protected:	

public:
	String			StateIdentifier;
	CClassStateASTNode* State;

	CStateChangeStatementASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone	(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual void Translate(CTranslator* translator);

};

#endif