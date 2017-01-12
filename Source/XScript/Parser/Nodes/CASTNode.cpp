/* *****************************************************************

		CASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#ifdef _WIN32
#include <Windows.h>
#endif

#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Translator/CTranslator.h"
#include "XScript/Semanter/CSemanter.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Parser/Types/Helper/CDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CAliasASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CPackageASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

#include "XScript/Parser/Types/CObjectDataType.h"

int CASTNode::g_create_index_tracker = 0;

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CASTNode::CASTNode()
{
	m_create_index = g_create_index_tracker++;

	Collected = false;

	TranslatedSymbol = NULL;

	Parent = NULL;
	Children.clear();
}

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CASTNode::CASTNode(CASTNode* parent, CToken token)
{
	m_create_index = g_create_index_tracker++;

	Collected = false;
	Parent = parent;
	Token = token;
	Semanted = false;

	TranslatedSymbol = NULL;

	if (parent != NULL)
	{
		parent->AddChild(this);
		//parent->Children.push_back(this);
	}
}

// =================================================================
//	Disposes of this node.
// =================================================================
CASTNode::~CASTNode()
{
//	for (unsigned int i = 0; i < Children.size(); i++)
//	{
//		CASTNode* node = Children.at(i);
//		SAFE_DELETE(node);
//	}

	Parent = NULL;
//	Children.clear();
}

// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CASTNode::ToString()
{
	return "untitled-node";
}

// =================================================================
//	Adds a child node to this node.
// =================================================================
void CASTNode::AddChild(CASTNode* node, bool atStart)
{
	node->Parent = this;

	if (atStart == true)
	{
		SEMANT_TRACE("Add %s At Start", typeid(*node).name());
		Children.insert(Children.begin(), node);
	}
	else
	{
		SEMANT_TRACE("Add %s", typeid(*node).name());
		Children.push_back(node);
	}
	
//	if (node->m_create_index == 54504)
//	{
//		__debugbreak();
//	}

	SEMANT_TRACE("==================================");
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		SEMANT_TRACE("\tChild:%s - %s", typeid(**iter).name(), (*iter)->ToString().c_str());
	}
	SEMANT_TRACE("==================================");
}

// =================================================================
//	Removes a child node from this node.
// =================================================================
void CASTNode::RemoveChild(CASTNode* node)
{
	node->Parent = NULL;

	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		if (*iter == node)
		{	
			SEMANT_TRACE("Removed %s", typeid(*node).name());

			iter = Children.erase(iter);
			break;
		}
	}
	
	SEMANT_TRACE("==================================");
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{	
		SEMANT_TRACE("\tChild:%s - %s", typeid(**iter).name(), (*iter)->ToString().c_str());
	}
	SEMANT_TRACE("==================================");
}

// =================================================================
//	Replaces a child with another child and returns it.
// =================================================================
CASTNode* CASTNode::ReplaceChild(CASTNode* replace, CASTNode* with)
{
	if (replace == with)
	{
		return replace;
	}

	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		if (*iter == replace)
		{
			SEMANT_TRACE("Removed %s", typeid(*replace).name());

			Children.erase(iter);
			break;
		}
	}
	
	SEMANT_TRACE("Replaced %s with %s", typeid(*replace).name(), typeid(*with).name());

	Children.push_back(with);
	with->Parent = this;

	SEMANT_TRACE("==================================");
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		SEMANT_TRACE("\tChild:%s - %s", typeid(**iter).name(), (*iter)->ToString().c_str());
	}
	SEMANT_TRACE("==================================");

	return with;
}

/*
// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CASTNode::Prepare(CSemanter* semanter)
{
	PrepareChildren(semanter);
	return NULL;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
void CASTNode::PrepareChildren(CSemanter* semanter)
{
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		(*iter)->Prepare(semanter);
	}
}
*/

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CASTNode::Semant(CSemanter* semanter)
{
	semanter->GetContext()->FatalError("AST node cannot be semanted.", Token);
	return NULL;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
void CASTNode::SemantChildren(CSemanter* semanter)
{
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		(*iter)->Semant(semanter);
	}
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CASTNode::Translate(CTranslator* translator)
{
	translator->GetContext()->FatalError("AST node cannot be translated.", Token);
}

// =================================================================
//	Causes this nodes children to be translated.
// =================================================================
void CASTNode::TranslateChildren(CTranslator* translator)
{
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		(*iter)->Translate(translator);
	}
}

