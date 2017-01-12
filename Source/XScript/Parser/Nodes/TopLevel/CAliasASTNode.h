/* *****************************************************************

		CAliasASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CALIASASTNODE_H_
#define _CALIASASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



class CClassBodyASTNode;
class CIdentifierDataType;
class CObjectDataType;
class CDataType;
class CASTNode;

// =================================================================
//	Stores information on a alias declaration.
// =================================================================
class CAliasASTNode : public CDeclarationASTNode
{
	

protected:	

public:
	CDeclarationASTNode* AliasedDeclaration;
	CDataType*			 AliasedDataType;

	// General management.
	virtual String ToString();

	CAliasASTNode(CASTNode* parent, CToken token);
	CAliasASTNode(CASTNode* parent, CToken token, String identifier, CDeclarationASTNode* decl);
	CAliasASTNode(CASTNode* parent, CToken token, String identifier, CDataType* decl);
	
	// Semantic analysis.
	virtual CASTNode* Semant				(CSemanter* semanter);	
	virtual CASTNode* Clone					(CSemanter* semanter);

	virtual void	  Translate				(CTranslator* translator);
};

#endif