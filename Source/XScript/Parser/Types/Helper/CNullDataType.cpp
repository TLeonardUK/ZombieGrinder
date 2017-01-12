/* *****************************************************************

		CNullDataType.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Types/Helper/CNullDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CNullDataType::CNullDataType(CToken& token) :
	CDataType(token)
{
}

// =================================================================
//	Checks if this data type is equal to another.
// =================================================================
bool CNullDataType::IsEqualTo(CSemanter* semanter, CDataType* type)
{
	return dynamic_cast<CNullDataType*>(type)	!= NULL ||
		   (dynamic_cast<CObjectDataType*>(type) != NULL && dynamic_cast<CObjectDataType*>(type)->GetClass(semanter)->IsStruct == false) ||
		   dynamic_cast<CArrayDataType*>(type) != NULL;
}

// =================================================================
//	Checks if this data type extends another.
// =================================================================
bool CNullDataType::CanCastTo(CSemanter* semanter, CDataType* type)
{
	return IsEqualTo(semanter, type);
}

// =================================================================
//	Converts data type to string.
// =================================================================
String	CNullDataType::ToString()
{
	return "null";
}
