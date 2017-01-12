/* *****************************************************************

		CConstDictionaryASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCONSTDICTIONARYASTNODE_H_
#define _CCONSTDICTIONARYASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"



class CClassBodyASTNode;
class CIdentifierDataType;
class CConstDictionaryASTNode;
class CObjectDataType;
class CClassReferenceDataType;
class CSymbol;

class CConstDictionaryValue
{
public:
	String							Identifier;
	CExpressionBaseASTNode*				ValueExpr;
};

// =================================================================
//	Stores information on a class declaration.
// =================================================================
class CConstDictionaryASTNode : public CExpressionBaseASTNode
{
	

protected:	
	bool								m_semanting;

public:

	// Parsing infered.
	std::vector<CConstDictionaryValue>	Values;

	// General management.
	virtual String ToString();

	// Initialization.
	CConstDictionaryASTNode(CASTNode* parent, CToken token);
	~CConstDictionaryASTNode();
	
	// Semantic analysis.
	virtual CASTNode*				Semant					(CSemanter* semanter);
	virtual CASTNode*				Finalize				(CSemanter* semanter);	
	virtual CASTNode*				Clone					(CSemanter* semanter);

	virtual void					Translate				(CTranslator* translator);
	virtual int						TranslateExpr			(CTranslator* translator);

};

#endif