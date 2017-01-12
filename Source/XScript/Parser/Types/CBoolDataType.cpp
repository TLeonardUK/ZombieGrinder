/* *****************************************************************

		CBoolDataType.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Semanter/CSemanter.h"

#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

CBoolDataType* CBoolDataType::StaticInstance = new CBoolDataType(CToken());

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CBoolDataType::CBoolDataType(CToken token) :
	CDataType(token)
{
}

// =================================================================
//	Checks if this data type extends another.
// =================================================================
bool CBoolDataType::CanCastTo(CSemanter* semanter, CDataType* type)
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
			dynamic_cast<CIntDataType*>(type) != NULL ||
			dynamic_cast<CStringDataType*>(type) != NULL)
		{
			return true;
		}
	}

	return false;
}

// =================================================================
//	Checks if this data type is equal to another.
// =================================================================
bool CBoolDataType::IsEqualTo(CSemanter* semanter, CDataType* type)
{
	return dynamic_cast<CBoolDataType*>(type) != NULL;
}

// =================================================================
//	Converts data type to string.
// =================================================================
String	CBoolDataType::ToString()
{
	return "bool";
}

// =================================================================
//	Gets the class this data type is based on.
// =================================================================
CClassASTNode* CBoolDataType::GetClass(CSemanter* semanter)
{
	CDeclarationIdentifier s_bool_ident("bool");

	return dynamic_cast<CClassASTNode*>(semanter->GetContext()->GetASTRoot()->FindDeclaration(semanter, s_bool_ident)->Semant(semanter));
}
