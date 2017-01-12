/* *****************************************************************

		CFloatDataType.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Parser/Types/Helper/CNumericDataType.h"

#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CFloatDataType::CFloatDataType(CToken& token) :
	CNumericDataType(token)
{
}

// =================================================================
//	Checks if this data type is equal to another.
// =================================================================
bool CFloatDataType::IsEqualTo(CSemanter* semanter, CDataType* type)
{
	return dynamic_cast<CFloatDataType*>(type) != NULL;
}

// =================================================================
//	Checks if this data type extends another.
// =================================================================
bool CFloatDataType::CanCastTo(CSemanter* semanter, CDataType* type)
{
	CObjectDataType* obj = dynamic_cast<CObjectDataType*>(type);

	CDeclarationIdentifier s_object_declaration("object");
	CDeclarationIdentifier s_value_ident("Value");

	if (obj != NULL)
	{
		// Can be upcast to anything that its boxed class allows.
		if (type->GetClass(semanter)->Identifier == s_object_declaration &&
			GetBoxClass(semanter) != NULL)
		{
			// Look to see if our box-class contains an argument that accepts us.
			CClassASTNode*			node	= GetBoxClass(semanter);
			CClassMemberASTNode*	field 	= node == NULL ? NULL : node->FindClassField(semanter, s_value_ident, NULL, NULL); 
			
			return field != NULL && field->ReturnType->IsEqualTo(semanter, this);
		}
	}
	else
	{
		if (IsEqualTo(semanter, type) ||
			dynamic_cast<CNumericDataType*>(type) != NULL ||
			dynamic_cast<CStringDataType*>(type) != NULL)
		{
			return true;
		}
	}

	return false;
}

// =================================================================
//	Converts data type to string.
// =================================================================
String	CFloatDataType::ToString()
{
	return "float";
}

// =================================================================
//	Gets the class this data type is based on.
// =================================================================
CClassASTNode* CFloatDataType::GetClass(CSemanter* semanter)
{
	CDeclarationIdentifier s_float_ident("float");

	return dynamic_cast<CClassASTNode*>(semanter->GetContext()->GetASTRoot()->FindDeclaration(semanter, s_float_ident)->Semant(semanter));
}
