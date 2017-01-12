/* *****************************************************************

		CDeclarationASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CDECLARATIONASTNODE_H_
#define _CDECLARATIONASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"

// =================================================================
//	Base class for all declarations.
// =================================================================
class CDeclarationASTNode : public CASTNode 
{
	

protected:	

public:
	CDeclarationIdentifier		Identifier;

	String						MangledIdentifier;
	bool						IsNative;

	CDeclarationASTNode(CASTNode* parent, CToken token);
	virtual ~CDeclarationASTNode() {}

	// Semanting.
	virtual void CheckAccess(CSemanter* semanter, CASTNode* referenceBy);

};

#endif