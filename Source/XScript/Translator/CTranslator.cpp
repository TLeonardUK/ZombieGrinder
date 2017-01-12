/* *****************************************************************

		CTranslator.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Translator/CTranslator.h"

#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CPackageASTNode.h"

#include "XScript/Semanter/CSemanter.h"

#include "XScript/Compiler/CTranslationUnit.h"

// =================================================================
//	Processes input and performs the actions requested.
// =================================================================
bool CTranslator::Process(CTranslationUnit* context)
{	
	m_context = context;
	m_semanter = context->GetSemanter();

	CPackageASTNode* package = dynamic_cast<CPackageASTNode*>(m_context->GetASTRoot());
	TranslatePackage(package);

	return true;
}

// =================================================================
//	Returns the context being translated.
// =================================================================
CTranslationUnit* CTranslator::GetContext()
{
	return m_context;
}
