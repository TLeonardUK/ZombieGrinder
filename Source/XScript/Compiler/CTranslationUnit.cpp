/* *****************************************************************

		CTranslationUnit.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Compiler/CCompiler.h"

#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Helpers/CPathHelper.h"

#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"

#include "XScript/Parser/Types/Helper/CIdentifierDataType.h"

#include "XScript/Translator/CTranslator.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/Stats/Stats.h"

#include <stdexcept>
#include <assert.h>
#include <algorithm>

#ifdef _WIN32

#include <windows.h>

#elif defined(__linux__) || defined(__APPLE__)

#include <sys/time.h>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#endif
	
DEFINE_FRAME_STATISTIC("Script Compiler/Load Time",					float,					g_script_load_time,			false);
DEFINE_FRAME_STATISTIC("Script Compiler/Preprocess Time",			float,					g_script_preprocess_time,	false);
DEFINE_FRAME_STATISTIC("Script Compiler/Lexer Time",				float,					g_script_lexer_time,		false);
DEFINE_FRAME_STATISTIC("Script Compiler/Parser Time",				float,					g_script_parser_time,		false);
DEFINE_FRAME_STATISTIC("Script Compiler/Semanter Time",				float,					g_script_semanter_time,		false);
DEFINE_FRAME_STATISTIC("Script Compiler/Translator Time",			float,					g_script_translator_time,	false);

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CCompileError::CCompileError(Type type, String line_text, String msg, CToken position)
	: m_type(type)
	, m_message(msg)
	, m_file(position.SourceFile)
	, m_line(position.Row)
	, m_column(position.Column)
	, m_line_text(line_text)
{
}

CCompileError::CCompileError(Type type, String line_text, String msg, String file, int line, int column)
	: m_type(type)
	, m_message(msg)
	, m_file(file)
	, m_line(line)
	, m_column(column)
	, m_line_text(line_text)
{
}

bool CCompileError::IsFatal()
{
	return (m_type == FatalError);
}

String CCompileError::ToString()
{
	String output = "";

	String arrow_line = CStringHelper::PadLeft("^", m_column - 1);

	static const char* type_name[] = {
		"error",
		"error",
		"warning",
		"info"
	};

	String path = CPathHelper::CleanPath(CPathHelper::GetAbsolutePath(m_file));

#ifdef PLATFORM_WIN32
	// If we don't do this then VS picks up the / & \ files as different files, and we end
	// up with duplicates open :(
	path = path.Replace("/", "\\");
#endif

	output += StringHelper::Format("%s(%i,%i): %s: %s", path.c_str(), m_line, m_column, type_name[m_type], m_message.c_str());
	if (m_type != Info)
	{
		output += StringHelper::Format("\n%s\n", m_line_text.c_str());
		output += StringHelper::Format("%s", arrow_line.c_str());
	}

	return output;
}

// Dumps out some general compile statistics.
void CTranslationUnit::Print_Compile_Stats()
{
	DBG_LOG("[Compile Statistics]");
	DBG_LOG("  Load Time: %s", g_script_load_time.To_String().c_str());
	DBG_LOG("  Preprocess Time: %s", g_script_preprocess_time.To_String().c_str());
	DBG_LOG("  Lexer Time: %s", g_script_lexer_time.To_String().c_str());
	DBG_LOG("  Parser Time: %s", g_script_parser_time.To_String().c_str());
	DBG_LOG("  Semanter Time: %s", g_script_semanter_time.To_String().c_str());
	DBG_LOG("  Translator Time: %s", g_script_translator_time.To_String().c_str());
}

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CTranslationUnit::CTranslationUnit(CCompiler* compiler, String file_path, std::vector<CDefine> defines)
{
	m_imported_package = false;
	m_importing_package = NULL;
	m_compiler  = compiler;
	m_file_path = file_path;
	m_filename  = CPathHelper::StripDirectory(m_file_path);
	m_defines   = defines;
	m_file_path_cleaned = CStringHelper::ToLower(CPathHelper::CleanPath(m_file_path));
	m_file_path_cleaned_abs = CStringHelper::ToLower(CPathHelper::CleanPath(CPathHelper::GetAbsolutePath(m_file_path)));
}

// =================================================================
//	Destructs an instance of this class.
// =================================================================
CTranslationUnit::~CTranslationUnit()
{
	for (std::vector<CTranslationUnit*>::iterator iter = m_imported_units.begin(); iter != m_imported_units.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_imported_units.clear();

	for (std::vector<CDataType*>::iterator iter = m_identifier_data_types.begin(); iter != m_identifier_data_types.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_identifier_data_types.clear();

	m_token_list.clear();
	m_using_files.clear();
	m_using_files_lower.clear();
	m_native_files.clear();
	m_copy_files.clear();
	m_library_files.clear();
	m_errors.clear();
	m_defines.clear();

	if (m_imported_package == false)
	{
		std::vector<CASTNode*> all_nodes;
		Collect_Unit_Nodes(this, all_nodes);

		for (std::vector<CASTNode*>::iterator iter = all_nodes.begin(); iter != all_nodes.end(); iter++)
		{
			CASTNode* node = *iter;
			SAFE_DELETE(node);
		}
	}
}

// =================================================================
// =================================================================
void CTranslationUnit::Collect_Unit_Nodes(CTranslationUnit* unit, std::vector<CASTNode*>& result)
{
	Collect_Nodes(unit->m_parser.GetASTRoot(), result);

	for (std::vector<CTranslationUnit*>::iterator iter = m_imported_units.begin(); iter != m_imported_units.end(); iter++)
	{
		Collect_Unit_Nodes(*iter, result);
	}
}

// =================================================================
// =================================================================
void CTranslationUnit::Collect_Nodes(CASTNode* node, std::vector<CASTNode*>& result)
{
	if (node->Collected == false)//std::find(result.begin(), result.end(), node) == result.end())
	{
		result.push_back(node);
		node->Collected = true;
	}
	else
	{
		return;
	}

	if (node->Children.size() > 0)
	{
		for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
		{
			CASTNode* sub_node = *iter;
			Collect_Nodes(sub_node, result);
		}
	}

	CClassASTNode* class_node = dynamic_cast<CClassASTNode*>(node);
	if (class_node != NULL)
	{
		for (std::vector<CClassASTNode*>::iterator iter = class_node->GenericInstances.begin(); iter != class_node->GenericInstances.end(); iter++)
		{
			CClassASTNode* sub_node = *iter;
			Collect_Nodes(sub_node, result);
		}
	}
}

// =================================================================
//	Get the compiler being used to compile this translation unit.
// =================================================================
CCompiler* CTranslationUnit::GetCompiler() 
{ 
	return m_compiler;
}

// =================================================================
//	Returns the semanter used by this translation unit.
//	TODO: This really shouldn't be needed, its only here because
//		  calls to ->GetClass ->FindDeclaration etc require it and
//		  translators need to call them.
// =================================================================
CSemanter* CTranslationUnit::GetSemanter()
{
	return &m_semanter;
}

// =================================================================
//	Get the root node of the AST.
// =================================================================
CASTNode* CTranslationUnit::GetASTRoot() 
{ 
	return m_parser.GetASTRoot();
}

// =================================================================
//	Get source-code for this translation unit.
// =================================================================
String& CTranslationUnit::GetSource() 
{ 
	return m_source;
}

// =================================================================
//	Get file-path for this translation unit.
// =================================================================
String& CTranslationUnit::GetFilePath() 
{ 
	return m_file_path;
}

// =================================================================
//	Get file-path for this translation unit.
// =================================================================
std::vector<CToken>& CTranslationUnit::GetTokenList() 
{ 
	return m_token_list;
}

// =================================================================
//	Get a list of files that are imported using the using decl.
// =================================================================
std::vector<String>& CTranslationUnit::GetUsingFileList() 
{ 
	return m_using_files;
}

// =================================================================
//	Gets a list of defines.
// =================================================================
std::vector<CDefine>& CTranslationUnit::GetDefines()
{
	return m_defines;
}

// =================================================================
//	Gets a list of all compile errors.
// =================================================================
std::vector<CCompileError>&	CTranslationUnit::GetErrors()
{
	return m_errors;
}

// =================================================================
//	Returns the last row of the file.
// =================================================================
int CTranslationUnit::GetLastLineRow()
{
	CToken& token = m_token_list.at(m_token_list.size() - 1);
	return token.Row;
}

// =================================================================
//	Returns the last column on the last line of the file.
// =================================================================
int CTranslationUnit::GetLastLineColumn()
{
	CToken& token = m_token_list.at(m_token_list.size() - 1);
	return token.Column + token.Literal.size();
}

// =================================================================
//	Adds a new using file, returns true on success, false on
//  duplicate using file.
// =================================================================
bool CTranslationUnit::AddUsingFile(String file)
{
	String cleaned = CPathHelper::CleanPath(file);
	String cleaned_lower = CStringHelper::ToLower(cleaned);

	if (cleaned_lower == m_file_path_cleaned ||
		cleaned_lower == m_file_path_cleaned_abs)
	{
		return false;
	}

	for (std::vector<String>::iterator iter = m_using_files_lower.begin(); iter != m_using_files_lower.end(); iter++)
	{
		String& clean_iter = *iter;

		if (cleaned_lower == clean_iter)
		{
			return false;
		}
	}

//	DBG_LOG("New Using File: %s", cleaned.c_str());

	m_using_files.push_back(cleaned);
	m_using_files_lower.push_back(cleaned_lower);

	return true;
}

// =================================================================
//	Gets the line of text for the given file.
// =================================================================
String CTranslationUnit::GetLineText(String file, int row)
{
	String line = "";

	if (file == m_file_path)
	{
		line = CStringHelper::Replace(CStringHelper::GetLineInString(m_source, row - 1), "\t", "    ");
	}
	else
	{
		line = "(could not retrieve source code)";
		for (std::vector<CTranslationUnit*>::iterator iter = m_imported_units.begin(); iter != m_imported_units.end(); iter++)
		{
			CTranslationUnit* unit = *iter;
			if (unit->m_file_path == file)
			{
				line = CStringHelper::Replace(CStringHelper::GetLineInString(unit->m_source, row - 1), "\t", "    ");;
				break;
			}
		}
	}	

	return line;
}

// =================================================================
//	Emits a fatal error and aborts compilation of this translation
//  unit.
// =================================================================
void CTranslationUnit::FatalError(String msg, String source, int row, int column)
{
	if (m_imported_package == true)
	{
		String line = GetLineText(source, row);
		m_importing_package->m_errors.push_back(CCompileError(CCompileError::FatalError, line, msg, source, row, column));
		throw std::runtime_error("Fatal Error");
	}
	else
	{
		String line = GetLineText(source, row);
		m_errors.push_back(CCompileError(CCompileError::FatalError, line, msg, source, row, column));
  		throw std::runtime_error("Fatal Error");
	}
}

// =================================================================
//	Emits a fatal error and aborts compilation of this translation
//  unit.
// =================================================================
void CTranslationUnit::FatalError(String msg, CToken& token)
{
	FatalError(msg, token.SourceFile, token.Row, token.Column);
}

// =================================================================
//	Emits an error and continues.
// =================================================================
void CTranslationUnit::Error(String msg, String source, int row, int column)
{
	if (m_imported_package == true)
	{
		String line = GetLineText(source, row);
		m_importing_package->m_errors.push_back(CCompileError(CCompileError::Error, line, msg, source, row, column));
	}
	else
	{
		String line = GetLineText(source, row);
		m_errors.push_back(CCompileError(CCompileError::Error, line, msg, source, row, column));
	}
}

// =================================================================
//	Emits an error and continues.
// =================================================================
void CTranslationUnit::Error(String msg, CToken& token)
{
	Error(msg, token.SourceFile, token.Row, token.Column);
}

// =================================================================
//	Emits a warning and continues.
// =================================================================
void CTranslationUnit::Warning(String msg, String source, int row, int column)
{
	if (m_imported_package == true)
	{
		String line = GetLineText(source, row);
		m_importing_package->m_errors.push_back(CCompileError(CCompileError::Warning, line, msg, source, row, column));
	}
	else
	{
		String line = GetLineText(source, row);
		m_errors.push_back(CCompileError(CCompileError::Warning, line, msg, source, row, column));
	}
}

// =================================================================
//	Emits a warning message and continues.
// =================================================================
void CTranslationUnit::Warning(String msg, CToken& token)
{
	Warning(msg, token.SourceFile, token.Row, token.Column);
}

// =================================================================
//	Emits info message and continues.
// =================================================================
void CTranslationUnit::Info(String msg, String source, int row, int column)
{
	if (m_imported_package == true)
	{
		String line = GetLineText(source, row);
		m_importing_package->m_errors.push_back(CCompileError(CCompileError::Info, line, msg, source, row, column));
	}
	else
	{
		String line = GetLineText(source, row);
		m_errors.push_back(CCompileError(CCompileError::Info, line, msg, source, row, column));
	}
}

// =================================================================
//	Emits info message and continues.
// =================================================================
void CTranslationUnit::Info(String msg, CToken& token)
{
	Info(msg, token.SourceFile, token.Row, token.Column);
}

// =================================================================
//	Attempts to compile this translation unit. 
//	Returns true on success.
// =================================================================
bool CTranslationUnit::Evaluate(String expr)
{
	try
	{
		m_file_path = "<eval>";
		m_source = expr;
			
		// Tokenize.
		if (!m_lexer.Process(this))
		{
			return false;
		}

		// Replace all identifier tokens with defines.
		for (std::vector<CToken>::iterator iter = m_token_list.begin(); iter != m_token_list.end(); iter++)
		{
			CToken& token = *iter;
			if (token.Type == TokenIdentifier::IDENTIFIER)
			{
				for (std::vector<CDefine>::iterator defIter = m_defines.begin(); defIter != m_defines.end(); defIter++)
				{
					CDefine& define = *defIter;
					if (define.Name == token.Literal)
					{
						switch (define.Type)
						{
							case DefineType::Bool:
							{
								if (CStringHelper::ToLower(define.Value) == "false" ||
									define.Value == "0" ||
									define.Value == "")
								{									
									token.Type = TokenIdentifier::KEYWORD_FALSE;
									token.Literal = "0";
								}
								else
								{
									token.Type = TokenIdentifier::KEYWORD_TRUE;
									token.Literal = "1";
								}
								break;
							}
							case DefineType::Int:
							{
								token.Type = TokenIdentifier::INT_LITERAL;
								token.Literal = define.Value;
								break;
							}
							case DefineType::Float:
							{
								token.Type = TokenIdentifier::FLOAT_LITERAL;
								token.Literal = define.Value;
								break;
							}
							case DefineType::String:
							{
								token.Type = TokenIdentifier::STRING_LITERAL;
								token.Literal = define.Value;
								break;
							}
						}
					}
				}
			}
		}

		// Parse.
		if (!m_parser.Evaluate(this))
		{
			return false;
		}

		// Semant.
		if (!m_semanter.Process(this))
		{
			return false;
		}

		return true;
	}
	catch (std::runtime_error ex)
	{
		if (m_imported_package == true)
		{
			throw std::runtime_error("Fatal Error");
		}
		return false;
	}
}

// =================================================================
//	Attempts to compile this translation unit. 
//	Returns true on success.
// =================================================================
bool CTranslationUnit::Compile(bool importedPackage, CTranslationUnit* importingPackage)
{
	m_imported_package = importedPackage;
	m_importing_package = importingPackage;

	try
	{
		int start_tick_count = GetTicks();

		if (importedPackage == false)
		{
		//	Info("Generating Package: " + m_filename);
		}
		else
		{
		//	Info("Importing Package: " + m_filename);
		}

		// Read source from disk.
		//DBG_LOG("Loading Source ...");
		double time = Platform::Get()->Get_Ticks();
		if (!CPathHelper::LoadFile(m_file_path, m_source))
		{
			FatalError("Could not read file: " + m_file_path);
		}
		g_script_load_time.Set((float)(Platform::Get()->Get_Ticks() - time));
			
		// Preprocess the source file.
		time = Platform::Get()->Get_Ticks();
		m_preprocessor.Process(this);
		g_script_preprocess_time.Set((float)(Platform::Get()->Get_Ticks() - time));

		// Convert source file into a stream of tokens.
		//DBG_LOG("Lexical Analysis ...");
		time = Platform::Get()->Get_Ticks();
		m_lexer.Process(this);
		g_script_lexer_time.Set((float)(Platform::Get()->Get_Ticks() - time));
		
		// Convert the token stream info an AST representation.
		//DBG_LOG("Parsing ...");
		time = Platform::Get()->Get_Ticks();
		m_parser.Process(this);
		g_script_parser_time.Set((float)(Platform::Get()->Get_Ticks() - time));

		// Import support files?
		if (importedPackage == false)
		{
			String					support_dir = m_compiler->GetPackageDirectory() + "/runtime/compiler";
			std::vector<String>	files		= CPathHelper::ListFiles(support_dir);

			for (std::vector<String>::iterator iter = files.begin(); iter != files.end(); iter++)
			{
				AddUsingFile(support_dir + "/" + (*iter));
			}
		}

		// Import all packages we are using and insert them into the AST tree.
		if (m_using_files.size() > 0 || m_native_files.size() > 0)
		{
			// If we are an imported package ourselves, then pass the import up to the main package.
			if (importedPackage == true)
			{
				for (std::vector<String>::iterator iter = m_using_files.begin(); iter != m_using_files.end(); iter++)
				{
					importingPackage->AddUsingFile(*iter);
				}
			}

			// If we are the main package, compile and import all packages.
			else
			{			
				//Info("Importing Packages ...");	
				std::vector<String> imported_files;
				while (imported_files.size() != m_using_files.size())
				{			
					for (std::vector<String>::iterator iter = m_using_files.begin(); iter != m_using_files.end(); iter++)
					{
						String& using_path = *iter;

						bool imported = false;
						for (std::vector<String>::iterator iter2 = imported_files.begin(); iter2 != imported_files.end(); iter2++)
						{
							if (using_path == *iter2)
							{
								imported = true;
								break;
							}
						}

						if (imported == false)
						{			
							CTranslationUnit* unit = new CTranslationUnit(m_compiler, using_path, m_defines);
							unit->Compile(true, this);

							CASTNode* unitRoot = unit->GetASTRoot();
							CASTNode* realRoot = GetASTRoot();
							
							for (std::vector<CASTNode*>::iterator childIter = unitRoot->Children.begin(); childIter != unitRoot->Children.end(); childIter++)
							{
								realRoot->AddChild(*childIter);
							}

							m_imported_units.push_back(unit);
							imported_files.push_back(unit->GetFilePath());

							break;
						}
					}
					
				}
			}
		}
		
		// If we are importing this package then everything that follows
		// is done by the importer.
		if (importedPackage == true)
		{
			int elapsed = GetTicks() - start_tick_count;
			//Info("Imported " + m_filename + " in " + CStringHelper::ToString(elapsed) + "ms");
			return true;
		}
		
		// Check semantics are correct for AST.
		//DBG_LOG("Semantic Analysis ...");
		time = Platform::Get()->Get_Ticks();
		m_semanter.Process(this);
		g_script_semanter_time.Set((float)(Platform::Get()->Get_Ticks() - time));
		
		//DBG_LOG("Translating ...");
		// Translate into target language.
		time = Platform::Get()->Get_Ticks();
		m_compiler->GetTranslator()->Process(this);		
		g_script_translator_time.Set((float)(Platform::Get()->Get_Ticks() - time));
		//Info(CStringHelper::FormatString("Translated %s translator in %s ms.",
		//								m_filename.c_str(), 
		//								CStringHelper::ToString(GetTicks() - tick_count).c_str()));

		// Work out elapsed time.
		//int elapsed = GetTicks() - start_tick_count;
		//Info("Generated " + m_filename + " in " + CStringHelper::ToString(elapsed) + "ms");
		
		//DBG_LOG("Generated.");
		return true;
	}
	catch (std::runtime_error ex)
	{
		if (m_imported_package == true)
		{
			throw std::runtime_error("Fatal Error");
		}
		return false;
	}
}

// =================================================================
//	Attempts to preprocess the translation unit.
//	Returns true on success.
// =================================================================
bool CTranslationUnit::PreProcess()
{
	try
	{
		// Read source from disk.
		if (!CPathHelper::LoadFile(m_file_path, m_source))
		{
			FatalError("Could not read file: " + m_file_path);
		}
			
		// Preprocess the source file.
		m_preprocessor.Process(this);

		return true;
	}
	catch (std::runtime_error ex)
	{
		//Error("Failed to process file: " + m_filename);
		return false;
	}
}

// =================================================================
// 	Gets the current tick count.
// =================================================================
int	CTranslationUnit::GetTicks()
{
#ifdef _WIN32
	return GetTickCount();	
#elif defined(__linux__) || defined(__APPLE__)
	return clock() / (CLOCKS_PER_SEC / 1000);
#else
	assert(0);
#endif
}

// =================================================================
//	Runs an executable witht he given arguments and emits the 
//	output to the stdout.
// =================================================================
bool CTranslationUnit::Execute(String path, String cmd_line)
{
#ifdef _WIN32

	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFOA		si = { sizeof(si) };
	
	path = CStringHelper::Replace(path, "/", "\\");

	String dir = CPathHelper::StripFilename(path);

	String cmd = ("\"" + path + "\" " + cmd_line);

	if (!CreateProcessA(NULL, (LPSTR)cmd.c_str(), NULL, NULL, true, CREATE_DEFAULT_ERROR_MODE, NULL, (LPSTR)dir.c_str(), &si, &pi)) 
	{
		int err = GetLastError();
		return false;		
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	int res = GetExitCodeProcess(pi.hProcess, (DWORD*)&res) ? res : -1;

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return (res == 0);

#elif defined(__linux__) || defined(__APPLE__)

	pid_t pid = fork();
	int status;

	switch (pid) 
	{
	case -1: 
		return 0;
	
	case 0: 
		execl(path.c_str(), path.c_str(), cmd_line.c_str(), NULL); 
		exit(1);
		
	default: 
		while (!WIFEXITED(status)) 
		{
			waitpid(pid, &status, 0); 
		}

		return (WEXITSTATUS(status) == 0);
	}
	
#else

	assert(0);

#endif
}

