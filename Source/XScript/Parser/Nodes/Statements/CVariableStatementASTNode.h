/* *****************************************************************

		CVariableStatementASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVARIABLESTATEMENTASTNODE_H_
#define _CVARIABLESTATEMENTASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"



#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"

class CDataType;
class CExpressionBaseASTNode;

// =================================================================
//	Stores information on an block statement.
// =================================================================
class CVariableStatementASTNode : public CDeclarationASTNode
{
	

protected:	

public:
	bool					IsParameter;
	CDataType*				Type;
	CExpressionBaseASTNode*	AssignmentExpression;

	CVariableStatementASTNode(CASTNode* parent, CToken token);
	
	virtual String ToString();

	virtual CASTNode* Clone	(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual void CheckAccess(CSemanter* semanter, CASTNode* referenceBy);

	virtual void Translate(CTranslator* translator);

};

#endif