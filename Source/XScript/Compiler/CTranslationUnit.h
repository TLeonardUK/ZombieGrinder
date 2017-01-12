/* *****************************************************************

		CTranslationUnit.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CTRANSLATIONUNIT_H_
#define _CTRANSLATIONUNIT_H_

#include <vector>
#include "Generic/Types/String.h"

#include "XScript/Compiler/CCompiler.h"
#include "XScript/Lexer/CLexer.h"
#include "XScript/Parser/CParser.h"
#include "XScript/Semanter/CSemanter.h"
#include "XScript/Preprocessor/CPreprocessor.h"

class CCompiler;

// =================================================================
//	This class stores an error that occurred during compilation.
// =================================================================
class CCompileError
{
public:
	enum Type
	{
		FatalError,
		Error,
		Warning,
		Info,
	};

private:
	String m_message;
	String m_file;
	int m_line;
	int m_column;
	Type m_type;
	String m_line_text;

public:
	CCompileError(Type type, String line_text, String msg, CToken position);
	CCompileError(Type type, String line_text, String msg, String file, int line, int column);

	bool IsFatal();
	String ToString();

};

// =================================================================
//	This class stores non-transient data that needs to be passed
//  around during the compilation process.
// =================================================================
class CTranslationUnit
{
private:
	CCompiler*					m_compiler;
	String					m_file_path;
	String					m_filename;
	String					m_file_path_cleaned;
	String					m_file_path_cleaned_abs;
	
	String					m_source;

	CLexer						m_lexer;
	CParser						m_parser;
	CSemanter					m_semanter;
	CPreprocessor				m_preprocessor;

	std::vector<CToken>			m_token_list;
	std::vector<String>	m_using_files;
	std::vector<String>	m_using_files_lower;
	std::vector<String>	m_native_files;
	std::vector<String>	m_copy_files;
	std::vector<String>	m_library_files;
	std::vector<CCompileError>	m_errors;

	int							m_last_line_row;
	int							m_last_line_column;

	std::vector<CTranslationUnit*>	m_imported_units;

	std::vector<CDefine>			m_defines;

	std::vector<CDataType*>			m_identifier_data_types;

	bool m_imported_package;
	CTranslationUnit* m_importing_package;
	
	void Collect_Unit_Nodes(CTranslationUnit* unit, std::vector<CASTNode*>& result);
	void Collect_Nodes(CASTNode* node, std::vector<CASTNode*>& result);

public:
	static void Print_Compile_Stats();

	~CTranslationUnit();
	CTranslationUnit(CCompiler* compiler, String file_path, std::vector<CDefine> defines);

	bool Evaluate	(String expr);
	bool Compile	(bool importedPackage = false, CTranslationUnit* importingPackage = NULL);
	bool PreProcess	();

	CCompiler*					GetCompiler			();
	CASTNode*					GetASTRoot			();
	String&				GetSource			();
	String&				GetFilePath			();
	std::vector<CToken>&		GetTokenList		();
	std::vector<String>&	GetUsingFileList	();
	std::vector<String>&	GetTranslatedFiles	();
	std::vector<CDefine>&		GetDefines			();
	std::vector<CCompileError>&	GetErrors			();
	CSemanter*					GetSemanter			();

	int							GetLastLineRow		();
	int							GetLastLineColumn	();

	int							GetTicks			();
	bool						Execute				(String path, String cmd_line);

	bool						AddUsingFile		(String file);

	String GetLineText(String file, int line);

	void FatalError	(String msg, String source="internal", int row=1, int column=1);
	void FatalError	(String msg, CToken& token);
	void Error		(String msg, String source="internal", int row=1, int column=1);
	void Error		(String msg, CToken& token);
	void Warning	(String msg, String source="internal", int row=1, int column=1);
	void Warning	(String msg, CToken& token);
	void Info		(String msg, String source="internal", int row=1, int column=1);
	void Info		(String msg, CToken& token);

};

#endif