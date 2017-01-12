/* *****************************************************************

		CCompiler.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCOMPILER_H_
#define _CCOMPILER_H_

#include "Generic/Types/String.h"

#include <map>
#include <vector>

class CTranslator;
class CBuilder;
class CTranslationUnit;

// =================================================================
//	Type of definition.
// =================================================================
namespace DefineType
{
	enum Type
	{
		String,
		Int,
		Float,
		Bool,
	};
};

// =================================================================
//	Used to define a preprocessor value.
// =================================================================
struct CDefine
{
public:
	DefineType::Type	Type;
	String				Name;
	String				Value;

	CDefine()
	{
	}
	CDefine(DefineType::Type type, String name, String value)
	{
		Type = type;
		Name = name;
		Value = value;
	}

};

// =================================================================
//	Class deals with process input and compiling the correct
//	files as requested.
// =================================================================
class CCompiler
{
private:
	String							m_packageDirectory;
	String							m_baseDirectory;

	String							m_fileExtension;

	CTranslator*						m_translator;

	std::vector<CDefine>				m_defines;

public:
	CCompiler();
	~CCompiler();

	String GetPackageDirectory		();	
	String GetFileExtension		();

	CTranslator* GetTranslator();

	CTranslationUnit* Compile			(String source, String package_path, std::vector<CDefine> defines);

};

#endif