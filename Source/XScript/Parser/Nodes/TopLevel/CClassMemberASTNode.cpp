/* *****************************************************************

		CClassMemberASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#ifdef _WIN32
#include <Windows.h>
#endif

#include "XScript/Helpers/CStringHelper.h"
#include "XScript/Helpers/CCollectionHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CMethodBodyASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/Helper/CNullDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassBodyASTNode.h"

#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Assignment/CAssignmentExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CBaseExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"
#include "XScript/Parser/Nodes/Statements/CReturnStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Types/CLiteralExpressionASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"

#include "XScript/Semanter/CSemanter.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassMemberASTNode::CClassMemberASTNode(CASTNode* parent, CToken token) :
	CDeclarationASTNode(parent, token)
{
	AccessLevel		= AccessLevel::PUBLIC;
	IsStatic		= false;
	IsAbstract		= false;
	IsVirtual		= false;
	IsConst			= false;
	IsConstructor	= false;
	IsOverride		= false;
	IsExtension		= false;
	IsEvent			= false;
	IsRPC			= false;
	IsSerialized	= false;
	IsOffsetForced	= false;
	SerializeVersion = 0;
	MemberType		= MemberType::Method;

	Body			= NULL;
	Assignment		= NULL;
	ReturnType		= NULL;

	BaseVirtualMethod = NULL;
}

// =================================================================
//	Destructor.
// =================================================================
CClassMemberASTNode::~CClassMemberASTNode()
{
}
	
// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CClassMemberASTNode::ToString()
{
	String val = ReturnType->ToString();

	if (MemberType == MemberType::Field)
	{
		if (IsStatic == true)
		{
			val = "static " + val;
		}
		if (IsConst == true)
		{
			val = "const " + val;
		}

		val += " " + Identifier.Get_String();
	}
	else if (MemberType == MemberType::Method)
	{
		if (IsStatic == true)
		{
			val = "static " + val;
		}
		if (IsAbstract == true)
		{
			val = "abstract " + val;
		}
		if (IsVirtual == true)
		{
			val = "virtual " + val;
		}
		if (IsOverride == true)
		{
			val = "override " + val;
		}
		if (IsEvent == true)
		{
			val = "event " + val;
		}
		if (IsRPC == true)
		{
			val = "rpc " + val;
		}

		val += " " + Identifier.Get_String();
		val += "(";

		for (std::vector<CVariableStatementASTNode*>::iterator iter = Arguments.begin(); iter != Arguments.end(); iter++)
		{
			if (iter != Arguments.begin())
			{
				val += ", ";
			}
			val += (*iter)->ToString();
		}

		val += ")";
	}

	return val;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassMemberASTNode::Semant(CSemanter* semanter)
{
	SEMANT_TRACE("CClassMemberASTNode=%s", Identifier.c_str());

	// Only semant once.
	if (Semanted == true)
	{
		return this;
	}
	Semanted = true;

	CClassASTNode* classScope = FindClassScope(semanter);

	// If we are constructor, replace return type with us.
	if (IsConstructor == true &&
		IsStatic == false &&
		classScope->InstanceConstructor != this)
	{
		ReturnType = FindClassScope(semanter)->ObjectDataType;
	}

	// Semant the type/return type of this member.
//	CDataType* original = ReturnType;
	ReturnType = ReturnType->Semant(semanter, this);
	
	// No assignment? Add default.
	if (Assignment == NULL && MemberType == MemberType::Field)
	{
		// Add default assignment expression.
		Assignment = semanter->ConstructDefaultAssignmentExpr(this, Token, ReturnType);
	}

	// Semant arguments.	
	std::vector<CDataType*> argument_types;
	for (std::vector<CVariableStatementASTNode*>::iterator iter = Arguments.begin(); iter != Arguments.end(); iter++)
	{
		CVariableStatementASTNode* arg = (*iter);
		arg->Semant(semanter);

		argument_types.push_back(arg->Type);

		if (dynamic_cast<CVoidDataType*>(arg->Type) != NULL)
		{
			semanter->GetContext()->FatalError("Methods arguments cannot be of type void.", Token);
		}
	}
	
	// Check for duplicate identifiers.
	if (MemberType == MemberType::Field)
	{
		if (classScope->IsEnum)
		{
			CheckForDuplicateScopedIdentifier(semanter, Identifier);
		}
		else
		{
			CheckForDuplicateIdentifier(semanter, Identifier);
		}
	}
	else
	{
		CheckForDuplicateMethodIdentifier(semanter, Identifier, argument_types, this);
	}

	// Find and store base method we are overriding.
	if (MemberType == MemberType::Method && IsOverride == true)
	{
		BaseVirtualMethod = FindClassMethod(semanter, Identifier, argument_types, true, this);
		if (BaseVirtualMethod == NULL)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Attempt to override unknown virtual method '%s'.", Identifier.c_str()), Token);
		}
	}

	// If RPC, make sure return value is void.
	if (IsRPC == true && !ReturnType->IsEqualTo(semanter, CVoidDataType::StaticInstance))
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("RPC Method '%s' has non-void return value. RPC method cannot return values.", Identifier.c_str()), Token);	
	}

	// If we are a class or instance constructor then we need to
	// add some stub code to the start.
	if (IsConstructor == true)
	{
		if (IsStatic == true)
		{
			AddClassConstructorStub(semanter);
		}
		else if (IsNative == false)
		{
			if (classScope->InstanceConstructor == this)
			{
				AddInstanceConstructorStub(semanter);
			}
			else
			{
				AddInstanceConstructorPrefix(semanter);
			}
		}
	}
	else
	{
		AddMethodConstructorStub(semanter);
	}

	// Component constructor?
	if (classScope->ComponentConstructor == this)
	{
		AddComponentConstructorPrefix(semanter);
	}

	// Semant the body.
	if (Body != NULL)
	{
		Body->Semant(semanter);
	}
	
	// If we are a class or instance constructor then we need to
	// add some stub code to the end.
	if (IsConstructor == true)
	{
		if (IsStatic == false && IsNative == false)
		{
			if (classScope->InstanceConstructor != this)
			{
				AddInstanceConstructorPostfix(semanter);
			}
		}
	}

	// We need to make sure to return a value.
	if (MemberType == MemberType::Method &&
		IsNative == false)
	{
		AddDefaultReturnExpression(semanter);
	}

	return this;
}

// =================================================================
//	Returns true if this member is equal to another. Usually used
//	to test if methods are equal to one another.
// =================================================================
bool CClassMemberASTNode::EqualToMember(CSemanter* semanter, CClassMemberASTNode* other)
{
	if (MemberType != other->MemberType)
	{
		return false;
	}
	if (Identifier != other->Identifier)
	{
		return false;
	}
	if (!ReturnType->IsEqualTo(semanter, other->ReturnType))
	{
		return false;
	}

	if (MemberType == MemberType::Method)
	{
		if (Arguments.size() != other->Arguments.size())
		{
			return false;
		}

		for (unsigned int i = 0; i < Arguments.size(); i++)
		{
			CVariableStatementASTNode* arg1 = Arguments.at(i);
			CVariableStatementASTNode* arg2 = other->Arguments.at(i);

			if (!arg1->Type->IsEqualTo(semanter, arg2->Type))
			{
				return false;
			}
		}
	}

	return true;
}

// =================================================================
//	Adds a return expression to the end of a method if required.
// =================================================================
void CClassMemberASTNode::AddDefaultReturnExpression(CSemanter* semanter)
{
	// Make sure we don't already end with a return statement.
	if (Body == NULL || (Body->Children.size() >= 1 && dynamic_cast<CReturnStatementASTNode*>(Body->Children.at(Body->Children.size() - 1)) != NULL))
	{
		return;
	}

	// Add default return statement.  
	CReturnStatementASTNode* node = new CReturnStatementASTNode(Body, Token);
	node->ReturnExpression = semanter->ConstructDefaultAssignmentExpr(node, Token, ReturnType);
	node->Semant(semanter);
}

// =================================================================
//	Adds variable declarations to start of method.
// =================================================================
void CClassMemberASTNode::AddMethodConstructorStub(CSemanter* semanter)	
{

}

// =================================================================
//	Adds the class-constructor stub code to this method.
// =================================================================
void CClassMemberASTNode::AddClassConstructorStub(CSemanter* semanter)
{
	CClassASTNode* classScope = FindClassScope(semanter);

	// Find all fields with assignment expressions
	// and add their assignment to this constructor.
	for (std::vector<CASTNode*>::iterator iter = classScope->Body->Children.begin(); iter != classScope->Body->Children.end(); iter++)
	{
		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
		if (member != NULL)
		{
			if (member->IsStatic   == true && 
				member->MemberType == MemberType::Field &&
				member->IsNative   == false)
			{
				if (member->Assignment == NULL)
				{
					member->Semant(semanter);
				}

				CToken op	= member->Assignment->Token;
				op.Type		= TokenIdentifier::OP_ASSIGN;
				op.Literal	= "=";
				
				CToken identTok	 = member->Assignment->Token;
				identTok.Type	 = TokenIdentifier::IDENTIFIER;
				identTok.Literal = member->Identifier.Get_String();
				
				CIdentifierExpressionASTNode* ident = new CIdentifierExpressionASTNode(NULL, identTok);
				
				CAssignmentExpressionASTNode* assignment = new CAssignmentExpressionASTNode(NULL, op);
				assignment->LeftValue  = ident;
				assignment->IgnoreConst = true;
				assignment->RightValue = member->Assignment;//->Clone(semanter);
				assignment->AddChild(assignment->RightValue);
				assignment->AddChild(ident);

				CExpressionASTNode* expr = new CExpressionASTNode(NULL, member->Assignment->Token);
				expr->LeftValue = assignment;
				expr->AddChild(assignment);

				Body->AddChild(expr);
			}
		}
	}

}

// =================================================================
//	Adds the instance-constructor stub code to this method.
// =================================================================
void CClassMemberASTNode::AddInstanceConstructorStub(CSemanter* semanter)
{
	CClassASTNode* classScope = FindClassScope(semanter);
	
	// Find all fields with assignment expressions
	// and add their assignment to this constructor.
	for (std::vector<CASTNode*>::reverse_iterator iter = classScope->Body->Children.rbegin(); iter != classScope->Body->Children.rend(); iter++)
	{
		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
		if (member != NULL)
		{
			if (member->IsStatic   == false && 
				member->MemberType == MemberType::Field &&
				member->IsNative   == false)
			{
				if (member->Assignment == NULL)
				{
					member->Semant(semanter);
				}

				CToken op	= member->Assignment->Token;
				op.Type		= TokenIdentifier::OP_ASSIGN;
				op.Literal	= "=";
				
				CToken identTok	 = member->Assignment->Token;
				identTok.Type	 = TokenIdentifier::IDENTIFIER;
				identTok.Literal = member->Identifier.Get_String();
				
				CIdentifierExpressionASTNode* ident = new CIdentifierExpressionASTNode(NULL, identTok);
				
				CAssignmentExpressionASTNode* assignment = new CAssignmentExpressionASTNode(NULL, op);
				assignment->LeftValue  = ident;
				assignment->IgnoreConst = true;
				assignment->RightValue = member->Assignment;//->Clone(semanter);
				assignment->AddChild(assignment->RightValue);
				assignment->AddChild(ident);

				CExpressionASTNode* expr = new CExpressionASTNode(NULL, member->Assignment->Token);
				expr->LeftValue = assignment;
				expr->AddChild(assignment);

				Body->AddChild(expr, true);
			}
		}
	}
}

// =================================================================
//	Adds the component-constructor stub code to this method.
// =================================================================
void CClassMemberASTNode::AddComponentConstructorPrefix(CSemanter* semanter)
{
}

// =================================================================
//	Adds the class-constructor stub code to this method.
// =================================================================
void CClassMemberASTNode::AddInstanceConstructorPrefix(CSemanter* semanter)
{
	CClassASTNode* classScope = FindClassScope(semanter);

	//if (classScope->IsEnum == false && classScope->IsNative == false && classScope->IsGeneric == false && classScope->IsStatic == false && classScope->InstanceConstructor == NULL)
	//{
	//	printf("WTF!");
	//}

	// Call components constructor.
	if (classScope->ComponentConstructor != NULL)
	{	
		CToken identTok	 = classScope->ComponentConstructor->Token;
		identTok.Type	 = TokenIdentifier::IDENTIFIER;
		identTok.Literal = classScope->ComponentConstructor->Identifier.Get_String();

		CMethodCallExpressionASTNode* call = new CMethodCallExpressionASTNode(NULL, identTok);
		call->LeftValue = new CThisExpressionASTNode(NULL, identTok);
		call->RightValue = new CIdentifierExpressionASTNode(NULL, identTok);
		call->AddChild(call->LeftValue);
		call->AddChild(call->RightValue);

		CExpressionASTNode* expr = new CExpressionASTNode(NULL, identTok);
		expr->LeftValue = call;
		expr->AddChild(call);

		Body->AddChild(expr, true);
	}

	// Call instance constructor.
	if (classScope->InstanceConstructor != NULL)
	{	
		CToken identTok	 = classScope->InstanceConstructor->Token;
		identTok.Type	 = TokenIdentifier::IDENTIFIER;
		identTok.Literal = classScope->InstanceConstructor->Identifier.Get_String();

		CMethodCallExpressionASTNode* call = new CMethodCallExpressionASTNode(NULL, identTok);
		call->LeftValue = new CThisExpressionASTNode(NULL, identTok);
		call->RightValue = new CIdentifierExpressionASTNode(NULL, identTok);
		call->AddChild(call->LeftValue);
		call->AddChild(call->RightValue);

		CExpressionASTNode* expr = new CExpressionASTNode(NULL, identTok);
		expr->LeftValue = call;
		expr->AddChild(call);

		Body->AddChild(expr, true);
	}

	// Now we add a call to the superclass's constructor.
	if (classScope->SuperClass != NULL)
	{	
		CClassMemberASTNode* baseConstructor = classScope->SuperClass->FindClassMethod(semanter, classScope->SuperClass->Identifier, std::vector<CDataType*>(), false);
	//	if (classScope->SuperClass->IsNative == false)
	//	{
			CToken identTok	 = baseConstructor->Token;
			identTok.Type	 = TokenIdentifier::IDENTIFIER;
			identTok.Literal = baseConstructor->Identifier.Get_String();

			CMethodCallExpressionASTNode* call = new CMethodCallExpressionASTNode(NULL, identTok);
			call->LeftValue = new CBaseExpressionASTNode(NULL, identTok);
			call->RightValue = new CIdentifierExpressionASTNode(NULL, identTok);
			call->AddChild(call->LeftValue);
			call->AddChild(call->RightValue);

			CExpressionASTNode* expr = new CExpressionASTNode(NULL, identTok);
			expr->LeftValue = call;
			expr->AddChild(call);

			Body->AddChild(expr, true);
	//	}
	}
}

// =================================================================
//	Adds the class-constructor stub code to this method.
// =================================================================
void CClassMemberASTNode::AddInstanceConstructorPostfix(CSemanter* semanter)
{
	// Return the class instance.
	CReturnStatementASTNode* ret = new CReturnStatementASTNode(NULL, Token);
	ret->ReturnExpression = new CThisExpressionASTNode(NULL, Token);
	ret->ReturnExpression->ExpressionResultType = ReturnType;

	ret->AddChild(ret->ReturnExpression);
	Body->AddChild(ret);
}

// =================================================================
//	Performs finalization on this member.
// =================================================================
CASTNode* CClassMemberASTNode::Finalize(CSemanter* semanter)
{
	// Work out mangled identifier.
	/*
	if (MangledIdentifier == "")
	{
		CClassASTNode* scope = FindClassScope(semanter);
		
		if (MemberType == MemberType::Field)
		{
			if (IsExtension == false)
			{
				MangledIdentifier = semanter->GetMangled("ls_f" + Identifier);
			}
			else
			{
				MangledIdentifier = semanter->GetMangled(scope->MangledIdentifier + "_f" + Identifier);
			}
		}
		else
		{
			if (IsExtension == false)
			{
				if (scope->IsInterface == true)
				{
					MangledIdentifier = semanter->GetMangled("ls_" + scope->Identifier + "_" + Identifier);
				}
				else
				{
					MangledIdentifier = semanter->GetMangled("ls_" + Identifier);
				}
			}
			else
			{
				MangledIdentifier = semanter->GetMangled(scope->MangledIdentifier + "_" + Identifier);
			}
		}
	}

	// Look for overridden mangled identifiers for virtuals / interfaces.
	if (IsExtension == false)
	{

		// Create a list of arguments.
		std::vector<CDataType*> types;
		for (std::vector<CVariableStatementASTNode*>::iterator iter = Arguments.begin(); iter != Arguments.end(); iter++)
		{
			types.push_back((*iter)->Type);
		}

		// If we are overriding then lets use the mangled identifier of our override.
		CClassASTNode* scope = FindClassScope(semanter);
		while (scope != NULL)
		{
			CClassMemberASTNode* node = scope->FindClassMethod(semanter, Identifier, types, true, this, this);
			if (node != NULL && 
				((IsOverride == true && node->IsVirtual == true) || scope->IsInterface == true))
			{
				if (node->MangledIdentifier == "")
				{
					node->Finalize(semanter);
				}
				MangledIdentifier = node->MangledIdentifier;
			}

			for (std::vector<CClassASTNode*>::iterator iter = scope->Interfaces.begin(); iter != scope->Interfaces.end(); iter++)
			{
				CClassASTNode* interf = *iter;		
				CClassMemberASTNode* interf_node = interf->FindClassMethod(semanter, Identifier, types, true, this, this);
				if (interf_node != NULL)
				{
					if (interf_node->MangledIdentifier == "")
					{
						interf_node->Finalize(semanter);
					}
					MangledIdentifier = interf_node->MangledIdentifier;
				}
			}

			scope = scope->SuperClass;
		}

	}
	*/
	
	FinalizeChildren(semanter);
	return this;
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassMemberASTNode::Clone(CSemanter* semanter)
{	
	CClassMemberASTNode* clone = new CClassMemberASTNode(NULL, Token);
	
	clone->IsNative			 = this->IsNative;
	clone->MangledIdentifier = this->MangledIdentifier;
	clone->Identifier		 = this->Identifier;
	clone->AccessLevel		 = this->AccessLevel;
	clone->IsStatic			 = this->IsStatic;
	clone->IsAbstract		 = this->IsAbstract;
	clone->IsVirtual		 = this->IsVirtual;
	clone->IsOverride		 = this->IsOverride;
	clone->IsConst			 = this->IsConst;
	clone->IsExtension		 = this->IsExtension;
	clone->IsEvent			 = this->IsEvent;
	clone->IsRPC			 = this->IsRPC;
	clone->MemberType		 = this->MemberType;
	clone->ReturnType		 = this->ReturnType;
	clone->IsConstructor	 = this->IsConstructor;	
	clone->IsSerialized		 = this->IsSerialized;
	clone->SerializeVersion	 = this->SerializeVersion;
	
	for (std::vector<CVariableStatementASTNode*>::iterator iter = Arguments.begin(); iter != Arguments.end(); iter++)
	{
		CVariableStatementASTNode* arg = dynamic_cast<CVariableStatementASTNode*>((*iter)->Clone(semanter));
		clone->Arguments.push_back(arg);
		clone->AddChild(arg);
	}

	if (Body != NULL)
	{
		clone->Body				 = dynamic_cast<CMethodBodyASTNode*>(this->Body->Clone(semanter));
		clone->AddChild(clone->Body);
	}

	if (Assignment != NULL)
	{
		clone->Assignment		 = dynamic_cast<CExpressionASTNode*>(this->Assignment->Clone(semanter));
		clone->AddChild(clone->Assignment);
	}

	return clone;
}

