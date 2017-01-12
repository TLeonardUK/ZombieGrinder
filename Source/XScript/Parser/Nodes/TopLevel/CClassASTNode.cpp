/* *****************************************************************

		CClassASTNode.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#ifdef _WIN32
#include <Windows.h>
#endif

#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Types/Helper/CIdentifierDataType.h"
#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Parser/Nodes/TopLevel/CDeclarationASTNode.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Nodes/TopLevel/CPackageASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CAliasASTNode.h"
#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"
#include "XScript/Parser/Nodes/TopLevel/CClassPropertyASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CMethodBodyASTNode.h"
#include "XScript/Parser/Types/CVoidDataType.h"

#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassStateASTNode.h"
#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassBodyASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassReplicationASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Types/CCastExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Values/CConstDictionaryASTNode.h"

#include "XScript/Translator/CTranslator.h"

// =================================================================
//	Constructs a new instance of this class.
// =================================================================
CClassASTNode::CClassASTNode(CASTNode* parent, CToken token) :
	CDeclarationASTNode(parent, token)
{
	Identifier				= "";
	AccessLevel				= AccessLevel::PUBLIC;
	IsStatic				= false;
	IsAbstract				= false;
	IsInterface				= false;
	IsGeneric				= false;
	IsSealed				= false;
	IsReplicated			= false;
	IsStruct				= false;
	ReplicationNode			= NULL;
	Body					= NULL;
	SuperClass				= NULL;
	InheritsNull			= false;
	ObjectDataType			= new CObjectDataType(token, this);
	ClassReferenceDataType	= new CClassReferenceDataType(token, this);
	m_semanting				= false;
	GenericInstanceOf		= NULL;
	ClassConstructor		= NULL;
	InstanceConstructor		= NULL;
	IsInstanced				= false;
	InstancedBy				= NULL;
	HasBoxClass				= false;
	BoxClassIdentifier		= "";
	IsEnum					= false;
	DefaultState			= NULL;
	ComponentConstructor	= NULL;
}

// =================================================================
//	Destructor.
// =================================================================
CClassASTNode::~CClassASTNode()
{
//	for (std::vector<CClassASTNode*>::iterator iter = GenericInstances.begin(); iter != GenericInstances.end(); iter++)
//	{
//		SAFE_DELETE(*iter);
//	}
//	GenericInstances.clear();
	
	SAFE_DELETE(ObjectDataType);
}

// =================================================================
//	Creates a clone of this node.
// =================================================================
CASTNode* CClassASTNode::Clone(CSemanter* semanter)
{
	CClassASTNode* clone = new CClassASTNode(NULL, Token);
	
	//clone->MangledIdentifier = this->MangledIdentifier;
	clone->IsNative			 = this->IsNative;
	clone->MangledIdentifier = this->MangledIdentifier;
	clone->Identifier		 = this->Identifier;
	clone->AccessLevel		 = this->AccessLevel;
	clone->IsStatic			 = this->IsStatic;
	clone->IsAbstract		 = this->IsAbstract;
	clone->IsInterface		 = this->IsInterface;
	clone->IsSealed			 = this->IsSealed;
	clone->IsGeneric		 = this->IsGeneric;
	clone->IsStruct			 = this->IsStruct;
	clone->IsReplicated		 = this->IsReplicated;
	clone->InheritsNull		 = this->InheritsNull;
	clone->GenericTypeTokens = this->GenericTypeTokens;
	clone->InheritedTypes	 = this->InheritedTypes;	
	clone->HasBoxClass		 = this->HasBoxClass;
	clone->BoxClassIdentifier = this->BoxClassIdentifier;
	clone->IsEnum			 = this->IsEnum;
	clone->Body				 = dynamic_cast<CClassBodyASTNode*>(this->Body->Clone(semanter));
	clone->ObjectDataType	 = new CObjectDataType(Token, clone);
	clone->AddChild(clone->Body);

	if (ReplicationNode != NULL)
	{	
		clone->ReplicationNode = dynamic_cast<CClassReplicationASTNode*>(this->ReplicationNode->Clone(semanter));
		clone->AddChild(clone->ReplicationNode);
	}

	if (ClassConstructor != NULL)
	{
		for (std::vector<CASTNode*>::iterator iter = clone->Body->Children.begin(); iter != clone->Body->Children.end(); iter++)
		{
			CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
			if (member != NULL)
			{
				if (member->Identifier == ClassConstructor->Identifier)
				{
					clone->ClassConstructor = member;
					break;
				}
			}
		}
	}
	
	if (InstanceConstructor != NULL)
	{
		for (std::vector<CASTNode*>::iterator iter = clone->Body->Children.begin(); iter != clone->Body->Children.end(); iter++)
		{
			CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
			if (member != NULL)
			{
				if (member->Identifier == InstanceConstructor->Identifier)
				{
					clone->InstanceConstructor = member;
					break;
				}
			}
		}
	}
	
	if (ComponentConstructor != NULL)
	{
		for (std::vector<CASTNode*>::iterator iter = clone->Body->Children.begin(); iter != clone->Body->Children.end(); iter++)
		{
			CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
			if (member != NULL)
			{
				if (member->Identifier == ComponentConstructor->Identifier)
				{
					clone->ComponentConstructor = member;
					break;
				}
			}
		}
	}

	// Add alias componenets.
	for (std::vector<CASTNode*>::iterator iter = clone->Body->Children.begin(); iter != clone->Body->Children.end(); iter++)
	{
		CAliasASTNode* member = dynamic_cast<CAliasASTNode*>(*iter);
		if (member != NULL)
		{
			clone->Body->AliasChildren.push_back(member);
		}
	}

	return clone;
}

// =================================================================
//	Converts this node to a string representation.
// =================================================================
String CClassASTNode::ToString()
{
	String result = Identifier.c_str();

	if (IsGeneric == true)
	{
		result += "<";
		if (GenericInstanceOf != NULL)
		{
			for (std::vector<CDataType*>::iterator iter = GenericInstanceTypes.begin(); iter != GenericInstanceTypes.end(); iter++)
			{
				if (iter != GenericInstanceTypes.begin())
				{
					result += ",";
				}
				result += (*iter)->ToString();
			}
		}
		else
		{
			for (std::vector<CToken>::iterator iter = GenericTypeTokens.begin(); iter != GenericTypeTokens.end(); iter++)
			{
				if (iter != GenericTypeTokens.begin())
				{
					result += ",";
				}
				result += (*iter).Literal;
			}
		}
		result += ">";
	}

	return result;
}

// =================================================================
//	Performs semantic analysis on this node.
// =================================================================
CASTNode* CClassASTNode::Semant(CSemanter* semanter)
{ 
	SEMANT_TRACE("CClassASTNode=%s", Identifier.c_str());

	// Only semant once.
	if (Semanted == true)
	{
		//m_semanting = false;
		return this;
	}
	Semanted = true;
	
	// Check for duplicate identifiers (only if we are not an instanced class).
	if (GenericInstanceOf == NULL)
	{
		Parent->CheckForDuplicateIdentifier(semanter, Identifier, this);
	}
	
	if (IsGeneric		  == false ||
		GenericInstanceOf != NULL)
	{
	
		// Work out mangled identifier.
		////if (MangledIdentifier == "")
		//{
		//	MangledIdentifier = semanter->GetMangled("ls_" + Identifier);
		//}

		// Interface cannot use inheritance.
		if (InheritedTypes.size() > 0 && IsInterface == true)
		{
			semanter->GetContext()->FatalError("Interfaces cannot inherit from other interfaces or classes.", Token);
		}		
		if (InheritedTypes.size() > 0 && IsStatic == true)
		{
			semanter->GetContext()->FatalError("Static classes cannot inherit from interfaces.", Token);
		}
		
		// Flag this class as semanting - we do this so we can detect
		// inheritance loops.
		if (m_semanting == true)
		{
			semanter->GetContext()->FatalError("Detected illegal cyclic inheritance of '" + Identifier.Get_String() + "'.", Token);
		}
		m_semanting = true;
	
		// Semant inherited types.
		bool foundSuper = false;
		for (std::vector<CIdentifierDataType*>::iterator iter = InheritedTypes.begin(); iter != InheritedTypes.end(); iter++)
		{
			CIdentifierDataType* type = *iter;
			CClassASTNode* node = type->SemantAsClass(semanter, this, true);

			if (type->Identifier == Identifier)
			{
				semanter->GetContext()->FatalError("Attempt to inherit class from itself.", Token);
			}

			if (node->IsInterface == true)
			{
				Interfaces.push_back(node);
			}
			else
			{
				if (foundSuper == true)
				{
					semanter->GetContext()->FatalError("Multiple inheritance is not supported. Use interfaces instead.", Token);
				}
				SuperClass = node;
				foundSuper = true;
			}
		}

		// Native classes are not allowed to implement interfaces.
		//if (IsNative == true && Interfaces.size() > 0)
		//{
		//	semanter->GetContext()->FatalError("Native classes cannot implement interfaces.", Token);
		//}

		// If no inherited types the we inherit from object.
		if (SuperClass == NULL && InheritsNull == false)// IsNative == false)
		{
			CDeclarationIdentifier s_object_ident("object");

			SuperClass = dynamic_cast<CClassASTNode*>(FindDeclaration(semanter, s_object_ident));

			if (SuperClass == NULL)
			{
				semanter->GetContext()->FatalError("Could not find base class to inherit from.", Token);
			}
		}
		else if (SuperClass != NULL)
		{
			// Check super class is valid.
			if (SuperClass->IsSealed == true)
			{
				semanter->GetContext()->FatalError("Classes cannot inherit from sealed class.", Token);
			}
		
			// Cannot inherit in static classes.
			if (IsStatic == true)
			{
				semanter->GetContext()->FatalError("Static classes cannot inherit from other classes.", Token);
			}
		}
		
		// Semant inherited classes.
		if (SuperClass != NULL)
		{
			SuperClass->Semant(semanter);
		}
		for (std::vector<CClassASTNode*>::iterator iter = Interfaces.begin(); iter != Interfaces.end(); iter++)
		{
			CClassASTNode* interfaceClass = *iter;
			interfaceClass->Semant(semanter);
		}

		// Copy replicated state.
		if (SuperClass != NULL && SuperClass->IsReplicated == true)
		{
			IsReplicated = true;
			
			// Create a clone of the parent class.
			if (ReplicationNode == NULL)
			{
				ReplicationNode = static_cast<CClassReplicationASTNode*>(SuperClass->ReplicationNode->Clone(semanter));
				AddChild(ReplicationNode);
			}

			// Copy all of the super-classes replication elements.
			else
			{
				for (std::vector<CClassReplicationVariable>::iterator iter = SuperClass->ReplicationNode->Variables.begin(); iter != SuperClass->ReplicationNode->Variables.end(); iter++)
				{
					ReplicationNode->Variables.push_back(*iter);
				}
			}
		}
	
		// Look for interface in parent classes.
		if (SuperClass != NULL)
		{
			for (std::vector<CClassASTNode*>::iterator iter = Interfaces.begin(); iter != Interfaces.end(); iter++)
			{
				CClassASTNode* interfaceClass = *iter;
				if (SuperClass->InheritsFromClass(semanter, interfaceClass))
				{
					semanter->GetContext()->FatalError(CStringHelper::FormatString("Attempt to implement interface '%s' that is already implemented by a parent class.", interfaceClass->Identifier.c_str()), Token);
				}
			}
		}

		// Remove semanting flag.
		m_semanting = false;		
	}

	// If we are generic we only semant children of instanced classes.
	if (IsGeneric		  == false ||
		GenericInstanceOf != NULL)
	{
		// Create static class constructor.
		if (IsInterface == false)
		{
			String cc_identifier = "__"+Identifier.Get_String()+"_ClassConstructor";
			if (IsGeneric == true)
			{
				cc_identifier = "__"+Identifier.Get_String()+"_" + ClassReferenceDataType->ToString() + "_ClassConstructor";
			}

			CClassMemberASTNode* defaultCtor = FindClassMethod(semanter, cc_identifier, std::vector<CDataType*>(), false);
			if (defaultCtor == NULL)
			{
				CClassMemberASTNode* member		= new CClassMemberASTNode(NULL, Token);
				member->MemberType				= MemberType::Method;
				member->Identifier				= cc_identifier;
				member->AccessLevel				= AccessLevel::PUBLIC;
				member->Body					= new CMethodBodyASTNode(member, Token);
				member->IsConstructor			= true;
				member->IsStatic				= true;
				member->ReturnType				= new CVoidDataType(Token);
				member->IsExtension				= IsNative;
				Body->AddChild(member);

				ClassConstructor				= member; 
			}

			//if (IsNative == false && IsEnum == false)
			if (IsEnum == false)
			{
				String ic_identifier = "__"+Identifier.Get_String()+"_InstanceConstructor";
				if (IsGeneric == true)
				{
					ic_identifier = "__"+Identifier.Get_String()+"_" + ClassReferenceDataType->ToString() + "_InstanceConstructor";
				}

				// Create instance constructor.
				CClassMemberASTNode* instanceCtor = FindClassMethod(semanter, ic_identifier, std::vector<CDataType*>(), false);
				if (instanceCtor == NULL)
				{
					CClassMemberASTNode* member		= new CClassMemberASTNode(NULL, Token);
					member->MemberType				= MemberType::Method;
					member->Identifier				= ic_identifier;
					member->AccessLevel				= AccessLevel::PUBLIC;
					member->Body					= new CMethodBodyASTNode(member, Token);
					member->IsConstructor			= true;
					member->IsStatic				= false;
					member->ReturnType				= new CVoidDataType(Token);
					Body->AddChild(member);

					InstanceConstructor				= member; 
				}
			}

			// If no argument-less constructor has been provided, lets create a default one.
			if (IsStatic == false && IsAbstract == false && IsInterface == false && IsEnum == false) // && IsNative == false
			{
				CClassMemberASTNode* defaultCtor = FindClassMethod(semanter, Identifier, std::vector<CDataType*>(), false);
				if (defaultCtor == NULL)
				{
					CClassMemberASTNode* member = new CClassMemberASTNode(NULL, Token);
					member->MemberType			= MemberType::Method;
					member->Identifier			= Identifier;
					member->AccessLevel			= AccessLevel::PUBLIC;
					member->Body				= new CMethodBodyASTNode(member, Token);
					member->IsConstructor		= true;
					member->ReturnType			= new CVoidDataType(Token);
					Body->AddChild(member);
				}
			}
	
		}
		
		// Semant all members.
		SemantChildren(semanter);
	}

	return this;
}

// =================================================================
//	Performs finalization on this class.
// =================================================================
CASTNode* CClassASTNode::Finalize(CSemanter* semanter)
{
	// If we are generic, only finalize instances.
	if (IsGeneric == false || GenericInstanceOf != NULL)
	{
		// Check for hiding variables and methods.
		for (std::vector<CASTNode*>::iterator iter = Body->Children.begin(); iter != Body->Children.end(); iter++)
		{
			CClassMemberASTNode* node = dynamic_cast<CClassMemberASTNode*>(*iter);
			if (node != NULL)
			{
				CClassASTNode* scope = SuperClass;
				while (scope != NULL)
				{
					for (std::vector<CASTNode*>::iterator iter2 = scope->Body->Children.begin(); iter2 != scope->Body->Children.end(); iter2++)
					{
						CClassMemberASTNode* node2 = dynamic_cast<CClassMemberASTNode*>(*iter2);
						if (node2 != NULL &&
							node->Identifier == node2->Identifier &&
							(
								(node->IsOverride == false && node2->IsVirtual == true) ||
								(node->MemberType == MemberType::Field || node2->MemberType == MemberType::Field) 
							))
						{
							if (node->MemberType  == MemberType::Method ||
								node2->MemberType == MemberType::Method)
							{
								semanter->GetContext()->FatalError(CStringHelper::FormatString("Method '%s' in class '%s' hides existing declaration in class '%s'.", node->Identifier.c_str(), ToString().c_str(), scope->ToString().c_str()), node->Token);
							}
							else
							{
								semanter->GetContext()->FatalError(CStringHelper::FormatString("Member '%s' in class '%s' hides existing declaration in class '%s'.", node->Identifier.c_str(), ToString().c_str(), scope->ToString().c_str()), node->Token);
							}
						}
					}
					scope = scope->SuperClass;
				}			
			}
		}

		// Flag us as abstract if we have any abstract methods in our inheritance tree.	
		if (IsAbstract == false)
		{
			CClassASTNode* scope = this;
			std::vector<CClassMemberASTNode*> members;
			while (scope != NULL && IsAbstract == false)
			{
				// Look for abstract methods in this scope.		
				for (std::vector<CASTNode*>::iterator iter = scope->Body->Children.begin(); iter != scope->Body->Children.end() && IsAbstract == false; iter++)
				{
					CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
					if (member != NULL && member->MemberType == MemberType::Method)
					{
						// If member is abstract, check it is implemented in the members we have 
						// see higher in the inheritance tree so far.
						if (member->IsAbstract == true)
						{
							bool found = false;
							for (std::vector<CClassMemberASTNode*>::iterator iter2 = members.begin(); iter2 != members.end(); iter2++)
							{
								CClassMemberASTNode* sub_member = *iter2;
								
								if (sub_member->EqualToMember(semanter, member))
								{
									found = true;
									break;
								}
							}

							// If not found, this class is abstract!
							if (found == false)
							{
								if (IsInstanced == true)
								{
									semanter->GetContext()->FatalError(CStringHelper::FormatString("Cannot instantiate abstract class '%s'.", ToString().c_str()), InstancedBy->Token);
								}
								IsAbstract = true;
							}
						}
						else
						{
							members.push_back(member);
						}
					}
				}

				// Move up the inheritance tree.
				scope = scope->SuperClass;
			}
		}

		// Throw errors if we do not implement all interface functions.	
		for (std::vector<CClassASTNode*>::iterator iter = Interfaces.begin(); iter != Interfaces.end(); iter++)
		{
			CClassASTNode* interfaceClass = *iter;
			for (std::vector<CASTNode*>::iterator iter2 = interfaceClass->Body->Children.begin(); iter2 != interfaceClass->Body->Children.end(); iter2++)
			{
				CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter2);
				if (member != NULL &&
					member->MemberType == MemberType::Method)
				{
					std::vector<CDataType*> argument_data_types;
					for (std::vector<CVariableStatementASTNode*>::iterator iter3 = member->Arguments.begin(); iter3 != member->Arguments.end(); iter3++)
					{
						CVariableStatementASTNode* arg = *iter3;
						argument_data_types.push_back(arg->Type);
					}

					if (FindClassMethod(semanter, member->Identifier, argument_data_types, true, NULL, this) == NULL)
					{
						semanter->GetContext()->FatalError(CStringHelper::FormatString("Class does not implement method '%s' of interface '%s'.", member->Identifier.c_str(), interfaceClass->Identifier.c_str()), Token);
					}
				}
			}
		}

		// Finalize children.
		FinalizeChildren(semanter);
	}

	// Finalize generic instances.
	else if (IsGeneric == true)
	{
		for (std::vector<CClassASTNode*>::iterator iter = GenericInstances.begin(); iter != GenericInstances.end(); iter++)
		{
			(*iter)->Finalize(semanter);
		}
	}

	return this;
}

// =================================================================
//	Gathers matching identifiers for use with FindClassMethod.
// =================================================================
void CClassASTNode::GatherMatchingClassMethods(std::vector<CClassMemberASTNode*>& nodes,
																CASTNode*					base,
																CSemanter*					semanter, 
																CDeclarationIdentifier					identifier, 
																std::vector<CDataType*>		arguments, 
																bool						explicit_arguments,
																CASTNode*					ignoreNode, 
																CASTNode*					referenceNode)
{
	for (std::vector<CASTNode*>::iterator iter = base->Children.begin(); iter != base->Children.end(); iter++)
	{
		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(*iter);
		if (member				!= NULL &&
			member->MemberType	== MemberType::Method && 
			member->Identifier	== identifier &&
			member				!= ignoreNode &&
			arguments.size()	<= member->Arguments.size())
		{
			// Has one of the other members overridcen this method already?
			bool alreadyExists = false;
			for (std::vector<CClassMemberASTNode*>::iterator iter2 = nodes.begin(); iter2 != nodes.end(); iter2++)
			{
				CClassMemberASTNode* member2 = *iter2;
				if (member->Identifier == member2->Identifier &&
					member->Arguments.size() == member2->Arguments.size() &&
					member->IsVirtual == true && member2->IsOverride == true)
				{
					bool argsSame = true;

					for (unsigned int i = 0; i < member->Arguments.size(); i++)
					{
						CVariableStatementASTNode* arg = member->Arguments.at(i);
						CVariableStatementASTNode* arg2 = member2->Arguments.at(i);
						if (!arg->Type->IsEqualTo(semanter, arg2->Type))
						{
							argsSame = false;
							break;
						}
					}

					if (argsSame == true)
					{
						alreadyExists = true;
						break;
					}
				}
			}

			if (alreadyExists == false)
			{
				member->Semant(semanter);
				nodes.push_back(member);
			}
		}
	}
}

// =================================================================
//	Check for duplicate identifier.
// =================================================================
CClassMemberASTNode* CClassASTNode::FindClassMethod(CSemanter*					semanter, 
													CDeclarationIdentifier					identifier, 
													std::vector<CDataType*>		arguments, 
													bool						explicit_arguments,
													CASTNode*					ignoreNode, 
													CASTNode*					referenceNode)
{
	// Make sure this class is semanted.
	if (!Semanted)
	{
		Semant(semanter);
	}

	// Find all possible methods with the name.
	std::vector<CClassMemberASTNode*> nodes;

	CClassASTNode* scope = this;
	while (scope != NULL)
	{
		// Check child members.
		if (scope->Body != NULL)
		{
			GatherMatchingClassMethods(nodes, scope->Body, semanter, identifier, arguments, explicit_arguments, ignoreNode, referenceNode);			
		}

		// Check all interfaces. 
		for (std::vector<CClassASTNode*>::iterator interface_iter = scope->Interfaces.begin(); interface_iter != scope->Interfaces.end(); interface_iter++)
		{
			CClassASTNode* inter = *interface_iter;
			GatherMatchingClassMethods(nodes, inter->Body, semanter, identifier, arguments, explicit_arguments, ignoreNode, referenceNode);			
		}

		scope = scope->SuperClass;
	}
		
	// If reference node's parent is a state body, then check in there as well.
	// This whole system is dumb.
	if (referenceNode != NULL && referenceNode->Parent != NULL)
	{
		CClassStateASTNode* state = dynamic_cast<CClassStateASTNode*>(referenceNode->Parent);
		if (state != NULL)
		{
			GatherMatchingClassMethods(nodes, state, semanter, identifier, arguments, explicit_arguments, ignoreNode, referenceNode);			
		}
	}

	// Try and find amatch!
	CClassMemberASTNode* match			= NULL;
	CClassMemberASTNode* ambiguousMatch	= NULL;
	bool				 isExactMatch	= false;
	String			 errorMessage	= "";

	// Look for valid nodes.		
	for (std::vector<CClassMemberASTNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		CClassMemberASTNode* member = *iter;

		bool exact		= true;
		bool possible	= true;

		for (unsigned int i = 0; i < member->Arguments.size(); i++)
		{
			CVariableStatementASTNode* arg = member->Arguments.at(i);

			if (arguments.size() > member->Arguments.size())
			{
				continue;
			}

			if (i < arguments.size())
			{
				if (arguments.at(i)->IsEqualTo(semanter, arg->Type))
				{
					continue;
				}
				exact = false;

				if (!explicit_arguments && CCastExpressionASTNode::IsValidCast(semanter, arguments.at(i), arg->Type, false))// arguments.at(i)->CanCastTo(semanter, arg->Type))
				{
					continue;
				}
			}
			else if (arg->AssignmentExpression != NULL)
			{
				exact = false;
				if (!explicit_arguments)
				{
					continue;
				}
			}

			possible = false;
			break;
		}

		if (!possible)
		{
			continue;
		}

		if (exact == true)
		{
			if (isExactMatch == true)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Found ambiguous reference to method of class '%s'. Reference could mean either '%s' or '%s'.", Identifier.c_str(), match->ToString().c_str(), member->ToString().c_str()),
													referenceNode == NULL ? Token : referenceNode->Token);
			}
			else
			{
				errorMessage	= "";
				match			= member;
				isExactMatch	= true;
			}
		}
		else
		{
			if (!isExactMatch)
			{
				if (match != NULL)
				{
					if (ambiguousMatch == NULL)
					{
						ambiguousMatch = member;
					}
				}
				else
				{
					match = member;
				}
			}
		}
	}

	// Ambiguous non-exact matchs?
	if (isExactMatch == false &&
		ambiguousMatch != NULL)
	{
		errorMessage = CStringHelper::FormatString("Found ambiguous reference to method of class '%s'. Reference could mean either '%s' or '%s'.", Identifier.c_str(), match->ToString().c_str(), ambiguousMatch->ToString().c_str());
	}

	// Return?
	if (!isExactMatch)
	{
		if (errorMessage != "")
		{
			semanter->GetContext()->FatalError(errorMessage, referenceNode == NULL ? Token : referenceNode->Token);
		}
		if (explicit_arguments == true)
		{
			return NULL;
		}
	}

	// No match available? :S
	if (match == NULL)
	{
		return NULL;
	}

	// Return matched class.
	return match;
}

// =================================================================
//	Check for duplicate identifier.
// =================================================================
CClassMemberASTNode* CClassASTNode::FindClassField(CSemanter*					semanter, 
													CDeclarationIdentifier					identifier, 
													CASTNode*					ignoreNode, 
													CASTNode*					referenceNode)
{
	// Make sure this class is semanted.
	if (!Semanted)
	{
		Semant(semanter);
	}

	// Look for some sweet sweet methods.
	if (Body != NULL)
	{
		CClassMemberASTNode* result = NULL;

		// Look for explicit member matchs.
		for (std::vector<CASTNode*>::iterator iter = Body->Children.begin(); iter != Body->Children.end(); iter++)
		{
			CClassMemberASTNode* classNode = dynamic_cast<CClassMemberASTNode*>(*iter);
			if (classNode					!= NULL &&
				classNode					!= ignoreNode &&
				classNode->Identifier		== identifier &&
				classNode->MemberType		== MemberType::Field)
			{
				
				result = classNode; 
			}
		}

		// Return result!
		if (result != NULL)
		{
			return result;
		}
	}

	// Look up the inheritance tree.
	if (SuperClass != NULL)
	{
		return SuperClass->FindClassField(semanter, identifier, ignoreNode, referenceNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Finds a class property with the given arguments.
// =================================================================
CClassPropertyASTNode* CClassASTNode::FindClassProperty(CSemanter* semanter, 
														   CDeclarationIdentifier identifier, 
														   CASTNode* ignoreNode, 
														   CASTNode* referenceNode)
{
	// Make sure this class is semanted.
	if (!Semanted)
	{
		Semant(semanter);
	}

	// Look for some sweet sweet methods.
	if (Body != NULL)
	{
		CClassPropertyASTNode* result = NULL;

		// Look for explicit member matchs.
		for (std::vector<CASTNode*>::iterator iter = Body->Children.begin(); iter != Body->Children.end(); iter++)
		{
			CClassPropertyASTNode* classNode = dynamic_cast<CClassPropertyASTNode*>(*iter);
			if (classNode					!= NULL &&
				classNode					!= ignoreNode &&
				classNode->Identifier		== identifier)
			{
				
				result = classNode; 
			}
		}

		// Return result!
		if (result != NULL)
		{
			return result;
		}
	}

	// Look up the inheritance tree.
	if (SuperClass != NULL)
	{
		return SuperClass->FindClassProperty(semanter, identifier, ignoreNode, referenceNode);
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Check for duplicate identifier.
// =================================================================
CClassStateASTNode* CClassASTNode::FindClassState(CSemanter*					semanter, 
													CDeclarationIdentifier					identifier, 
													CASTNode*					ignoreNode, 
													CASTNode*					referenceNode)
{
	// Make sure this class is semanted.
	if (!Semanted)
	{
		Semant(semanter);
	}

	// Look for some sweet sweet methods.
	if (Body != NULL)
	{
		CClassStateASTNode* result = NULL;

		// Look for explicit member matchs.
		for (std::vector<CASTNode*>::iterator iter = Body->Children.begin(); iter != Body->Children.end(); iter++)
		{
			CClassStateASTNode* classNode = dynamic_cast<CClassStateASTNode*>(*iter);
			if (classNode					!= NULL &&
				classNode					!= ignoreNode &&
				classNode->Identifier		== identifier)
			{
				
				result = classNode; 
			}
		}

		// Return result!
		if (result != NULL)
		{
			return result;
		}
	}

	// Look up the inheritance tree.
	if (SuperClass != NULL)
	{
		return SuperClass->FindClassState(semanter, identifier, ignoreNode, referenceNode);
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
CASTNode* CClassASTNode::GetParentSearchScope(CSemanter* semanter)
{
	return Parent;
}

// =================================================================
//	Gets the list of children to be searched when looking for
//	declarations.
// =================================================================
std::vector<CASTNode*>& CClassASTNode::GetSearchScopeChildren(CSemanter* semanter)
{
	return Body->Children;
}

std::vector<CAliasASTNode*>& CClassASTNode::GetSearchScopeAliasChildren(CSemanter* semanter)
{
	return Body->AliasChildren;
}

// =================================================================
//	Checks if we can access this declaration from the given node.
// =================================================================
void CClassASTNode::CheckAccess(CSemanter* semanter, CASTNode* referenceBy)
{
	// If we are in a different package and not public, then refuse access.
	if (Token.SourceFile != referenceBy->Token.SourceFile && this->AccessLevel != AccessLevel::PUBLIC)
	{
		semanter->GetContext()->FatalError(CStringHelper::FormatString("Class '%s' is not accessible from this package.", ToString().c_str()), referenceBy->Token);
	}
}

// =================================================================
//	Returns true if the given class is in the inheritance 
//  or implementation tree for this class.
// =================================================================
bool CClassASTNode::InheritsFromClass(CSemanter* semanter, CClassASTNode* node)
{
	if (node == this)
	{
		return true;
	}

	Semant(semanter);
	node->Semant(semanter);

	CClassASTNode* check = this;
	while (check != NULL)
	{
		// Check for direct class inheriting.
		if (check == node)
		{
			return true;
		}

		// Check for interface inheriting.
		for (std::vector<CClassASTNode*>::iterator iter = check->Interfaces.begin(); iter != check->Interfaces.end(); iter++)
		{
			if ((*iter) == node)
			{
				return true;
			}
		}

		check = check->SuperClass;
	}

	return false;
}

// =================================================================
//	Instantiates a copy of this class if its a generic, or just
//	returns the class if its not generic.
// =================================================================
CClassASTNode* CClassASTNode::GenerateClassInstance(CSemanter* semanter, CASTNode* referenceNode, std::vector<CDataType*> generic_arguments)
{
	if (IsGeneric == true)
	{
		if (generic_arguments.size() != GenericTypeTokens.size())
		{
			if (generic_arguments.size() == 0)
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Class '%s' is generic and expects generic arguments.", Token.Literal.c_str()), referenceNode->Token);
			}
			else
			{
				semanter->GetContext()->FatalError(CStringHelper::FormatString("Incorrect number of generic arguments given to class '%s' during instantiation.", Token.Literal.c_str()), referenceNode->Token);
			}
		}

		// Instance with these data types already exists?
		for (std::vector<CClassASTNode*>::iterator iter = GenericInstances.begin(); iter != GenericInstances.end(); iter++)
		{
			CClassASTNode* instance = *iter;
			bool argumentsMatch = true;
			
			for (unsigned int i = 0; i < instance->GenericInstanceTypes.size(); i++)
			{
				if (!instance->GenericInstanceTypes.at(i)->IsEqualTo(semanter, generic_arguments.at(i)))
				{
					argumentsMatch = false;
					break;
				}
			}

			if (argumentsMatch == true)
			{
				return instance;
			}
		}

		// Nope, time to create it.
		CClassASTNode* astNode			= dynamic_cast<CClassASTNode*>(this->Clone(semanter));
		astNode->Parent					= Parent; // We set the derived node to our parent so it can correctly find things in its scope, but so that it can't be found by others.
		astNode->GenericInstanceOf		= this;
		astNode->GenericInstanceTypes	= generic_arguments;
		GenericInstances.push_back(astNode);

		// Create alias's for all generic type tokens.		
		for (unsigned int i = 0; i < generic_arguments.size(); i++)
		{
			CToken&		token	= GenericTypeTokens.at(i);
			CDataType*	type	= generic_arguments.at(i);
			
			CAliasASTNode* alias = new CAliasASTNode(astNode->Body, this->Token, token.Literal, type);
			astNode->Body->AddChild(alias);
			astNode->Body->AliasChildren.push_back(alias);
			alias->Semant(semanter);
		}

		// Semant our new instance.
		astNode->Semant(semanter);

		return astNode;
	}
	else
	{
		if (generic_arguments.size() > 0)
		{
			semanter->GetContext()->FatalError(CStringHelper::FormatString("Class '%s' is not generic and cannot be instantiated.", Token.Literal.c_str()), referenceNode->Token);
		}
		return this;
	}
}

// =================================================================
//	Causes this node to be translated.
// =================================================================
void CClassASTNode::Translate(CTranslator* translator)
{
	if (IsGeneric == true)
	{		
		for (std::vector<CClassASTNode*>::iterator iter = GenericInstances.begin(); iter != GenericInstances.end(); iter++)
		{
			CClassASTNode* instance = *iter;
			translator->TranslateClass(instance);
		}
	}
	else
	{
		translator->TranslateClass(this);
	}
}