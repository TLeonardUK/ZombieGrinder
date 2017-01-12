/* *****************************************************************

		CParser.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CPARSER_H_
#define _CPARSER_H_

#include "XScript/Lexer/CToken.h"

#include "XScript/Parser/Nodes/CASTNodeMetaData.h"

class CCompiler;
class CTranslationUnit;
class CASTNode;
class CDataType;
class CClassBodyASTNode;
class CClassStateASTNode;
class CMethodBodyASTNode;
class CClassMemberASTNode;
class CClassASTNode;
class CClassStateASTNode;
class CClassPropertyASTNode;
class CExpressionASTNode;

class CIfStatementASTNode;
class CBlockStatementASTNode;

class CWhileStatementASTNode;
class CBreakStatementASTNode;
class CContinueStatementASTNode;
class CDoStatementASTNode;
class CSwitchStatementASTNode;
class CForStatementASTNode;
class CForEachStatementASTNode;
class CReturnStatementASTNode;
class CCaseStatementASTNode;
class CDefaultStatementASTNode;
class CVariableStatementASTNode;
class CStateChangeStatementASTNode;
class CMethodCallExpressionASTNode;

class CConstDictionaryASTNode;

class CDataType;
class CIdentifierDataType;
class CArrayInitializerASTNode;

// =================================================================
//	Class deals with taken a stream of tokens and converting in
//	into a AST representation.
// =================================================================
class CParser
{
private:
	CTranslationUnit*				m_context;
	int								m_token_offset;
	CToken							m_eof_token;
	CToken							m_sof_token;

	CASTNode*						m_root;
	CASTNode*						m_scope;

	std::vector<CASTNode*>			m_scope_stack;

	std::vector<CASTNodeMetaData>	m_meta_data_queue;

	// Token stream manipulation.
	bool						EndOfTokens						(int offset = 0);
	CToken&						NextToken						();
	CToken&						LookAheadToken					(int offset = 1);
	CToken&						CurrentToken					();
	CToken&						PreviousToken					();
	CToken&						ExpectToken						(TokenIdentifier::Type type);
	void						RewindStream					(int offset = 1);

	// Scope manipulation.
	void						PushScope						(CASTNode* node);
	void						PopScope						();
	CASTNode*					CurrentScope					();

	bool						IsGenericTypeListFollowing		(int& final_token_offset);

	CClassASTNode*				CurrentClassScope				();
	CClassStateASTNode*			CurrentClassStateScope			();
	CClassMemberASTNode*		CurrentClassMemberScope			();

	void						BindMetaData					(CASTNode* node);
	
	// Parse statements.
	void						ParseTopLevelStatement			();

	void						ParseUsingStatement				();

	void						ParseMetaDataStatement			();

	CDataType*					ParseDataType					(bool acceptArrays = true);
	CIdentifierDataType*		ParseIdentifierDataType			();

	CClassASTNode*				ParseClassStatement				();
	CClassStateASTNode*			ParseClassStateStatement		();
	CClassBodyASTNode*			ParseClassBody					();
	CASTNode*					ParseClassBodyStatement			();
	CClassMemberASTNode*		ParseClassMemberStatement		();
	CClassPropertyASTNode*		ParsePropertyStatement			();

	void						ParseMethodArguments			(CClassMemberASTNode* method);
	CMethodBodyASTNode*			ParseMethodBody					();
	CASTNode*					ParseMethodBodyStatement		();

	CIfStatementASTNode*		ParseIfStatement				();
	CBlockStatementASTNode*		ParseBlockStatement				();
	CWhileStatementASTNode*		ParseWhileStatement				();
	CBreakStatementASTNode*		ParseBreakStatement				();
	CReturnStatementASTNode*	ParseReturnStatement			();
	CContinueStatementASTNode*	ParseContinueStatement			();
	CDoStatementASTNode*		ParseDoStatement				();
	CSwitchStatementASTNode*	ParseSwitchStatement			();
	CForStatementASTNode*		ParseForStatement				();
	CForEachStatementASTNode*	ParseForEachStatement			();
	CVariableStatementASTNode*	ParseLocalVariableStatement		(bool acceptMultiple = true, bool acceptAssignment = true, bool acceptNonConstAssignment = true);

	CStateChangeStatementASTNode* ParseStateChangeStatement		();
	CExpressionASTNode*			  ParseRPCStatement				();
	CMethodCallExpressionASTNode* ParseMethodCall				();

	void						  ParseComponentsStatement		();

	void						  ParseReplicateStatement		();
	CConstDictionaryASTNode*	  ParseConstDictionary			();

	CArrayInitializerASTNode*	ParseArrayInitializer			();

	CExpressionASTNode*			ParseExpr						(bool useNullScope, bool noSequencePoints = false);
	CExpressionASTNode*			ParseConstExpr					(bool useNullScope, bool noSequencePoints = false);
	CASTNode*					ParseExprComma					();
	CASTNode*					ParseExprIsAs					();
	CASTNode*					ParseExprAssignment				();
	CASTNode*					ParseExprTernary				();
	CASTNode*					ParseExprLogical				();
	CASTNode*					ParseExprBitwise				();
	CASTNode*					ParseExprCompare				();
	CASTNode*					ParseExprAddSub					();
	CASTNode*					ParseExprMulDiv					();
	CASTNode*					ParseExprTypeCast				();
	CASTNode*					ParseExprPrefix					();
	CASTNode*					ParseExprPostfix				();
	CASTNode*					ParseExprFactor					();

	CASTNode*					ParseNewExpression				(bool struct_syntax = false);
	CASTNode*					ParseTypeOfExpression			();
	CASTNode*					ParseIndexOfExpression			();

public:
	CParser														();
	
	CASTNode*					GetASTRoot						();

	bool Process												(CTranslationUnit* context);
	bool Evaluate												(CTranslationUnit* context);

};

#endif