// =================================================================
//	Checks if we can access this declaration from the given node.
// =================================================================
void CClassMemberASTNode::CheckAccess(CSemanter* semanter, CASTNode* referenceBy)
{
	CClassASTNode* ourClass				= FindClassScope(semanter);//dynamic_cast<CClassASTNode*>(Parent->Parent);
	CClassASTNode* referenceClass		= referenceBy->FindClassScope(semanter);	
	
	// If we are in a different package and our class is private, then refuse access.
	if (ourClass->Token.SourceFile != referenceClass->Token.SourceFile && 
		ourClass->AccessLevel != AccessLevel::PUBLIC)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Class member '%s' is not accessible from this package.", Identifier.c_str()), referenceBy->Token);	
	}

	// If the class we are referenced in extends from the class this member is in.
	if (referenceClass->InheritsFromClass(semanter, ourClass))
	{
		// Same class then we have access to everything.
		if (referenceClass == ourClass)
		{
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//			 TODO: Fix all of below, its ugly, hacky and just plain horrible.
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

			CClassMemberASTNode* methodScope = referenceBy->FindClassMethodScope(semanter);
		
			// If we are the instance constructor we need to check if we are accessing variables
			// before they are declared.
			if (methodScope == ourClass->InstanceConstructor)
			{
				// See if we are the l-value of an assignment, in which case we don't give a shit 
				// about the member.
				bool lValueAssignment = false;
				CASTNode* lastScope = referenceBy;
				CASTNode* scope = lastScope->Parent;
				while (scope != NULL)
				{
					CAssignmentExpressionASTNode* assignment = dynamic_cast<CAssignmentExpressionASTNode*>(scope);
					if (assignment != NULL)
					{
						lValueAssignment = (assignment->LeftValue == lastScope);
						break;
					}
					lastScope = scope;
					scope = scope->Parent;
				}

				if (lValueAssignment == false)
				{
					CASTNode* referenceParent = referenceBy;
					while (referenceParent != NULL &&
						   referenceParent->Parent != methodScope->Body)
					{
						referenceParent = referenceParent->Parent;
					}

					// Check assignment index and member index, which *should* have a one-to-one mapping.
					// Using this we can determine if we are assigning member before its declared.
					int indexOfAssignmentExpression = CCollectionHelper::VectorIndexOf<CASTNode*>(methodScope->Body->Children, referenceParent);
					int indexOfMemberDeclaration    = CCollectionHelper::VectorIndexOf<CASTNode*>(ourClass->Body->Children, this);
				
					if (indexOfAssignmentExpression <= indexOfMemberDeclaration)
					{
						semanter->GetContext()->FatalError(CStringHelper::FormatString("Attempt to access variable '%s' before it is declared.", Identifier.c_str()), referenceBy->Token);	
					}
				}
			}

			return;
		}

		// Otherwise we only have access to public and protected variables.
		else
		{
			if (this->AccessLevel != AccessLevel::PUBLIC &&
				this->AccessLevel != AccessLevel::PROTECTED)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Class member '%s' is not accessible from here.", Identifier.c_str()), referenceBy->Token);	
			}
		}
	}

	// Otherwise we are accessing class from outside its inheritance tree so check if its public,
	// if not then refuse access.
	else
	{
		if (this->AccessLevel != AccessLevel::PUBLIC)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Class member '%s' is not accessible from here.", Identifier.c_str()), referenceBy->Token);	
		}
	}
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CClassMemberASTNode::Translate(CTranslator* translator)
{
	translator->TranslateClassMember(this);
}