/* *****************************************************************

		CTranslator.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CTRANSLATOR_H_
#define _CTRANSLATOR_H_

#include "XScript/Lexer/CToken.h"

class CTranslationUnit;
class CPackageASTNode;
class CClassASTNode;
class CClassMemberASTNode;
class CDataType;
class CExpressionASTNode;
class CExpressionBaseASTNode;
class CClassStateASTNode;

class CBlockStatementASTNode;
class CBreakStatementASTNode;
class CContinueStatementASTNode;
class CDoStatementASTNode;
class CForEachStatementASTNode;
class CForStatementASTNode;
class CIfStatementASTNode;
class CReturnStatementASTNode;
class CSwitchStatementASTNode;
class CWhileStatementASTNode;

class CVariableStatementASTNode;
class CAssignmentExpressionASTNode;
class CBaseExpressionASTNode;
class CBinaryMathExpressionASTNode;
class CCastExpressionASTNode;
class CClassRefExpressionASTNode;
class CCommaExpressionASTNode;
class CComparisonExpressionASTNode;
class CFieldAccessExpressionASTNode;
class CIdentifierExpressionASTNode;
class CIndexExpressionASTNode;
class CLiteralExpressionASTNode;
class CLogicalExpressionASTNode;
class CMethodCallExpressionASTNode;
class CNewExpressionASTNode;
class CPostFixExpressionASTNode;
class CPreFixExpressionASTNode;
class CSliceExpressionASTNode;
class CTernaryExpressionASTNode;
class CThisExpressionASTNode;
class CTypeExpressionASTNode;
class CArrayInitializerASTNode;
class CStateChangeStatementASTNode;
class CTypeOfExpressionASTNode;
class CIndexOfExpressionASTNode;

class CSemanter;

// =================================================================
//	Responsible for translating a parsed AST tree into a 
//	native language.
// =================================================================
class CTranslator
{
protected:
	CTranslationUnit* m_context;
	CSemanter* m_semanter;

public:
	virtual ~CTranslator() {}

	bool				Process							(CTranslationUnit* context);
	CTranslationUnit*	GetContext						();

	virtual void TranslatePackage						(CPackageASTNode* node) = 0;
	virtual void TranslateClass							(CClassASTNode* node) = 0;
	virtual void TranslateClassState					(CClassStateASTNode* node) = 0;
	virtual void TranslateClassMember					(CClassMemberASTNode* node) = 0;
	virtual void TranslateVariableStatement				(CVariableStatementASTNode* node) = 0;

	virtual void TranslateBlockStatement				(CBlockStatementASTNode* node) = 0;
	virtual void TranslateBreakStatement				(CBreakStatementASTNode* node) = 0;
	virtual void TranslateContinueStatement				(CContinueStatementASTNode* node) = 0;
	virtual void TranslateDoStatement					(CDoStatementASTNode* node) = 0;
	virtual void TranslateForEachStatement				(CForEachStatementASTNode* node) = 0;
	virtual void TranslateForStatement					(CForStatementASTNode* node) = 0;
	virtual void TranslateIfStatement					(CIfStatementASTNode* node) = 0;
	virtual void TranslateReturnStatement				(CReturnStatementASTNode* node) = 0;
	virtual void TranslateSwitchStatement				(CSwitchStatementASTNode* node) = 0;
	virtual void TranslateWhileStatement				(CWhileStatementASTNode* node) = 0;
	virtual void TranslateExpressionStatement			(CExpressionASTNode* node) = 0;
	virtual void TranslateStateChangeStatement			(CStateChangeStatementASTNode* node) = 0;
	
	virtual int TranslateExpression				(CExpressionASTNode* node) = 0;
	virtual int TranslateAssignmentExpression		(CAssignmentExpressionASTNode* node) = 0;
	virtual int TranslateBaseExpression			(CBaseExpressionASTNode* node) = 0;
	virtual int TranslateBinaryMathExpression		(CBinaryMathExpressionASTNode* node) = 0;
	virtual int TranslateCastExpression			(CCastExpressionASTNode* node) = 0;
	virtual int TranslateClassRefExpression		(CClassRefExpressionASTNode* node) = 0;
	virtual int TranslateCommaExpression			(CCommaExpressionASTNode* node) = 0;
	virtual int TranslateComparisonExpression		(CComparisonExpressionASTNode* node) = 0;
	virtual int TranslateFieldAccessExpression		(CFieldAccessExpressionASTNode* node) = 0;
	virtual int TranslateIdentifierExpression		(CIdentifierExpressionASTNode* node) = 0;
	virtual int TranslateLiteralExpression			(CLiteralExpressionASTNode* node, bool push_to_stack = false) = 0;
	virtual int TranslateLogicalExpression			(CLogicalExpressionASTNode* node) = 0;
	virtual int TranslateMethodCallExpression		(CMethodCallExpressionASTNode* node) = 0;
	virtual int TranslateNewExpression				(CNewExpressionASTNode* node) = 0;
	virtual int TranslatePostFixExpression			(CPostFixExpressionASTNode* node) = 0;
	virtual int TranslatePreFixExpression			(CPreFixExpressionASTNode* node) = 0;
	virtual int TranslateSliceExpression			(CSliceExpressionASTNode* node) = 0;
	virtual int TranslateTernaryExpression			(CTernaryExpressionASTNode* node) = 0;
	virtual int TranslateThisExpression			(CThisExpressionASTNode* node) = 0;
	virtual int TranslateTypeExpression			(CTypeExpressionASTNode* node) = 0;
	virtual int TranslateArrayInitializerExpression(CArrayInitializerASTNode* node) = 0;
	virtual int TranslateIndexExpression			(CIndexExpressionASTNode* node) = 0;
	virtual int TranslateTypeOfExpression			(CTypeOfExpressionASTNode* node) = 0;
	virtual int TranslateIndexOfExpression			(CIndexOfExpressionASTNode* node) = 0;
	
};

#endif

