/* *****************************************************************

		CObjectDataType.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _COBJECTDATATYPE_H_
#define _COBJECTDATATYPE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Types/Helper/CDataType.h"



class CSemanter;
class CTranslationUnit;
class CArrayDataType;
class CDataType;

// =================================================================
//	Base class for all data types.
// =================================================================
class CObjectDataType : public CDataType
{
	

protected:
	CClassASTNode* m_class;

public:

	CObjectDataType(CToken& token, CClassASTNode* classNode);
	
	virtual CClassASTNode*	GetClass	(CSemanter* semanter);
	virtual bool			IsEqualTo	(CSemanter* semanter, CDataType* type);
	virtual bool			CanCastTo	(CSemanter* semanter, CDataType* type);
	virtual String		ToString	();

	virtual CDataType*		Semant			(CSemanter* semanter, CASTNode* node);
	virtual CClassASTNode*	SemantAsClass	(CSemanter* semanter, CASTNode* node);

};

#endif