// =================================================================
//	Semants an expression and returns the result as an expression
//	base node.
// =================================================================
CExpressionBaseASTNode* CASTNode::SemantAsExpression(CSemanter* semanter)
{
	return dynamic_cast<CExpressionBaseASTNode*>(Semant(semanter));
}

// =================================================================
//	Performs finalization on this nodes children.
// =================================================================
void CASTNode::FinalizeChildren(CSemanter* semanter)
{
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		(*iter)->Finalize(semanter);
	}
}

// =================================================================
//	Performs finalization on this node.
// =================================================================
CASTNode* CASTNode::Finalize(CSemanter* semanter)
{
	FinalizeChildren(semanter);
	return NULL;
}

// =================================================================
//	Clones this nodes children and adds them to a parent node.
// =================================================================
void CASTNode::CloneChildren(CSemanter* semanter, CASTNode* parent)
{
	for (std::vector<CASTNode*>::iterator iter = Children.begin(); iter != Children.end(); iter++)
	{
		CASTNode* node = (*iter)->Clone(semanter);
		parent->AddChild(node);
	}
}

// =================================================================
//	Gets the package the given node is in.
// =================================================================
CPackageASTNode* CASTNode::FindNodePackageScope(CSemanter* semanter)
{
	CASTNode* node = this;

	while (node != NULL)
	{
		CPackageASTNode* package = dynamic_cast<CPackageASTNode*>(node);
		if (package != NULL)
		{
			return package;
		}
		node = node->Parent;
	}

	return NULL;
}

// =================================================================
//	Finds the class method scope we are in.
// =================================================================
CClassMemberASTNode* CASTNode::FindClassMethodScope(CSemanter* semanter)
{
	CASTNode* node = this;

	while (node != NULL)
	{
		CClassMemberASTNode* scope = dynamic_cast<CClassMemberASTNode*>(node);
		if (scope != NULL && scope->MemberType == MemberType::Method)
		{
			return scope;
		}
		node = node->Parent;
	}

	return NULL;
}

// =================================================================
//	Finds the class a node is in.
// =================================================================
CClassASTNode* CASTNode::FindClassScope(CSemanter* semanter)
{	
	CASTNode* node = this;

	while (node != NULL)
	{
		CClassASTNode* scope = dynamic_cast<CClassASTNode*>(node);
		if (scope != NULL)
		{
			return scope;
		}
		node = node->GetParentSearchScope(semanter);//node->Parent;
	}

	return NULL;
}

