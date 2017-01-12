/* *****************************************************************

		CTranslator.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVMBinaryTranslator_H_
#define _CVMBinaryTranslator_H_

#include "XScript/Lexer/CToken.h"
#include "XScript/Translator/CTranslator.h"
#include "XScript/VirtualMachine/CSymbol.h"
#include "Generic/Types/HashTable.h"

class CVMBinary;
class CASTNode;
class CTranslationUnit;
class CClassASTNode;
class CClassStateASTNode;
class CClassPropertyASTNode;
class CDataType;
class CExpressionBaseASTNode;
class CExpressionASTNode;
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
class CTypeOfExpressionASTNode;
class CIndexOfExpressionASTNode;

// =================================================================
//	Serialization
// =================================================================
/*
struct CCompiledVMClassPackage
{
public:
	CCompiledVMClassBinary* classes;
	int						class_count;
};

struct CCompiledVMClassBinary
{
public:
	const char*				name;
	unsigned int			name_hash;
	CCompiledVMInstruction*	instructions;
	int						instruction_count;
};
*/

// =================================================================
//	Responsible for translating a parsed AST tree into a 
//	vm-runnable binary file.
// =================================================================
class CVMBinaryTranslator : public CTranslator
{
private:
	CPackageASTNode* m_package;
	CSymbol*		 m_base_scope;
	
	CSymbolDebugInfo			m_last_translated_debug_info;

	std::vector<String>	m_string_table;
	HashTable<int, int>			m_string_table_lookup;

	std::vector<CSymbol*>		m_symbol_table;
	std::vector<CSymbol*>		m_scope_stack;
	std::vector<int>			m_loop_break_jump_target_stack;
	std::vector<int>			m_loop_continue_jump_target_stack;

	void		SortSerializedFields(CASTNode* node);

	void		BuildStringTable	(CASTNode* node);
	void		BuildSymbolTable	(CASTNode* node);
	CSymbol*	BuildClassSymbol	(CClassASTNode* node);
	CSymbol*	BuildMemberSymbol	(CClassMemberASTNode* node);
	CSymbol*	BuildPropertySymbol	(CClassPropertyASTNode* node);
	CSymbol*	BuildVariableSymbol	(CVariableStatementASTNode* node);
	CSymbol*	BuildStateSymbol	(CClassStateASTNode* node);

	CSymbol*	NewSymbol(SymbolType::Type type, CASTNode* node);
	void		PushScope(CSymbol* scope);
	CSymbol*	PopScope();
	CSymbol*	GetScope();

	CSymbol*	GetParentScope(SymbolType::Type type);

	void PrintDebugSymbol(CSymbol* symbol);

	CInstructionBuffer* GetInstructionBuffer();
	
	int PushBreakJmpTarget();
	int PushContinueJmpTarget();
	void PopBreakJumpTarget();
	void PopContinueJumpTarget();

	void TranslateArguments(std::vector<CASTNode*>& nodes);
	int TranslateSubAssignmentExpression(CToken* token, CExpressionBaseASTNode* left_base, CExpressionBaseASTNode* right_base, bool postfix=false);

	void UpdateDebugTrace(CASTNode* node);

	int AddToStringTable(const String& str);

	void BuildVFTables();

public:

	CVMBinaryTranslator();
	~CVMBinaryTranslator();

	void TranslateChildren(CASTNode* node);

	CVMBinary* GetVMBinary();

	virtual void TranslatePackage					(CPackageASTNode* node);
	virtual void TranslateClass						(CClassASTNode* node);
	virtual void TranslateClassState				(CClassStateASTNode* node);
	virtual void TranslateClassMember				(CClassMemberASTNode* node);
	virtual void TranslateVariableStatement			(CVariableStatementASTNode* node);
	virtual void TranslateBlockStatement			(CBlockStatementASTNode* node);
	virtual void TranslateBreakStatement			(CBreakStatementASTNode* node);
	virtual void TranslateContinueStatement			(CContinueStatementASTNode* node);
	virtual void TranslateDoStatement				(CDoStatementASTNode* node);
	virtual void TranslateForEachStatement			(CForEachStatementASTNode* node);
	virtual void TranslateForStatement				(CForStatementASTNode* node);
	virtual void TranslateIfStatement				(CIfStatementASTNode* node);
	virtual void TranslateReturnStatement			(CReturnStatementASTNode* node);
	virtual void TranslateSwitchStatement			(CSwitchStatementASTNode* node);
	virtual void TranslateWhileStatement			(CWhileStatementASTNode* node);
	virtual void TranslateExpressionStatement		(CExpressionASTNode* node);
	virtual void TranslateStateChangeStatement		(CStateChangeStatementASTNode* node);

	virtual int TranslateExpression				(CExpressionASTNode* node);
	virtual int TranslateAssignmentExpression		(CAssignmentExpressionASTNode* node);
	virtual int TranslateBaseExpression			(CBaseExpressionASTNode* node);
	virtual int TranslateBinaryMathExpression		(CBinaryMathExpressionASTNode* node);
	virtual int TranslateCastExpression			(CCastExpressionASTNode* node);
	virtual int TranslateClassRefExpression		(CClassRefExpressionASTNode* node);
	virtual int TranslateCommaExpression			(CCommaExpressionASTNode* node);
	virtual int TranslateComparisonExpression		(CComparisonExpressionASTNode* node);
	virtual int TranslateFieldAccessExpression		(CFieldAccessExpressionASTNode* node);
	virtual int TranslateIdentifierExpression		(CIdentifierExpressionASTNode* node);
	virtual int TranslateLiteralExpression			(CLiteralExpressionASTNode* node, bool push_to_stack = false);
	virtual int TranslateLogicalExpression			(CLogicalExpressionASTNode* node);
	virtual int TranslateMethodCallExpression		(CMethodCallExpressionASTNode* node);
	virtual int TranslateNewExpression				(CNewExpressionASTNode* node);
	virtual int TranslatePostFixExpression			(CPostFixExpressionASTNode* node);
	virtual int TranslatePreFixExpression			(CPreFixExpressionASTNode* node);
	virtual int TranslateSliceExpression			(CSliceExpressionASTNode* node);
	virtual int TranslateTernaryExpression			(CTernaryExpressionASTNode* node);
	virtual int TranslateThisExpression				(CThisExpressionASTNode* node);
	virtual int TranslateTypeExpression				(CTypeExpressionASTNode* node);
	virtual int TranslateArrayInitializerExpression(CArrayInitializerASTNode* node);
	virtual int TranslateIndexExpression			(CIndexExpressionASTNode* node);
	virtual int TranslateTypeOfExpression			(CTypeOfExpressionASTNode* node);
	virtual int TranslateIndexOfExpression			(CIndexOfExpressionASTNode* node);

	

};

#endif

