/* *****************************************************************

		CDeclarationASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"

#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CDeclarationASTNode::CDeclarationASTNode(CASTNode* parent, CToken token) :
	CASTNode(parent, token)
{
	Identifier			= "";
	MangledIdentifier	= "";
	IsNative			= false;
}

// =================================================================
//	Checks if we can access this declaration from the given node.
// =================================================================
void CDeclarationASTNode::CheckAccess(CSemanter* semanter, CASTNode* referenceBy)
{
	semanter->GetContext()->FatalError("Internal error. Invalid access validation.", Token);
}

