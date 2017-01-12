/* *****************************************************************

		CBoolDataType.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CBOOLDATATYPE_H_
#define _CBOOLDATATYPE_H_

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
class CBoolDataType : public CDataType
{
	

protected:

public:
	static CBoolDataType* StaticInstance;

	CBoolDataType(CToken token);
	
	virtual CClassASTNode*	GetClass	(CSemanter* semanter);
	virtual bool			IsEqualTo	(CSemanter* semanter, CDataType* type);
	virtual bool			CanCastTo	(CSemanter* semanter, CDataType* type);
	virtual String		ToString	();

};

#endif