/* *****************************************************************

		CPreprocessor.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CPREPROCESSOR_H_
#define _CPREPROCESSOR_H_

#include "XScript/Lexer/CToken.h"

#include "XScript/Parser/Nodes/EvaluationResult.h"

class CCompiler;
class CTranslationUnit;

// =================================================================
//	Responsible for preprocessing a source file.
// =================================================================
class CPreprocessor
{
private:
	std::vector<String>	m_lines;
	unsigned int				m_line_index;
	String					m_result;
	std::vector<CDefine>*		m_defines;
	CTranslationUnit*			m_context;
	CToken						m_lineToken;
	String					m_currentLine;

	bool						m_accept_input;
		
	std::vector<String> SplitLine			(String line);
	String				 ReplaceDefineTags	(String line);

	bool		EndOfLines		();
	void		ParseLine		(String line);
	String ReadLine		();
	String LookAheadLine	();
	String CurrentLine		();
	void		Output			(String output);

	void		ParseIfBlock	();
	void		SkipIfBlock		();

	void		ParseIf			(String line);
	void		ParseDefine		(String line);
	void		ParseUndefine	(String line);
	void		ParseError		(String line);
	void		ParseWarning	(String line);
	void		ParseInfo		(String line);

public:
	bool Process(CTranslationUnit* context);
	EvaluationResult Evaluate(CTranslationUnit* context, CToken& token, String expr, std::vector<CDefine>* defines);

};

#endif

