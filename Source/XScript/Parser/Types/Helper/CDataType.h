/* *****************************************************************

		CDataType.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CDATATYPE_H_
#define _CDATATYPE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"



class CASTNode;
class CSemanter;
class CTranslationUnit;
class CArrayDataType;
class CClassASTNode;

// =================================================================
//	Base class for all data types.
// =================================================================
class CDataType
{
	

protected:
	CArrayDataType*		m_array_of_datatype;

public:
	CToken Token;

	CDataType(CToken& token);

	virtual CClassASTNode*	GetClass	(CSemanter* semanter);
	virtual CClassASTNode*	GetBoxClass	(CSemanter* semanter);
	virtual bool			IsEqualTo	(CSemanter* semanter, CDataType* type);
	virtual bool			CanCastTo	(CSemanter* semanter, CDataType* type);
	virtual String		ToString	();
	virtual CArrayDataType*	ArrayOf		();
	virtual CDataType*		Semant		(CSemanter* semanter, CASTNode* node);

};

#endif