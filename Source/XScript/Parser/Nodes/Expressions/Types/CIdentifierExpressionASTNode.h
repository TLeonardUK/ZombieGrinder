/* *****************************************************************

		CIdentifierExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CIDENTIFIEREXPRESSIONASTNODE_H_
#define _CIDENTIFIEREXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"



// =================================================================
//	Stores information on an expression.
// =================================================================
class CIdentifierExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	CClassMemberASTNode*		ExpressionResultClassMember;
	CVariableStatementASTNode*	ExpressionResultVariable;
	CDeclarationASTNode*		ResolvedDeclaration;
	std::vector<CDataType*>		GenericTypes;

	CIdentifierExpressionASTNode(CASTNode* parent, CToken token);
	
	virtual CASTNode* Clone	(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual int TranslateExpr(CTranslator* translator);
};

#endif