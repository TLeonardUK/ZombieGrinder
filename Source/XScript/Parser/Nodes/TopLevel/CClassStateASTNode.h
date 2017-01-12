/* *****************************************************************

		CClassStateASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCLASSSTATEASTNODE_H_
#define _CCLASSSTATEASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



class CClassBodyASTNode;
class CIdentifierDataType;
class CObjectDataType;
class CClassReferenceDataType;
class CSymbol;

// =================================================================
//	Stores information on a class declaration.
// =================================================================
class CClassStateASTNode : public CDeclarationASTNode
{
	

protected:	
	bool								m_semanting;

public:

	// Parsing infered.
	bool								IsDefault;
	std::vector<CClassMemberASTNode*>	Events;

	// General management.
	virtual String ToString();

	// Initialization.
	CClassStateASTNode(CASTNode* parent, CToken token);
	~CClassStateASTNode();
	
	// Semantic analysis.
	virtual CASTNode*				Semant					(CSemanter* semanter);
	virtual CASTNode*				Finalize				(CSemanter* semanter);	
	virtual CASTNode*				Clone					(CSemanter* semanter);

	virtual void					Translate				(CTranslator* translator);

};

#endif