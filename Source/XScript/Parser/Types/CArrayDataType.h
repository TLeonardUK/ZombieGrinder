/* *****************************************************************

		CArrayDataType.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CARRAYDATATYPE_H_
#define _CARRAYDATATYPE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Types/CObjectDataType.h"



class CSemanter;
class CTranslationUnit;
class CArrayDataType;
class CDataType;

// =================================================================
//	Base class for all data types.
// =================================================================
class CArrayDataType : public CObjectDataType
{
	

protected:

public:
	CDataType* ElementType;

	CArrayDataType(CToken& token, CDataType* type);
	
	virtual CClassASTNode*	GetClass	(CSemanter* semanter);
	virtual CClassASTNode*	GetBoxClass	(CSemanter* semanter);
	virtual bool			IsEqualTo	(CSemanter* semanter, CDataType* type);
	virtual bool			CanCastTo	(CSemanter* semanter, CDataType* type);
	virtual String		ToString	();
	virtual CArrayDataType*	ArrayOf		();
	virtual CDataType*		Semant		(CSemanter* semanter, CASTNode* node);

};

#endif