/* *****************************************************************

		CClassBodyASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCLASSBODYASTNODE_H_
#define _CCLASSBODYASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"



// =================================================================
//	Stores information on a class body.
// =================================================================
class CClassBodyASTNode : public CASTNode
{
	

protected:	

public:
	CClassBodyASTNode(CASTNode* parent, CToken token);
	
	// Semantic analysis.
	virtual CASTNode* Semant(CSemanter* semanter);
	virtual CASTNode* Clone	(CSemanter* semanter);
};

#endif