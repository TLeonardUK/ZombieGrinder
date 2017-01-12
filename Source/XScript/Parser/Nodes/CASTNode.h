/* *****************************************************************

		CASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CASTNODE_H_
#define _CASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"

#include "XScript/Parser/Nodes/EvaluationResult.h"
#include "XScript/Parser/Nodes/CASTNodeMetaData.h"

#include "XScript/Parser/Nodes/CDeclarationIdentifier.h"

#include "XScript/VirtualMachine/CSymbol.h"



class CSemanter;
class CTranslationUnit;
class CExpressionBaseASTNode;
class CDataType;

class CAliasASTNode;
class CDeclarationASTNode;
class CPackageASTNode;
class CClassASTNode;
class CClassMemberASTNode;
class CClassStateASTNode;
class CClassPropertyASTNode;

class CTranslator;

class CSymbol;

// =================================================================
//	Access level type.
// =================================================================
namespace AccessLevel
{
	enum Type
	{
		PUBLIC,
		PRIVATE,
		PROTECTED
	};
};

// =================================================================
//	Base class used to store a representation of individual nodes
//	in an Abstract Syntax Tree.
// =================================================================
class CASTNode
{
	

private:
	static int g_create_index_tracker;
	int m_create_index;

protected:

public:
	CToken							Token;
	CASTNode*						Parent;
	std::vector<CASTNode*>			Children;
	std::vector<CAliasASTNode*>		AliasChildren;
	std::vector<CASTNodeMetaData>	MetaData;

	bool							Collected;
	
	bool							Semanted;

	// Translation helper fields. Really should go
	// in translator, not here D:
	CSymbol* TranslatedSymbol;
	CSymbolDebugInfo TranslatedDebugInfo;

	// General management.
	virtual String ToString();
	
	// Child management.
	void	  AddChild		(CASTNode* node, bool atStart=false);
	void	  RemoveChild	(CASTNode* node);
	CASTNode* ReplaceChild	(CASTNode* replace, CASTNode* with);

	// Semantic analysis.
	//void					PrepareChildren		(CSemanter* semanter);
	//virtual CASTNode*		Prepare				(CSemanter* semanter);

	void					SemantChildren		(CSemanter* semanter);
	virtual CASTNode*		Semant				(CSemanter* semanter);

	CExpressionBaseASTNode* SemantAsExpression	(CSemanter* semanter);	

	void					FinalizeChildren	(CSemanter* semanter);
	virtual CASTNode*		Finalize			(CSemanter* semanter);

	virtual CASTNode*		Clone				(CSemanter* semanter) = 0;
	virtual void			CloneChildren		(CSemanter* semanter, CASTNode* parent);

	virtual void			Translate			(CTranslator* translator);
	virtual void			TranslateChildren	(CTranslator* translator);
		
	// Finding things.	
	virtual CAliasASTNode*			FindAlias							(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode* ignoreNode=NULL);
	virtual CDeclarationASTNode*	FindDeclaration						(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode* ignoreNode=NULL);
	virtual CDeclarationASTNode*	FindScopedDeclaration				(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode* ignoreNode=NULL);
	virtual CDeclarationASTNode*	FindDataTypeDeclaration				(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode* ignoreNode=NULL);
	virtual CDataType*				FindDataType						(CSemanter* semanter, CDeclarationIdentifier identifier, std::vector<CDataType*> generic_arguments, bool ignore_access = false, bool do_not_semant = false);
	virtual CPackageASTNode* 		FindNodePackageScope				(CSemanter* semanter);
	virtual CClassASTNode*	 		FindClassScope						(CSemanter* semanter);
	virtual CClassMemberASTNode*	FindClassMethodScope				(CSemanter* semanter);
	virtual CClassMemberASTNode*	FindClassMethod						(CSemanter* semanter, CDeclarationIdentifier identifier, std::vector<CDataType*> arguments, bool explicit_arguments, CASTNode* ignoreNode=NULL, CASTNode* referenceNode=NULL);
	virtual CClassMemberASTNode*	FindClassField						(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode, CASTNode* referenceNode=NULL);
	virtual CClassPropertyASTNode*	FindClassProperty					(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode, CASTNode* referenceNode=NULL);
	virtual CClassStateASTNode*		FindClassState						(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode, CASTNode* referenceNode=NULL);
	virtual CASTNode*				FindLoopScope						(CSemanter* semanter);

	virtual	CASTNode*				GetParentSearchScope				(CSemanter* semanter);
	virtual std::vector<CASTNode*>& GetSearchScopeChildren				(CSemanter* semanter);
	virtual std::vector<CAliasASTNode*>& GetSearchScopeAliasChildren(CSemanter* semanter);

	virtual void					CheckForDuplicateScopedIdentifier	(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode=NULL);
	virtual void					CheckForDuplicateIdentifier			(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode=NULL);
	virtual void					CheckForDuplicateMethodIdentifier	(CSemanter* semanter, CDeclarationIdentifier identifier, std::vector<CDataType*> arguments, CClassMemberASTNode*	ignoreNode=NULL);

	virtual	EvaluationResult		Evaluate							(CTranslationUnit* unit);

	virtual bool					AcceptBreakStatement				();
	virtual bool					AcceptContinueStatement				();

	// Constructing.
	CASTNode();
	CASTNode(CASTNode* parent, CToken token);
	~CASTNode();

};

#endif