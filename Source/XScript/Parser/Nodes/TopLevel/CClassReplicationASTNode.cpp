/* *****************************************************************

		CClassReplicationASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Values/CConstDictionaryASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassReplicationASTNode.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"



#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassReplicationASTNode::CClassReplicationASTNode(CASTNode* parent, CToken token) 
	: CASTNode(parent, token)
{
}

// =================================================================
//	Destructor.
// =================================================================
CClassReplicationASTNode::~CClassReplicationASTNode()
{
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassReplicationASTNode::Clone(CSemanter* semanter)
{
	CClassReplicationASTNode* clone = new CClassReplicationASTNode(NULL, Token);
	
	clone->PriorityExpression = dynamic_cast<CExpressionBaseASTNode*>(PriorityExpression->Clone(semanter));
	clone->AddChild(clone->PriorityExpression);

	for (std::vector<CClassReplicationVariable>::iterator iter = Variables.begin(); iter != Variables.end(); iter++)
	{
		CClassReplicationVariable& var = *iter;
		CClassReplicationVariable clone_var;
		clone_var.Identifier		 = var.Identifier;
		clone_var.OwnerExpression	 = dynamic_cast<CExpressionBaseASTNode*>(var.OwnerExpression->Clone(semanter));
		clone_var.ModeExpression	 = dynamic_cast<CExpressionBaseASTNode*>(var.ModeExpression->Clone(semanter));

		if (var.RangeMinExpression != NULL)
			clone_var.RangeMinExpression = dynamic_cast<CExpressionBaseASTNode*>(var.RangeMinExpression->Clone(semanter));
		
		if (var.RangeMaxExpression != NULL)
			clone_var.RangeMaxExpression = dynamic_cast<CExpressionBaseASTNode*>(var.RangeMaxExpression->Clone(semanter));

		clone->AddChild(clone_var.OwnerExpression);
		clone->AddChild(clone_var.ModeExpression);
	
		if (var.RangeMinExpression != NULL)
			clone->AddChild(clone_var.RangeMinExpression);
		
		if (var.RangeMaxExpression != NULL)
			clone->AddChild(clone_var.RangeMaxExpression);

		clone->Variables.push_back(var);
	}

	return clone;
}

// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CClassReplicationASTNode::ToString()
{
	return "Class Replication";
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassReplicationASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CClassReplicationASTNode");

	// Only semant once.
	if (Semanted == true)
	{
		//m_semanting = false;
		return this;
	}
	Semanted = true;

	// Semant all values.
	SemantChildren(semanter);
	
	// Check vars are pointing to valid fields.
	for (std::vector<CClassReplicationVariable>::iterator iter = Variables.begin(); iter != Variables.end(); iter++)
	{
		CClassReplicationVariable& var = *iter;
		var.Resolved = NULL;
		var.ResolvedProperty = NULL;

		var.Resolved = FindClassField(semanter, var.Identifier, NULL, NULL);
		if (var.Resolved == NULL)
		{
			var.ResolvedProperty = FindClassProperty(semanter, var.Identifier, NULL, NULL);
			if (var.ResolvedProperty == NULL)
			{
				semanter->GetContext()->FatalError(StringHelper::Format("Attempt to replicate undefined field '%s'.", var.Identifier.c_str()), var.OwnerExpression->Token);
			}
		}
	}

	return this;
}

// =================================================================
//	Performs finalization on this class.
// =================================================================
CASTNode* CClassReplicationASTNode::Finalize(CSemanter* semanter)
{
	return this;
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CClassReplicationASTNode::Translate(CTranslator* translator)
{
	// No translation is performed for this, as it just holds values for the actual class.
}