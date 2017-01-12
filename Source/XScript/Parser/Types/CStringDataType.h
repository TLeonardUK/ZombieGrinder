/* *****************************************************************

		CStringDataType.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CSTRINGDATATYPE_H_
#define _CSTRINGDATATYPE_H_

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
class CStringDataType : public CDataType
{
	

protected:

public:
	CStringDataType(CToken& token);
	
	virtual CClassASTNode*	GetClass	(CSemanter* semanter);
	virtual bool			IsEqualTo	(CSemanter* semanter, CDataType* type);
	virtual bool			CanCastTo	(CSemanter* semanter, CDataType* type);
	virtual String		ToString	();

};

#endif