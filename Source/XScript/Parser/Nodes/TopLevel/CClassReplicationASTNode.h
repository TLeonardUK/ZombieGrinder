/* *****************************************************************

		CClassReplicationASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCLASSREPLICATIONASTNODE_H_
#define _CCLASSREPLICATIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



class CClassBodyASTNode;
class CIdentifierDataType;
class CConstDictionaryASTNode;
class CClassMemberASTNode;
class CClassPropertyASTNode;
class CObjectDataType;
class CClassReferenceDataType;
class CSymbol;

class CClassReplicationVariable
{
public:
	String					Identifier;
	CExpressionBaseASTNode*		OwnerExpression;
	CExpressionBaseASTNode*		ModeExpression;
	CExpressionBaseASTNode*		RangeMinExpression;
	CExpressionBaseASTNode*		RangeMaxExpression;
	//CConstDictionaryASTNode*	Configuration;	
	CClassMemberASTNode*		Resolved;
	CClassPropertyASTNode*		ResolvedProperty;
};

// =================================================================
//	Stores information on a class declaration.
// =================================================================
class CClassReplicationASTNode : public CASTNode
{
	

protected:	
	bool								m_semanting;

public:

	// Parsing infered.
	CExpressionBaseASTNode*					PriorityExpression;
	std::vector<CClassReplicationVariable>	Variables;

	// General management.
	virtual String ToString();

	// Initialization.
	CClassReplicationASTNode(CASTNode* parent, CToken token);
	~CClassReplicationASTNode();
	
	// Semantic analysis.
	virtual CASTNode*				Semant					(CSemanter* semanter);
	virtual CASTNode*				Finalize				(CSemanter* semanter);	
	virtual CASTNode*				Clone					(CSemanter* semanter);

	virtual void					Translate				(CTranslator* translator);

};

#endif