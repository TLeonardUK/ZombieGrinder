/* *****************************************************************

		CCompiler.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "Generic/Types/String.h"
#include <map>
#include <stdio.h>
#include <assert.h>

#include "XScript/Compiler/CCompiler.h"
#include "XScript/Lexer/CLexer.h"
#include "XScript/Parser/CParser.h"
#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Helpers/CPathHelper.h"
#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Translator/VMBinary/CVMBinaryTranslator.h"

#include "XScript/Parser/Types/Helper/CDataType.h"



#ifdef _WIN32
#include <Windows.h>
#endif

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CCompiler::CCompiler()
{
	// Some general settings.
	m_fileExtension	= "xs";

	m_translator = new CVMBinaryTranslator();
}

// =================================================================
//	Deconstructs a new instance of this class.
// =================================================================
CCompiler::~CCompiler()
{
	SAFE_DELETE(m_translator);
}

// =================================================================
//	Gets the translator used to convert the AST tree.
// =================================================================
CTranslator* CCompiler::GetTranslator()
{
	return m_translator;
}

// =================================================================
//	Compiles the given package file.
// =================================================================
CTranslationUnit* CCompiler::Compile(String path, String package_path, std::vector<CDefine> defines)
{	
	m_packageDirectory = package_path;

	CTranslationUnit* context = new CTranslationUnit(this, path.c_str(), defines);
	context->Compile();
	return context;
}

// =================================================================
//	Gets the directory that packages are stored in.
// =================================================================
String CCompiler::GetPackageDirectory()
{
	return m_packageDirectory;
}

// =================================================================
//	Gets the file extension the compiler uses.
// =================================================================
String CCompiler::GetFileExtension()
{
	return m_fileExtension;
}