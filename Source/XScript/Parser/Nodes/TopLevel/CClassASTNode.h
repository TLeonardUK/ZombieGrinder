/* *****************************************************************

		CClassASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCLASSASTNODE_H_
#define _CCLASSASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



class CClassBodyASTNode;
class CClassStateASTNode;
class CIdentifierDataType;
class CObjectDataType;
class CClassReplicationASTNode;
class CClassReferenceDataType;
class CSymbol;

// =================================================================
//	Stores information on a class declaration.
// =================================================================
class CClassASTNode : public CDeclarationASTNode
{
	

protected:	
	bool								m_semanting;

public:

	// Parsing infered data.
	AccessLevel::Type					AccessLevel;
	bool								IsStatic;
	bool								IsAbstract;
	bool								IsInterface;
	bool								IsSealed;
	bool								IsGeneric;
	bool								InheritsNull;
	bool								IsInstanced;
	bool								IsEnum;
	bool								IsReplicated;
	CASTNode*							InstancedBy;
	bool								IsStruct;

	CClassReplicationASTNode*			ReplicationNode;

	bool								HasBoxClass;
	String							BoxClassIdentifier;

	std::vector<CToken>					GenericTypeTokens;
	std::vector<CIdentifierDataType*>	InheritedTypes;		

	CClassBodyASTNode*					Body;	

	CObjectDataType*					ObjectDataType;
	CClassReferenceDataType*			ClassReferenceDataType;

	CClassMemberASTNode*				ClassConstructor;
	CClassMemberASTNode*				InstanceConstructor;
	CClassMemberASTNode*				ComponentConstructor;

	// Semanting infered data.
	std::vector<CClassASTNode*>			GenericInstances;
	CClassASTNode*						GenericInstanceOf;
	std::vector<CDataType*>				GenericInstanceTypes;
	
	CClassASTNode*						SuperClass;
	std::vector<CClassASTNode*>			Interfaces;	

	CClassStateASTNode*					DefaultState;
	std::vector<CClassStateASTNode*>	States;

	// General management.
	virtual String ToString();

	// Initialization.
	CClassASTNode(CASTNode* parent, CToken token);
	virtual ~CClassASTNode();
	
	// Semantic analysis.
	virtual CASTNode*				Semant					(CSemanter* semanter);
	virtual CASTNode*				Finalize				(CSemanter* semanter);	
	virtual CASTNode*				Clone					(CSemanter* semanter);

	virtual void					CheckAccess				(CSemanter* semanter, CASTNode* referenceBy);
	bool							InheritsFromClass		(CSemanter* semanter, CClassASTNode* node);

	virtual	CASTNode*				GetParentSearchScope	(CSemanter* semanter);
	virtual std::vector<CASTNode*>& GetSearchScopeChildren	(CSemanter* semanter);
	virtual std::vector<CAliasASTNode*>& GetSearchScopeAliasChildren(CSemanter* semanter);

	CClassASTNode*					GenerateClassInstance	(CSemanter* semanter, CASTNode* referenceNode, std::vector<CDataType*> generic_arguments);
	
	virtual CClassMemberASTNode*	FindClassMethod			(CSemanter* semanter, CDeclarationIdentifier identifier, std::vector<CDataType*> arguments, bool explicit_arguments, CASTNode* ignoreNode=NULL, CASTNode* referenceNode=NULL);
	virtual CClassMemberASTNode*	FindClassField			(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode, CASTNode* referenceNode);
	virtual CClassStateASTNode*		FindClassState			(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode, CASTNode* referenceNode);
	virtual CClassPropertyASTNode*	FindClassProperty		(CSemanter* semanter, CDeclarationIdentifier identifier, CASTNode*	ignoreNode, CASTNode* referenceNode);

	void							GatherMatchingClassMethods(std::vector<CClassMemberASTNode*>& nodes, CASTNode* base, CSemanter* semanter, CDeclarationIdentifier	identifier, std::vector<CDataType*>	arguments, bool explicit_arguments, CASTNode* ignoreNode, CASTNode* referenceNode);

	virtual void					Translate				(CTranslator* translator);

};

#endif