// =================================================================
//	Finds the scope the looping statement this node is contained by.
// =================================================================
CASTNode* CASTNode::FindLoopScope(CSemanter* semanter)
{
	// Overridden in loop statements to return themselves.

	if (Parent != NULL)
	{
		return Parent->FindLoopScope(semanter);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Returns true if this node can accept break statements inside
//	of it.
// =================================================================
bool CASTNode::AcceptBreakStatement()
{
	// Overridden in loop statements.

	return false;
}

// =================================================================
//	Returns true if this node can accept continue statements inside
//	of it.
// =================================================================
bool CASTNode::AcceptContinueStatement()
{
	// Overridden in loop statements.

	return false;
}

// =================================================================
//	Finds the given type on the scope stack.
// =================================================================
CDataType* CASTNode::FindDataType(CSemanter* semanter, CDeclarationIdentifier identifier, std::vector<CDataType*> generic_arguments, bool ignore_access, bool do_not_semant)
{
	CDeclarationASTNode* decl = FindDataTypeDeclaration(semanter, identifier);

	CAliasASTNode* dt_decl = dynamic_cast<CAliasASTNode*>(decl);
	if (dt_decl != NULL)
	{
		if (dt_decl->AliasedDataType != NULL)
		{
			return dt_decl->AliasedDataType;
		}
		else
		{
			decl = dt_decl->AliasedDeclaration;
		}
	}

	CClassASTNode* class_decl = dynamic_cast<CClassASTNode*>(decl);
	if (class_decl != NULL)
	{
		if (ignore_access == false)
		{
			class_decl->CheckAccess(semanter, this);
		}
		class_decl = class_decl->GenerateClassInstance(semanter, this, generic_arguments);
		if (do_not_semant == false)
		{
		//	class_decl->Semant(semanter);
		}
		return class_decl->ObjectDataType;
	}

	return NULL;
}

// =================================================================
//	Finds the declaration in the scope stack with the given
//  identifier.
// =================================================================
CDeclarationASTNode* CASTNode::FindDataTypeDeclaration(CSemanter* semanter, 
													   CDeclarationIdentifier identifier,
													  CASTNode* ignoreNode)
{
	CAliasASTNode* aliasNode = FindAlias(semanter, identifier, ignoreNode);
	if (aliasNode != NULL)
	{
		return aliasNode;
	}

	// Find normal declarations.
	std::vector<CASTNode*>& nodes = GetSearchScopeChildren(semanter);
	for (std::vector<CASTNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		CClassASTNode* node = dynamic_cast<CClassASTNode*>(*iter);
		if (node != NULL && node != ignoreNode)
		{
			if (node->Identifier == identifier)
			{
				return node;
			}
		}
	}

	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindDataTypeDeclaration(semanter, identifier, ignoreNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds the declaration in the scope stack with the given
//  identifier.
// =================================================================
CDeclarationASTNode* CASTNode::FindDeclaration(CSemanter* semanter, 
											   CDeclarationIdentifier identifier,
											   CASTNode* ignoreNode)
{
	//DBG_LOG("Looking for %s", identifier.Get_String().c_str());

	CAliasASTNode* aliasNode = FindAlias(semanter, identifier, ignoreNode);
	if (aliasNode != NULL)
	{
		return aliasNode;
	}

	// Find normal declarations.
	std::vector<CASTNode*>& nodes = GetSearchScopeChildren(semanter);
	for (std::vector<CASTNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		CDeclarationASTNode* node = dynamic_cast<CDeclarationASTNode*>(*iter);
		if (node != NULL && node != ignoreNode)
		{
	//		DBG_LOG("\tChecking %s", node->Identifier.Get_String().c_str());
			if (node->Identifier == identifier)
			{
				return node;
			}
		}
	}

	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindDeclaration(semanter, identifier, ignoreNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds the declaration in the scope stack with the given
//  identifier.
// =================================================================
CDeclarationASTNode* CASTNode::FindScopedDeclaration(CSemanter* semanter, 
											   CDeclarationIdentifier identifier,
											   CASTNode* ignoreNode)
{
	CAliasASTNode* aliasNode = FindAlias(semanter, identifier, ignoreNode);
	if (aliasNode != NULL)
	{
		return aliasNode;
	}

	// Find normal declarations.
	std::vector<CASTNode*>& nodes = GetSearchScopeChildren(semanter);
	for (std::vector<CASTNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		CDeclarationASTNode* node = dynamic_cast<CDeclarationASTNode*>(*iter);
		if (node != NULL && node != ignoreNode)
		{
			if (node->Identifier == identifier)
			{
				return node;
			}
		}
	}

	return NULL;
}

// =================================================================
//	Finds an alias declaration.
// =================================================================
CAliasASTNode* CASTNode::FindAlias(CSemanter* semanter, 
								   CDeclarationIdentifier identifier,
								   CASTNode* ignoreNode)
{
	// Find normal declarations.
	std::vector<CAliasASTNode*>& nodes = GetSearchScopeAliasChildren(semanter);
	for (std::vector<CAliasASTNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		CAliasASTNode* node = *iter;
		if (node != NULL && node != ignoreNode)
		{
			if (node->Identifier == identifier)
			{
				return node;
			}
		}
	}
	
	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindAlias(semanter, identifier, ignoreNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds a class method with the given arguments.
// =================================================================
CClassMemberASTNode* CASTNode::FindClassMethod(CSemanter* semanter, 
											   CDeclarationIdentifier identifier, 
											   std::vector<CDataType*> arguments, 
											   bool explicit_arguments,
											   CASTNode* ignoreNode, 
											   CASTNode* referenceNode)
{
	// Real work is done inside the derived method in CClassASTNode

	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindClassMethod(semanter, identifier, arguments, explicit_arguments, ignoreNode, referenceNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds a class property with the given arguments.
// =================================================================
CClassPropertyASTNode* CASTNode::FindClassProperty(CSemanter* semanter, 
											   CDeclarationIdentifier identifier, 
											   CASTNode* ignoreNode, 
											   CASTNode* referenceNode)
{
	// Real work is done inside the derived method in CClassASTNode

	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindClassProperty(semanter, identifier, ignoreNode, referenceNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds a class field.
// =================================================================
CClassMemberASTNode* CASTNode::FindClassField(CSemanter* semanter, 
											  CDeclarationIdentifier identifier, 
											  CASTNode* ignoreNode, 
											  CASTNode* referenceNode)
{
	// Real work is done inside the derived method in CClassASTNode

	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindClassField(semanter, identifier, ignoreNode, referenceNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds a class state.
// =================================================================
CClassStateASTNode* CASTNode::FindClassState(CSemanter* semanter, 
											  CDeclarationIdentifier identifier, 
											  CASTNode* ignoreNode, 
											  CASTNode* referenceNode)
{
	// Real work is done inside the derived method in CClassASTNode

	CASTNode* next_scope = GetParentSearchScope(semanter);
	if (next_scope != NULL)
	{
		return next_scope->FindClassState(semanter, identifier, ignoreNode, referenceNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Gets the next scope up the tree to check when looking for
//	declarations.
// =================================================================
CASTNode* CASTNode::GetParentSearchScope(CSemanter* semanter)
{
	return Parent;
}

// =================================================================
//	Gets the list of children to be searched when looking for
//	declarations.
// =================================================================
std::vector<CASTNode*>& CASTNode::GetSearchScopeChildren(CSemanter* semanter)
{
	return Children;
}

std::vector<CAliasASTNode*>& CASTNode::GetSearchScopeAliasChildren(CSemanter* semanter)
{
	return AliasChildren;
}

// =================================================================
//	Check for duplicate identifier.
// =================================================================
void CASTNode::CheckForDuplicateIdentifier(CSemanter* semanter, 
										   CDeclarationIdentifier identifier,
										   CASTNode* ignore_node)
{
	if (ignore_node == NULL)
	{
		ignore_node = this;
	}

	CASTNode* duplicate = FindDeclaration(semanter, identifier, ignore_node);
	if (duplicate != NULL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Encountered duplicate identifier '%s'.", identifier.c_str()), Token);
	}
}

// =================================================================
//	Check for duplicate identifier.
// =================================================================
void CASTNode::CheckForDuplicateScopedIdentifier(CSemanter* semanter, 
										   CDeclarationIdentifier identifier,
										   CASTNode* ignore_node)
{
	if (ignore_node == NULL)
	{
		ignore_node = this;
	}

	CASTNode* duplicate = FindScopedDeclaration(semanter, identifier, ignore_node);
	if (duplicate != NULL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Encountered duplicate identifier '%s'.", identifier.c_str()), Token);
	}
}

// =================================================================
//	Check for duplicate identifiers.
// =================================================================
void CASTNode::CheckForDuplicateMethodIdentifier(CSemanter* semanter, 
												 CDeclarationIdentifier identifier, 
												 std::vector<CDataType*> arguments,
												 CClassMemberASTNode* ignore_node)
{
	if (ignore_node == NULL)
	{
		semanter->GetContext()->FatalError("Internal error, no method specified when looking for duplicate identifier.", Token);	
	}

	CClassMemberASTNode* method = FindClassMethod(semanter, identifier, arguments, true, ignore_node, this);
	
	// If we are overriding and we haven't found a base method,
	// then wtf are we overriding?
	if (ignore_node->IsOverride == true && method == NULL)
	{ 
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Attempt to override unknown virtual method '%s'.", ignore_node->Identifier.c_str()), Token);
	}

	// If we are overriding this method, then a method lower
	// in the inheritance tree with the same name is valid.
	if (method					!= NULL	&&
		method->IsVirtual		== true &&
		ignore_node->IsVirtual	== true &&
		method->Parent			!= ignore_node->Parent &&
		(ignore_node->IsOverride == true || ignore_node->IsAbstract == true))
	{
		method = NULL;
	}
	
	if (method != NULL)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Encountered duplicate method '%s'.", ignore_node->Identifier.c_str()), Token);
	}
}

// =================================================================
//	Evalulates the constant value of this node.
// =================================================================
EvaluationResult CASTNode::Evaluate(CTranslationUnit* unit)
{
	unit->FatalError("Could not statically analyse node. Expression is not constant.", Token);
	return EvaluationResult(false);
}
