/* *****************************************************************

		CClassPropertyASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Values/CConstDictionaryASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassPropertyASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"
#include "XScript/Parser/Types/CVoidDataType.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassPropertyASTNode::CClassPropertyASTNode(CASTNode* parent, CToken token) 
	: CDeclarationASTNode(parent, token)
	, IsSerialized(false)
	, SerializeVersion(0)
	, Set_Method(NULL)
	, Get_Method(NULL)
{
}

// =================================================================
//	Destructor.
// =================================================================
CClassPropertyASTNode::~CClassPropertyASTNode()
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassPropertyASTNode::Clone(CSemanter* semanter)
{
	CClassPropertyASTNode* clone = new CClassPropertyASTNode(NULL, Token);
	
	clone->IsSerialized = IsSerialized;
	clone->SerializeVersion = SerializeVersion;
	clone->ReturnType = ReturnType;

	return clone;
}

// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CClassPropertyASTNode::ToString()
{
	return "Class Property";
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassPropertyASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CClassPropertyASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		//m_semanting = false;
		return this;
	}
	Semanted = true;
	
	// Semant return type.
	ReturnType = ReturnType->Semant(semanter, this);

	// Get the set/get methods again (incase we have been cloned since parsing!)
	std::vector<CDataType*> args;
	args.push_back(ReturnType);

	CDeclarationIdentifier set_ident = CDeclarationIdentifier(StringHelper::Format("Set_%s", Identifier.c_str()).c_str());
	CDeclarationIdentifier get_ident = CDeclarationIdentifier(StringHelper::Format("Get_%s", Identifier.c_str()).c_str());

	Set_Method = Parent->FindClassMethod(semanter, set_ident, args, true);
	Get_Method = Parent->FindClassMethod(semanter, get_ident, std::vector<CDataType*>(), true);

	// Check all fields have correct return type and are either Set/Get signatures.
	if (Set_Method != NULL)
	{
		if (!Set_Method->ReturnType->IsEqualTo(semanter, CVoidDataType::StaticInstance))
		{
			semanter->GetContext()->FatalError(StringHelper::Format("Set method in property '%s' does not return correct data type.", Identifier.c_str()), Token);
		}
	}
	if (Get_Method != NULL)
	{
		if (!Get_Method->ReturnType->IsEqualTo(semanter, ReturnType))
		{
			semanter->GetContext()->FatalError(StringHelper::Format("Get method in property '%s' does not return correct data type.", Identifier.c_str()), Token);
		}
	}

	return this;
}

// =================================================================
//	Performs finalization on this class.
// =================================================================
CASTNode* CClassPropertyASTNode::Finalize(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CClassPropertyASTNode::Translate(CTranslator* translator)
{
	// No translation is performed for this, as it just holds values for the actual class.
}