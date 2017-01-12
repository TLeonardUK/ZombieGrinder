/* *****************************************************************

		CDataType.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/Helper/CIdentifierDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include <algorithm>

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CDataType::CDataType(CToken& token) :
	m_array_of_datatype(NULL),
	Token(token)
{
}

// =================================================================
//	Gets the class this data type is based on.
// =================================================================
CClassASTNode* CDataType::GetClass(CSemanter* semanter)
{
	return NULL;
}

// =================================================================
//	Gets the class that this data type can be boxed into.
// =================================================================
CClassASTNode* CDataType::GetBoxClass(CSemanter* semanter)
{	
	CClassASTNode* node = GetClass(semanter);
	if (node->HasBoxClass == true)
	{
		return dynamic_cast<CClassASTNode*>(semanter->GetContext()->GetASTRoot()->FindDeclaration(semanter, node->BoxClassIdentifier)->Semant(semanter));
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Checks if this data type is equal to another.
// =================================================================
bool CDataType::IsEqualTo(CSemanter* semanter, CDataType* type)
{
	return false;
}

// =================================================================
//	Checks if this data type extends another.
// =================================================================
bool CDataType::CanCastTo(CSemanter* semanter, CDataType* type)
{
	return IsEqualTo(semanter, type);
}

// =================================================================
//	Converts data type to string.
// =================================================================
String	CDataType::ToString()
{
	return "Unknown DataType";
}

// =================================================================
//	Returns data type thats an array of this data type.
// =================================================================
CArrayDataType* CDataType::ArrayOf()
{
	if (m_array_of_datatype == NULL)
	{
		m_array_of_datatype = new CArrayDataType(Token, this);
	}
	return m_array_of_datatype;
}

// =================================================================
//	Performs semantic analysis of this data type.
// =================================================================
CDataType* CDataType::Semant(CSemanter* semanter, CASTNode* node)
{
	SEMANT_TRACE("CDataType");

	return this;
}

