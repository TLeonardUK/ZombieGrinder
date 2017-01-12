/* *****************************************************************

		CClassMemberASTNode.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CCLASSMEMBERASTNODE_H_
#define _CCLASSMEMBERASTNODE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/Lexer/CToken.h"
#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

class CClassBodyASTNode;
class CExpressionASTNode;
class CDataType;
class CMethodBodyASTNode;
class CVariableStatementASTNode;
class CClassASTNode;

#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



// =================================================================
//	Member type type.
// =================================================================
namespace MemberType
{
	enum Type
	{
		Method,
		Field
	};
};

// =================================================================
//	Stores information on a class member declaration.
// =================================================================
class CClassMemberASTNode : public CDeclarationASTNode
{
	

protected:	

public:
	AccessLevel::Type							AccessLevel;
	bool										IsStatic;
	bool										IsAbstract;
	bool										IsVirtual;
	bool										IsConst;
	bool										IsOverride;
	bool										IsConstructor;
	bool										IsExtension;
	bool										IsEvent;
	bool										IsRPC;
	int											VFTableOffset;	

	bool										IsSerialized;
	int											SerializeVersion;

	bool										IsOffsetForced;
	int											ForcedOffset;

	bool										IsOperatorOverload;
	CToken										OverloadedOperator;

	MemberType::Type							MemberType;		

	CClassMemberASTNode*						BaseVirtualMethod;

	CMethodBodyASTNode*							Body;		
	CExpressionASTNode*							Assignment;	
	std::vector<CVariableStatementASTNode*>		Arguments;
	CDataType*									ReturnType;		

	// Constructors.
	CClassMemberASTNode	(CASTNode* parent, CToken token);
	~CClassMemberASTNode();
	
	// General management.
	virtual String ToString();

	// Semantic analysis.
	virtual CASTNode* Semant				(CSemanter* semanter);
	virtual CASTNode* Finalize				(CSemanter* semanter);	
	virtual CASTNode* Clone					(CSemanter* semanter);
	virtual void	  CheckAccess			(CSemanter* semanter, CASTNode* referenceBy);

	void AddClassConstructorStub			(CSemanter* semanter);
	void AddInstanceConstructorStub			(CSemanter* semanter);
	void AddInstanceConstructorPostfix		(CSemanter* semanter);
	void AddInstanceConstructorPrefix		(CSemanter* semanter);
	void AddDefaultReturnExpression			(CSemanter* semanter);
	void AddComponentConstructorPrefix		(CSemanter* semanter);

	void AddMethodConstructorStub			(CSemanter* semanter);
	
	bool EqualToMember						(CSemanter* semanter, CClassMemberASTNode* other);

	virtual void	Translate				(CTranslator* translator);

};

#endif