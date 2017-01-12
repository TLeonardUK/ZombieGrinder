/* *****************************************************************

		CNewExpressionASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CNEWEXPRESSIONASTNODE_H_
#define _CNEWEXPRESSIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CDataType;
class CArrayInitializerASTNode;

// =================================================================
//	Stores information on an expression.
// =================================================================
class CNewExpressionASTNode : public CExpressionBaseASTNode
{
	

protected:	

public:
	CDataType*					DataType;
	bool						IsArray;
	CClassMemberASTNode*		ResolvedConstructor;
	std::vector<CASTNode*>		ArgumentExpressions;
	CArrayInitializerASTNode*	ArrayInitializer;
	bool						StructSyntax;

	CNewExpressionASTNode(CASTNode* parent, CToken token);

	virtual CASTNode* Clone(CSemanter* semanter);
	virtual CASTNode* Semant(CSemanter* semanter);

	virtual CASTNode* Finalize(CSemanter* semanter);

	virtual int TranslateExpr(CTranslator* translator);

};

#endif