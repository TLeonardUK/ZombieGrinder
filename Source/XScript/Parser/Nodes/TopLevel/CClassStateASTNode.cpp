/* *****************************************************************

		CClassStateASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassStateASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassStateASTNode::CClassStateASTNode(CASTNode* parent, CToken token) :
	CDeclarationASTNode(parent, token)
{
	Identifier				= "";
	IsDefault				= false;
	m_semanting				= false;
}

// =================================================================
//	Destructor.
// =================================================================
CClassStateASTNode::~CClassStateASTNode()
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassStateASTNode::Clone(CSemanter* semanter)
{
	CClassStateASTNode* clone = new CClassStateASTNode(NULL, Token);
	
	clone->IsDefault		 = this->IsDefault;
	clone->MangledIdentifier = this->MangledIdentifier;
	clone->Identifier		 = this->Identifier;
	
	return clone;
}

// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CClassStateASTNode::ToString()
{
	return Identifier.c_str();
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassStateASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CClassStateASTNode=%s", Identifier.c_str());

	// Only semant once.
	if (Semanted == true)
	{
		//m_semanting = false;
		return this;
	}
	Semanted = true;

	// Check for duplicate state identifier.
	CClassASTNode* class_scope = FindClassScope(semanter);
	CClassStateASTNode* class_state = class_scope->FindClassState(semanter, Identifier, this, this);
	
	if (class_state != NULL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Duplicate class state encountered '%s'.", Identifier.c_str()), Token);
	}

	// Check for duplicate default state.
	if (IsDefault == true)
	{
		if (class_scope->DefaultState != NULL)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Duplicate default class state encountered '%s' and '%s'.", Identifier.c_str(), class_scope->DefaultState->Identifier.c_str()), Token);
		}

		class_scope->DefaultState = this;
		class_scope->States.push_back(this);
	}

	// Semant all members.
	SemantChildren(semanter);

	return this;
}

// =================================================================
//	Performs finalization on this class.
// =================================================================
CASTNode* CClassStateASTNode::Finalize(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CClassStateASTNode::Translate(CTranslator* translator)
{
	translator->TranslateClassState(this);
}