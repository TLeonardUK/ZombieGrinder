/* *****************************************************************

		CClassReferenceDataType.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassReferenceDataType::CClassReferenceDataType(CToken& token, CClassASTNode* node) :
	CDataType(token),
	m_class(node)
{
}

// =================================================================
//	Checks if this data type is equal to another.
// =================================================================
bool CClassReferenceDataType::IsEqualTo(CSemanter* semanter, CDataType* type)
{
	CClassReferenceDataType* classRef = dynamic_cast<CClassReferenceDataType*>(type) ;
	return (classRef != NULL && classRef->m_class == m_class);
}

// =================================================================
//	Checks if this data type extends another.
// =================================================================
bool CClassReferenceDataType::CanCastTo(CSemanter* semanter, CDataType* type)
{
	return IsEqualTo(semanter, type);
}

// =================================================================
//	Converts data type to string.
// =================================================================
String	CClassReferenceDataType::ToString()
{
	return m_class->ToString();
}

// =================================================================
//	Gets the class this data type is based on.
// =================================================================
CClassASTNode* CClassReferenceDataType::GetClass(CSemanter* semanter)
{
	return m_class;
}
