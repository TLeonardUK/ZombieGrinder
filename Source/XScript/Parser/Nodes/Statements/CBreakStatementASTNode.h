/* *****************************************************************

		CBreakStatementASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CBREAKSTATEMENTASTNODE_H_
#define _CBREAKSTATEMENTASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"



// =================================================================
//	Stores information on an block statement.
// =================================================================
class CBreakStatementASTNode : public CASTNode
{
	

protected:	

public:
	CBreakStatementASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone	(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual void Translate(CTranslator* translator);

};

#endif