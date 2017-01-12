/* *****************************************************************

		CClassPropertyASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCLASSPROPERTYASTNODE_H_
#define _CCLASSPROPERTYASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

class CClassBodyASTNode;
class CExpressionASTNode;
class CDataType;
class CMethodBodyASTNode;
class CVariableStatementASTNode;
class CClassASTNode;

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



// =================================================================
//	Stores information on a class member declaration.
// =================================================================
class CClassPropertyASTNode : public CDeclarationASTNode
{
	

protected:	

public:
	bool										IsSerialized;
	int											SerializeVersion;
	CDataType*									ReturnType;		

	CClassMemberASTNode*						Set_Method;
	CClassMemberASTNode*						Get_Method;

	// Constructors.
	CClassPropertyASTNode	(CASTNode* parent, CToken token);
	~CClassPropertyASTNode();
	
	// General management.
	virtual String ToString();

	// Semantic analysis.
	virtual CASTNode* Semant				(CSemanter* semanter);
	virtual CASTNode* Finalize				(CSemanter* semanter);	
	virtual CASTNode* Clone					(CSemanter* semanter);

	virtual void	Translate				(CTranslator* translator);

};

#endif