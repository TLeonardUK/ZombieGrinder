/* *****************************************************************

		CVMBinaryTranslator.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#ifdef _WIN32
#pragma warning(disable:4996) // Remove Microsofts obsession with their proprietary _s postfixed functions.
#endif

#include "XScript/Translator/VMBinary/CVMBinaryTranslator.h"

#include "XScript/Compiler/CTranslationUnit.h"
#include "XScript/Compiler/CCompiler.h"

#include "XScript/Helpers/CCollectionHelper.h"
#include "XScript/Helpers/CPathHelper.h"
#include "XScript/Helpers/CStringHelper.h"

#include "XScript/Parser/Nodes/CASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CPackageASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassStateASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassBodyASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassPropertyASTNode.h"
#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CMethodBodyASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"

#include "XScript/Parser/Nodes/Statements/CCaseStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CDefaultStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CBlockStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CBreakStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CContinueStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CDoStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CForEachStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CForStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CIfStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CReturnStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CSwitchStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CWhileStatementASTNode.h"
#include "XScript/Parser/Nodes/Statements/CStateChangeStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Types/CArrayInitializerASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Assignment/CAssignmentExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CBaseExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Math/CBinaryMathExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CCastExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CClassRefExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CCommaExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Math/CComparisonExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CFieldAccessExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIdentifierExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CIndexExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CLiteralExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Math/CLogicalExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CMethodCallExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CNewExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Math/CPostFixExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Math/CPreFixExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CSliceExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Branching/CTernaryExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CThisExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CTypeOfExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Polymorphism/CIndexOfExpressionASTNode.h"
#include "XScript/Parser/Nodes/Expressions/Types/CTypeExpressionASTNode.h"

#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"
#include "XScript/Parser/Types/Helper/CNullDataType.h"
#include "XScript/Parser/Types/Helper/CClassReferenceDataType.h"

#include "XScript/VirtualMachine/CVMBinary.h"

#include <stdarg.h> 
#include <algorithm>

// If set then debug-trace instructions are output, which give fine-grained
// callstack location support.
#define VM_WRITE_DEBUG_TRACE

// =================================================================
//	Closes the source file.
// =================================================================
CVMBinaryTranslator::CVMBinaryTranslator() 
	: m_package(NULL)
{
}

// =================================================================
//
// =================================================================
CVMBinaryTranslator::~CVMBinaryTranslator()
{
	for (std::vector<CSymbol*>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	m_loop_break_jump_target_stack.clear();
	m_loop_continue_jump_target_stack.clear();
	m_string_table.clear();
	m_scope_stack.clear();
}

// =================================================================
//	Gets a VM-Binary.
// =================================================================
CVMBinary* CVMBinaryTranslator::GetVMBinary()
{
	CVMBinary* binary = new CVMBinary();
	binary->Setup(m_semanter, m_string_table, m_symbol_table);
	return binary;
}

// =================================================================
//	Creates a new symbol.
// =================================================================
CSymbol* CVMBinaryTranslator::NewSymbol(SymbolType::Type type, CASTNode* node)
{
	CSymbol* scope = GetScope();

	CSymbol* sym = new CSymbol();
	sym->Type = type;
	sym->Node = node;
	sym->Instructions.SetSemanter(m_semanter);
	sym->Index = m_symbol_table.size(); 

	if (scope != NULL)
	{
		scope->Children.push_back(sym);
	}
	m_symbol_table.push_back(sym);

	return sym;
}

// =================================================================
//	Pushs a new symbol scope.
// =================================================================
void CVMBinaryTranslator::PushScope(CSymbol* scope)
{
	m_scope_stack.push_back(scope);
}

// =================================================================
//	Pops current symbol scope.
// =================================================================
CSymbol* CVMBinaryTranslator::PopScope()
{
	CSymbol* sym = (m_scope_stack.at(m_scope_stack.size() - 1));
	m_scope_stack.pop_back();
	return sym;
}

// =================================================================
//	Gets the current scope we are in.
// =================================================================
CSymbol* CVMBinaryTranslator::GetScope()
{
	if (m_scope_stack.size() > 0)
	{
		return (m_scope_stack.at(m_scope_stack.size() - 1));
	}
	else
	{
		return NULL;
	}
}

// =================================================================
//	Gets the current instruction buffer.
// =================================================================
CInstructionBuffer* CVMBinaryTranslator::GetInstructionBuffer()
{
	return &GetScope()->Instructions;
}

// =================================================================
//	Gets a parent scope by type.
// =================================================================
CSymbol* CVMBinaryTranslator::GetParentScope(SymbolType::Type type)
{
	for (std::vector<CSymbol*>::reverse_iterator iter = m_scope_stack.rbegin(); iter != m_scope_stack.rend(); iter++)
	{
		CSymbol* sym = *iter;
		if (sym->Type == type)
		{
			return sym;
		}
	}

	DBG_ASSERT(false);
	return NULL;
}

// =================================================================
//	Glorious fun for keeping track of loop variables.
// =================================================================
int CVMBinaryTranslator::PushBreakJmpTarget()
{
	int target = GetInstructionBuffer()->CreateJmpTarget();
	m_loop_break_jump_target_stack.push_back(target);
	return target;
}

int CVMBinaryTranslator::PushContinueJmpTarget()
{
	int target = GetInstructionBuffer()->CreateJmpTarget();
	m_loop_continue_jump_target_stack.push_back(target);
	return target;
}

void CVMBinaryTranslator::PopBreakJumpTarget()
{
	m_loop_break_jump_target_stack.pop_back();
}

void CVMBinaryTranslator::PopContinueJumpTarget()
{
	m_loop_continue_jump_target_stack.pop_back();
}

// =================================================================
//	Prints a dissassembly of the symbol.
// =================================================================
void CVMBinaryTranslator::PrintDebugSymbol(CSymbol* symbol)
{
	//DBG_LOG("[Symbol %i]", symbol->Index);
	switch (symbol->Type)
	{
	case SymbolType::Root:
		{
			/*
			DBG_LOG("Root");
			*/
			break;
		}
	case SymbolType::Class:
		{
			DBG_LOG("- Class %s", symbol->Identifier.c_str());
			DBG_LOG("- - Data Size = %i", symbol->FieldCount);

			for (std::vector<CInterfaceSymbol>::iterator iter = symbol->Interfaces.begin(); iter != symbol->Interfaces.end(); iter++)
			{
				CInterfaceSymbol& inter_symbol = *iter;
				DBG_LOG("- - Interface[%s]: Offset=%i", inter_symbol.InterfaceClass->Identifier.c_str(), inter_symbol.VFTableOffset);
			}

			for (unsigned int i = 0; i < symbol->VFTable.size(); i++)
			{
				if (symbol->VFTable[i] == NULL)
				{
					DBG_LOG("- - VFTable[%i] = NULL", i);	
				}
				else
				{
					DBG_LOG("- - VFTable[%i] = [%i] %s (%s:%i)", i, symbol->VFTable[i]->Index, symbol->VFTable[i]->Identifier.c_str(), symbol->VFTable[i]->Node->Token.SourceFile.c_str(), symbol->VFTable[i]->Node->Token.Row);
				}
			}

			break;
		}
	case SymbolType::ClassField:
		{
			/*
			DBG_LOG("- - Field %s", symbol->Identifier.c_str());
			*/
			break;
		}
	case SymbolType::ClassState:
		{
			DBG_LOG("- - State %s", symbol->Identifier.c_str());
			break;
		}
	case SymbolType::ClassProperty:
		{
			DBG_LOG("- - Property %s", symbol->Identifier.c_str());
			break;
		}
	case SymbolType::ClassMethod:
		{
			if (symbol->Instructions.Get_Instruction_Count() > 0)
			{
				DBG_LOG("- - [%i] Method %s", symbol->Index, symbol->Identifier.c_str());

				for (int i = 0; i < symbol->Instructions.Get_Instruction_Count(); i++)
				{
					for (int j = 0; j < symbol->Instructions.Get_JmpTarget_Count(); j++)
					{
						int offset = symbol->Instructions.Get_JmpTarget(j);
						if (offset == i)
						{
							DBG_LOG("- - - _%i:", j);
							break;
						}					
					}

					CInstruction& instruction = symbol->Instructions.Get_Instruction(i);
					DBG_LOG("- - - [%i] %s", i, instruction.Disassemble(m_symbol_table).c_str());
				}
			}
			break;
		}
	case SymbolType::LocalVariable:
		{
			/*
			DBG_LOG("- - - Local %s", symbol->Identifier.c_str());
			break;
			*/
		}
	}

	for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
	{
		PrintDebugSymbol(*iter);
	}
}

// =================================================================
//	Sorts serialized fields into alpha-numeric order.
// =================================================================
bool AlphaNum_Node_Sort(CASTNode* a, CASTNode* b)
{
	CDeclarationASTNode* da = static_cast<CDeclarationASTNode*>(a);
	CDeclarationASTNode* db = static_cast<CDeclarationASTNode*>(b);

	return strcmp(da->Identifier.c_str(), db->Identifier.c_str()) < 0;
}

void CVMBinaryTranslator::SortSerializedFields(CASTNode* node)
{
	std::vector<CASTNode*> serialized;

	for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); )
	{
		CASTNode* child = *iter;
		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(child);
		CClassPropertyASTNode* prop = dynamic_cast<CClassPropertyASTNode*>(child);

		if (member != NULL)
		{
			if (member->MemberType == MemberType::Field &&
				member->IsSerialized == true)
			{
				serialized.push_back(child);
				iter = node->Children.erase(iter);
				continue;
			}
		}
		else if (prop != NULL)
		{
			if (prop->IsSerialized == true)
			{
				serialized.push_back(child);
				iter = node->Children.erase(iter);
				continue;
			}
		}

		iter++;
	}

	// Sort and re-add serialized fields in alphabetical order.
	std::sort(serialized.begin(), serialized.end(), &AlphaNum_Node_Sort);
	for (std::vector<CASTNode*>::iterator iter = serialized.begin(); iter != serialized.end(); iter++)
	{
		CASTNode* child = *iter;
		node->Children.push_back(child);
	}

	// Sort children.
	for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
	{
		CASTNode* child = *iter;
		SortSerializedFields(child);
	}
}

// =================================================================
//	Translate a package node.
// =================================================================
void CVMBinaryTranslator::TranslatePackage(CPackageASTNode* node)
{	
	// Sort serialized fields and properties into alphanumeric order
	// so that when adding new ones we don't upset the save/load order.
	SortSerializedFields(node);

	// Build the symbol table.
	BuildSymbolTable(node);

	// Build the string table.
	BuildStringTable(node);

	// Translate to actual instructions!
	PushScope(m_base_scope);
	for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
	{
		CClassASTNode* child = dynamic_cast<CClassASTNode*>(*iter);
		if (child == NULL)
		{
			continue;
		}
		TranslateClass(child);
	}
	PopScope();

	// Build all class VF-Tables. Realistically this can be done during
	// translation, but this is simpler.
	BuildVFTables();

	//PrintDebugSymbol(m_base_scope);
}

// =================================================================
//	Adds a string to the string table and returns its index.
// =================================================================
int CVMBinaryTranslator::AddToStringTable(const String& str)
{
	int hash = StringHelper::Hash(str.c_str());

	int index = 0;
	if (m_string_table_lookup.Get(hash, index))
	{
		return index;
	}
	
	index = m_string_table.size();

	m_string_table.push_back(str);
	m_string_table_lookup.Set(hash, index);

	return index;
}

// =================================================================
//	Builds all class VF tables.
// =================================================================
void CVMBinaryTranslator::BuildVFTables()
{
	for (std::vector<CSymbol*>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CSymbol* symbol = *iter;
		if (symbol->Type == SymbolType::Class)
		{
			// Calculate method count.
			// Should be done during translation, but lazy etc.
			for (std::vector<CSymbol*>::iterator iter2 = symbol->Children.begin(); iter2 != symbol->Children.end(); iter2++)
			{
				CSymbol* child_symbol = *iter2;
				if (symbol->Type == SymbolType::ClassMethod)
				{
					symbol->MethodCount++;
				}
			}

			// Fill in the VF-Table.
			symbol->VFTable.resize(symbol->VirtualMethodCount);
			for (int i = 0; i < symbol->VirtualMethodCount; i++)
			{
				// Look through normal functions.
				for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
				{
					CSymbol* child_symbol = *iter;
					if (child_symbol->Type == SymbolType::ClassMethod &&
						child_symbol->IsVirtual == true && 
						child_symbol->VFTableOffset == i)
					{
						symbol->VFTable[i] = child_symbol;
					}
				}

				// Wouldn't we need to cast to base type if we do this????? 
				// Look through interface methods that are not overriden (tl;dr thats pretty much just ToString).
				if (symbol->VFTable[i] == NULL)
				{
					for (std::vector<CInterfaceSymbol>::iterator iter = symbol->Interfaces.begin(); iter != symbol->Interfaces.end(); iter++)
					{
						CInterfaceSymbol& inter_symbol = *iter;
			
						// Grab normal declarations for the functions.
						for (std::vector<CSymbol*>::iterator iter = inter_symbol.InterfaceClass->Children.begin(); iter != inter_symbol.InterfaceClass->Children.end(); iter++)
						{
							CSymbol* child_symbol = *iter;
							if (child_symbol->Type == SymbolType::ClassMethod &&
								child_symbol->IsVirtual == true && 
								child_symbol->VFTableOffset + inter_symbol.VFTableOffset == i)
							{
								symbol->VFTable[i] = child_symbol;
								break;
							}
						}
					}
				}
			}
		}
	}
}

// =================================================================
//	Builds the main string table.
// =================================================================
void CVMBinaryTranslator::BuildStringTable(CASTNode* node)
{
	CLiteralExpressionASTNode* lit_node	= dynamic_cast<CLiteralExpressionASTNode*>(node);

	if (lit_node != NULL)
	{
		if (dynamic_cast<CStringDataType*>(lit_node->ExpressionResultType) != NULL)
		{
			lit_node->StringTableIndex = AddToStringTable(lit_node->Literal);
		}
	}

	node->TranslatedDebugInfo.Row			= node->Token.Row;
	node->TranslatedDebugInfo.Column		= node->Token.Column; 	
	node->TranslatedDebugInfo.FileNameIndex	= AddToStringTable(node->Token.SourceFile);

	for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
	{
		BuildStringTable(*iter);
	}

	CClassASTNode* class_type = dynamic_cast<CClassASTNode*>(node);
	if (class_type != NULL && class_type->IsGeneric == true && class_type->GenericInstanceOf == NULL)
	{		
		for (std::vector<CClassASTNode*>::iterator iter = class_type->GenericInstances.begin(); iter != class_type->GenericInstances.end(); iter++)
		{
			BuildStringTable(*iter);
		}
	}
}

// =================================================================
//	Writes out debug trace instructions if trace has changed.
// =================================================================
void CVMBinaryTranslator::UpdateDebugTrace(CASTNode* node)
{
#ifdef VM_WRITE_DEBUG_TRACE
	if (//m_last_translated_debug_info.Column != node->TranslatedDebugInfo.Column ||
		m_last_translated_debug_info.Row != node->TranslatedDebugInfo.Row ||
		m_last_translated_debug_info.FileNameIndex != node->TranslatedDebugInfo.FileNameIndex)
	{
		m_last_translated_debug_info = node->TranslatedDebugInfo;
		DBG_ASSERT(m_last_translated_debug_info.Row >= 0);

		GetInstructionBuffer()->EmitDebugTrace(m_last_translated_debug_info.Row, m_last_translated_debug_info.Column, m_last_translated_debug_info.FileNameIndex);
	}
#endif
}

// =================================================================
//	Builds the translated symbol table recursively form the AST.
// =================================================================
void CVMBinaryTranslator::BuildSymbolTable(CASTNode* node)
{
	CPackageASTNode*			pkg_node	= dynamic_cast<CPackageASTNode*>(node);
	CClassASTNode*				class_node	= dynamic_cast<CClassASTNode*>(node);
	CClassMemberASTNode*		member_node = dynamic_cast<CClassMemberASTNode*>(node);
	CClassStateASTNode*			state_node	= dynamic_cast<CClassStateASTNode*>(node);
	CClassPropertyASTNode*		prop_node	= dynamic_cast<CClassPropertyASTNode*>(node);
	CVariableStatementASTNode*	var_node	= dynamic_cast<CVariableStatementASTNode*>(node);
	CSymbol*					symbol		= NULL;
	
	if (node->TranslatedSymbol == NULL)
	{
		// Package
		if (pkg_node != NULL)
		{
			symbol = NewSymbol(SymbolType::Root, node);
			m_base_scope = symbol;
		}

		// Class
		else if (class_node != NULL)
		{
			symbol = BuildClassSymbol(class_node);
			if (class_node->IsGeneric == true && class_node->GenericInstanceOf == NULL)
			{
				return;
			}
		}
		
		// State
		else if (state_node != NULL)
		{
			symbol = BuildStateSymbol(state_node);
		}

		// Class field/method
		else if (member_node != NULL)
		{
			symbol = BuildMemberSymbol(member_node);
		}

		// Class property
		else if (prop_node != NULL)
		{
			symbol = BuildPropertySymbol(prop_node);
		}

		// Local variable.
		else if (var_node != NULL)
		{
			symbol = BuildVariableSymbol(var_node);
		}
	}
	else
	{
		symbol = node->TranslatedSymbol;
	}

	// Build children.
	if (symbol != NULL)
	{
		PushScope(symbol);
	}

	for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
	{
		BuildSymbolTable(*iter);
	}

	if (symbol != NULL)
	{
		PopScope();
	}
}

// =================================================================
//	Builds a class symbol.
// =================================================================
CSymbol* CVMBinaryTranslator::BuildClassSymbol(CClassASTNode* node)
{
	if (node->IsGeneric == true && node->GenericInstanceOf == NULL)
	{
		for (unsigned int i = 0; i < node->GenericInstances.size(); i++)
		{
			BuildSymbolTable(node->GenericInstances.at(i));
		}
	}
	else
	{
		// Do not translate generic classes only generic instance or normal classes.
		if ((node->IsGeneric == false || node->GenericInstanceOf != NULL))
		{
			CSymbol* symbol = NewSymbol(SymbolType::Class, node);
			symbol->Identifier = node->ClassReferenceDataType->ToString();
			symbol->FieldCount = 0;
			symbol->MethodCount = 0;
			symbol->VirtualMethodCount = 0;
			symbol->StaticCount = 0;
			symbol->StateCount = 0;

			//DBG_LOG("Class %s", symbol->Identifier.c_str());

			// Translate super class.
			if (node->SuperClass != NULL && node->SuperClass->TranslatedSymbol == NULL)
			{
				BuildSymbolTable(node->SuperClass);
			}
		
			// Add space for parents fields/methods/virtual-methods.
			if (node->SuperClass != NULL)
			{
				CSymbol* superclass_symbol = node->SuperClass->TranslatedSymbol;

				symbol->FieldCount			+= superclass_symbol->FieldCount;
				symbol->VirtualMethodCount	+= superclass_symbol->VirtualMethodCount;
				symbol->StaticCount			+= superclass_symbol->StaticCount;
				
				std::vector<CSymbol*> child_symbols = superclass_symbol->Children;

				for (std::vector<CSymbol*>::iterator iter = superclass_symbol->Children.begin(); iter != superclass_symbol->Children.end(); iter++)
				{
					CSymbol* child_symbol = *iter;
					child_symbol->FromInherit = true;
					symbol->Children.push_back(child_symbol);
				}

				// Copy over interface symbols.
				for (std::vector<CInterfaceSymbol>::iterator iter = superclass_symbol->Interfaces.begin(); iter != superclass_symbol->Interfaces.end(); iter++)
				{
					CInterfaceSymbol inter = *iter;
					symbol->Interfaces.push_back(inter);
				}
			}

			// Translate interfaces.
			for (std::vector<CClassASTNode*>::iterator iter = node->Interfaces.begin(); iter != node->Interfaces.end(); iter++)
			{
				CClassASTNode* inter = *iter;

				// TODO: refactor, fix code dupe.

				// Translate super class.
				if (inter->TranslatedSymbol == NULL)
				{
					BuildSymbolTable(inter);
				}

				// Add space for parents fields/methods/virtual-methods.
				CSymbol* superclass_symbol = inter->TranslatedSymbol;

				// Store interface casting information.
				CInterfaceSymbol interface_symbol;
				interface_symbol.InterfaceClass = superclass_symbol;
				interface_symbol.VFTableOffset = symbol->VirtualMethodCount;
				symbol->Interfaces.push_back(interface_symbol);

				std::vector<CSymbol*> child_symbols = superclass_symbol->Children;

				for (std::vector<CSymbol*>::iterator iter = superclass_symbol->Children.begin(); iter != superclass_symbol->Children.end(); iter++)
				{
					CSymbol* child_symbol = *iter;

					if (child_symbol->IsVirtual == true)
					{
						// Push the object-inherited methods in to interface.
						if (child_symbol->FromInherit == true)
						{
							// If this class already has an instance of the inherited method, use that
							// rather than the base interface one.
//							for (std::vector<CSymbol*>::iterator iter = superclass_symbol->Children.begin(); iter != superclass_symbol->Children.end(); iter++)
//							{
//								CSymbol* child_symbol = *iter;
//								if (child_symbol->Node == 
//							}

							// TODO: Patch at instruction emission-time.

							// TODO: Fix this, its not going to work, it won't get the instructions.
							//CSymbol* inherit_symbol = NewSymbol(SymbolType::ClassMethod, child_symbol->Node);	
							//inherit_symbol->MethodTableOffset = symbol->MethodCount;
							//inherit_symbol->VFTableOffset = symbol->VirtualMethodCount;
						}

						symbol->VirtualMethodCount++;
					}
				}
			}

			node->TranslatedSymbol = symbol;
		}
	}

	return node->TranslatedSymbol;
}

// =================================================================
//	Builds a class member symbol.
// =================================================================
CSymbol* CVMBinaryTranslator::BuildMemberSymbol(CClassMemberASTNode* node)
{
	CClassASTNode* classNode = dynamic_cast<CClassASTNode*>(GetParentScope(SymbolType::Class)->Node);
	CSymbol* classSymbol = GetParentScope(SymbolType::Class);

	CSymbol* symbol = NULL;
	if (node->MemberType == MemberType::Field)
	{
		symbol = NewSymbol(SymbolType::ClassField, node);
		symbol->Identifier = node->Identifier.Get_String();
		symbol->FieldDataType = node->ReturnType;
		symbol->IsSerialized  = node->IsSerialized;
		symbol->SerializeVersion = node->SerializeVersion;

		if (node->IsStatic == true)
		{
			symbol->FieldOffset = classSymbol->StaticCount;
			classSymbol->StaticCount++;
		}
		else
		{
			symbol->FieldOffset = classSymbol->FieldCount;
		}

		if (node->IsOffsetForced)
		{
			symbol->FieldOffset = node->ForcedOffset;
		}

		classSymbol->FieldCount++;
	}
	else
	{
		symbol = NewSymbol(SymbolType::ClassMethod, node);	
		symbol->Identifier = node->Identifier.Get_String();
		symbol->MethodTableOffset = classSymbol->MethodCount;
		symbol->ReturnDataType = node->ReturnType;
		symbol->LocalCount = 0;

		if (node->IsVirtual == true)
		{
			symbol->IsVirtual = true;

			if (node->IsOverride == true)
			{
				CClassASTNode* interfaceClassNode = node->BaseVirtualMethod != NULL ? dynamic_cast<CClassASTNode*>(node->BaseVirtualMethod->Parent->Parent) : NULL;

				// Try and find already existing virtual method so we can nick its vf offset.
				symbol->VFTableOffset = node->BaseVirtualMethod->TranslatedSymbol->VFTableOffset;

				// If this is an interface we need to add the interface offset to get the correct vf-table offset.
				if (interfaceClassNode != NULL &&
					interfaceClassNode->IsInterface == true)
				{
					for (std::vector<CInterfaceSymbol>::iterator iter = classSymbol->Interfaces.begin(); iter != classSymbol->Interfaces.end(); iter++)
					{
						CInterfaceSymbol inter = *iter;
						if (inter.InterfaceClass->Node == interfaceClassNode)
						{
							symbol->VFTableOffset += inter.VFTableOffset;
						}
					}
				}

				// Remove other symbols with same vf-table offset.
				for (std::vector<CSymbol*>::iterator iter = classSymbol->Children.begin(); iter != classSymbol->Children.end(); )
				{
					CSymbol* base = *iter;
					if (base != symbol &&
						base->VFTableOffset == symbol->VFTableOffset &&
						base->Type == SymbolType::ClassMethod &&
						base->IsVirtual == true)
					{
						iter = classSymbol->Children.erase(iter);			
					}
					else
					{
						iter++;
					}
				}
			}
			else
			{
				symbol->VFTableOffset = classSymbol->VirtualMethodCount;
				classSymbol->VirtualMethodCount++;
			}
		}
	}

	node->TranslatedSymbol = symbol;
	return node->TranslatedSymbol;
}

// =================================================================
//	Builds a class property symbol.
// =================================================================
CSymbol* CVMBinaryTranslator::BuildPropertySymbol(CClassPropertyASTNode* node)
{
	CClassASTNode* classNode = dynamic_cast<CClassASTNode*>(GetParentScope(SymbolType::Class)->Node);
	CSymbol* classSymbol = GetParentScope(SymbolType::Class);

	CSymbol* symbol				= NewSymbol(SymbolType::ClassProperty, node);
	symbol->Identifier			= node->Identifier.Get_String();
	symbol->PropertyDataType	= node->ReturnType;
	symbol->IsSerialized		= node->IsSerialized;
	symbol->SerializeVersion	= node->SerializeVersion;
	
	//DBG_LOG("- Property %s", symbol->Identifier.c_str());

	node->TranslatedSymbol = symbol;
	return node->TranslatedSymbol;
}

// =================================================================
//	Builds a class variable symbol.
// =================================================================
CSymbol* CVMBinaryTranslator::BuildVariableSymbol(CVariableStatementASTNode* node)
{
	CClassASTNode* classNode = dynamic_cast<CClassASTNode*>(GetParentScope(SymbolType::Class)->Node);	
	CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);
	
	CSymbol* symbol = NewSymbol(SymbolType::LocalVariable, node);
	symbol->Identifier = node->Identifier.Get_String();
	symbol->LocalDataType = node->Type;
	symbol->LocalOffset = methodSymbol->LocalCount;
	methodSymbol->LocalCount++;

	node->TranslatedSymbol = symbol;
	return node->TranslatedSymbol;
}

// =================================================================
//	Builds a class state symbol.
// =================================================================
CSymbol* CVMBinaryTranslator::BuildStateSymbol(CClassStateASTNode* node)
{
	CSymbol* classSymbol = GetParentScope(SymbolType::Class);
	
	CSymbol* symbol = NewSymbol(SymbolType::ClassState, node);
	symbol->Identifier = node->Identifier.Get_String();
	symbol->IsDefault = node->IsDefault;

	classSymbol->StateCount++;

	node->TranslatedSymbol = symbol;
	return node->TranslatedSymbol;
}

// =================================================================
//	Translates a nodes children.
// =================================================================
void CVMBinaryTranslator::TranslateChildren(CASTNode* node)
{
	for (std::vector<CASTNode*>::iterator iter = node->Children.begin(); iter != node->Children.end(); iter++)
	{
		(*iter)->Translate(this);
	}
}

// =================================================================
//	Translate a class node.
// =================================================================
void CVMBinaryTranslator::TranslateClass(CClassASTNode* node)
{
	if (node->IsGeneric == true && node->GenericInstanceOf == NULL)
	{
		for (unsigned int i = 0; i < node->GenericInstances.size(); i++)
		{
			TranslateClass(node->GenericInstances.at(i));
		}
	}
	else
	{
		// Do not translate generic classes only generic instance or normal classes.
		if ((node->IsGeneric == false || node->GenericInstanceOf != NULL))
		{
			PushScope(node->TranslatedSymbol);
			TranslateChildren(node->Body);
			PopScope();
		}
	}
}

// =================================================================
//	Translate a state-change statement.
// =================================================================
void CVMBinaryTranslator::TranslateStateChangeStatement(CStateChangeStatementASTNode* node)
{
	switch (node->Token.Type)
	{
	case TokenIdentifier::KEYWORD_PUSH_STATE:
		{
			GetInstructionBuffer()->EmitPushState(node->State->TranslatedSymbol->Index);
			break;
		}
	case TokenIdentifier::KEYWORD_POP_STATE:
		{
			GetInstructionBuffer()->EmitPopState();
			break;
		}
	case TokenIdentifier::KEYWORD_CHANGE_STATE:
		{
			GetInstructionBuffer()->EmitChangeState(node->State->TranslatedSymbol->Index);
			break;
		}
	}
}

// =================================================================
//	Translate a class state node.
// =================================================================
void CVMBinaryTranslator::TranslateClassState(CClassStateASTNode* node)
{
	PushScope(node->TranslatedSymbol);
	TranslateChildren(node);
	PopScope();
}

// =================================================================
//	Translate a class member node.
// =================================================================
void CVMBinaryTranslator::TranslateClassMember(CClassMemberASTNode* node)
{		
	if (node->MemberType == MemberType::Method &&
		node->IsAbstract == false &&
		node->IsNative == false &&
		node->Body != NULL)
	{
		PushScope(node->TranslatedSymbol);

		// If not static we need to reserve a register for the 'this' pointer to go in.
		if (node->IsStatic == false)
		{
			node->TranslatedSymbol->ThisRegister = GetInstructionBuffer()->NewRegister();
			GetInstructionBuffer()->EmitLoadThis(node->TranslatedSymbol->ThisRegister);
		}
		else
		{
			node->TranslatedSymbol->ThisRegister = -1;
		}

		TranslateChildren(node->Body);

		if (node->IsStatic == false)
		{
			GetInstructionBuffer()->FreeRegister(node->TranslatedSymbol->ThisRegister);
		}
		
		PopScope();
	}
}

// =================================================================
//	Translates a local variable statement.
// =================================================================
void CVMBinaryTranslator::TranslateVariableStatement(CVariableStatementASTNode* node)
{
	UpdateDebugTrace(node);

	if (node->AssignmentExpression != NULL)
	{
		int temp_reg = node->AssignmentExpression->TranslateExpr(this);

		// Store result in local variable.
		GetInstructionBuffer()->EmitStoreLocal(temp_reg, node->TranslatedSymbol);

		GetInstructionBuffer()->FreeRegister(temp_reg);
	}
}

// =================================================================
//	Translates a block statement.
// =================================================================	
void CVMBinaryTranslator::TranslateBlockStatement(CBlockStatementASTNode* node)
{
	node->TranslateChildren(this);
}

// =================================================================
//	Translates a break statement.
// =================================================================	
void CVMBinaryTranslator::TranslateBreakStatement(CBreakStatementASTNode* node)
{
	UpdateDebugTrace(node);

	DBG_ASSERT(m_loop_break_jump_target_stack.size() > 0);

	int jump_target = m_loop_break_jump_target_stack.at(m_loop_break_jump_target_stack.size() - 1);

	GetInstructionBuffer()->EmitJmp(jump_target);
}

// =================================================================
//	Translates a continue statement.
// =================================================================	
void CVMBinaryTranslator::TranslateContinueStatement(CContinueStatementASTNode* node)
{	
	UpdateDebugTrace(node);

	DBG_ASSERT(m_loop_continue_jump_target_stack.size() > 0);

	int jump_target = m_loop_continue_jump_target_stack.at(m_loop_continue_jump_target_stack.size() - 1);

	GetInstructionBuffer()->EmitJmp(jump_target);	
}

// =================================================================
//	Translates a do statement.
// =================================================================	
void CVMBinaryTranslator::TranslateDoStatement(CDoStatementASTNode* node)
{
	UpdateDebugTrace(node);

	int break_jump_target = PushBreakJmpTarget();
	int continue_jump_target = PushContinueJmpTarget();

	// _start:
	GetInstructionBuffer()->PatchJmpTarget(continue_jump_target);

	// <body>
	node->BodyStatement->Translate(this);

	// <expr>
	int output_reg = node->ExpressionStatement->TranslateExpr(this);

	// jmpontrue expr_result, _start
	GetInstructionBuffer()->EmitJmpOnTrue(output_reg, continue_jump_target);
	GetInstructionBuffer()->FreeRegister(output_reg);

	// _break:
	GetInstructionBuffer()->PatchJmpTarget(break_jump_target);

	PopBreakJumpTarget();
	PopContinueJumpTarget();
}

// =================================================================
//	Translates a for statement.
// =================================================================	
void CVMBinaryTranslator::TranslateForStatement(CForStatementASTNode* node)
{
	UpdateDebugTrace(node);

	int break_jump_target = PushBreakJmpTarget();
	int continue_jump_target = PushContinueJmpTarget();
	int start_jump_target = PushContinueJmpTarget();

	// <init-expr>
	node->InitialStatement->Translate(this);

	// _start:
	GetInstructionBuffer()->PatchJmpTarget(start_jump_target);

	// <expr>
	int output_reg = node->ConditionExpression->TranslateExpr(this);
	
	// jmponfalse expr_result, _break
	GetInstructionBuffer()->EmitJmpOnFalse(output_reg, break_jump_target);
	GetInstructionBuffer()->FreeRegister(output_reg);

	// <body>
	node->BodyStatement->Translate(this);

	// _continue:
	GetInstructionBuffer()->PatchJmpTarget(continue_jump_target);

	// <inc-expr>
	output_reg = node->IncrementExpression->TranslateExpr(this);
	GetInstructionBuffer()->FreeRegister(output_reg);

	// jmp _start	
	GetInstructionBuffer()->EmitJmp(start_jump_target);

	// _break:
	GetInstructionBuffer()->PatchJmpTarget(break_jump_target);

	PopBreakJumpTarget();
	PopContinueJumpTarget();
}

// =================================================================
//	Translates a if statement.
// =================================================================	
void CVMBinaryTranslator::TranslateIfStatement(CIfStatementASTNode* node)
{
	UpdateDebugTrace(node);

	int else_jump_target = GetInstructionBuffer()->CreateJmpTarget();
	int end_jump_target = GetInstructionBuffer()->CreateJmpTarget();

	// <expr>
	int output_reg = node->ExpressionStatement->TranslateExpr(this);

	// jmponfalse expr_result, _else
	if (node->ElseStatement != NULL)
	{
		GetInstructionBuffer()->EmitJmpOnFalse(output_reg, else_jump_target);
	}
	else
	{
		GetInstructionBuffer()->EmitJmpOnFalse(output_reg, end_jump_target);
	}
	GetInstructionBuffer()->FreeRegister(output_reg);

	// <body>
	node->BodyStatement->Translate(this);
	
	if (node->ElseStatement != NULL)
	{
		// jmp _end
		GetInstructionBuffer()->EmitJmp(end_jump_target);

		// _else:
		GetInstructionBuffer()->PatchJmpTarget(else_jump_target);

		// <else-body>
		node->ElseStatement->Translate(this);
	}

	// _end:
	GetInstructionBuffer()->PatchJmpTarget(end_jump_target);
}

// =================================================================
//	Translates a return statement.
// =================================================================	
void CVMBinaryTranslator::TranslateReturnStatement(CReturnStatementASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* expr = dynamic_cast<CExpressionBaseASTNode*>(node->ReturnExpression);
	int return_reg = -1;

	if (expr != NULL)
	{
		return_reg = expr->TranslateExpr(this);
		GetInstructionBuffer()->EmitRet(expr->ExpressionResultType, return_reg);
	}
	else
	{
		GetInstructionBuffer()->EmitRet();
	}
}

// =================================================================
//	Translates a switch statement.
// =================================================================	
void CVMBinaryTranslator::TranslateSwitchStatement(CSwitchStatementASTNode* node)
{
	UpdateDebugTrace(node);

	// <expr>

	// <case-expr-1>
	// cmp expr_reg, case_reg
	// jmpontrue expr_reg, _body
	// <case-expr-2>
	// cmp expr_reg, case_reg
	// jmpontrue expr_reg, _body
	// jmp _next
	// _body:
	// <case-body>
	// jmp _end
	// _next:

	// <default-body>

	// _end:

	int end_jmp_target = PushBreakJmpTarget();

	int expr_reg = node->ExpressionStatement->TranslateExpr(this);

	// Skip first child, thats the expression.
	std::vector<CASTNode*>::iterator iterBegin = node->Children.begin() + 1;

	for (std::vector<CASTNode*>::iterator iter = iterBegin; iter != node->Children.end(); iter++)
	{
		CCaseStatementASTNode* caseStmt = dynamic_cast<CCaseStatementASTNode*>(*iter);
		CDefaultStatementASTNode* defaultStmt = dynamic_cast<CDefaultStatementASTNode*>(*iter);

		if (caseStmt != NULL)
		{
			int next_jmp_target = GetInstructionBuffer()->CreateJmpTarget();
			int body_jmp_target = GetInstructionBuffer()->CreateJmpTarget();

			for (std::vector<CASTNode*>::iterator exprIter = caseStmt->Expressions.begin(); exprIter != caseStmt->Expressions.end(); exprIter++)
			{
				CExpressionBaseASTNode* expr = dynamic_cast<CExpressionBaseASTNode*>(*exprIter);
				
				// <case-expr-1>
				int case_expr_reg = expr->TranslateExpr(this);

				// cmp expr_reg, case_reg
				GetInstructionBuffer()->EmitCmpEQ(expr->ExpressionResultType, case_expr_reg, expr_reg);

				// jmpontrue expr_reg, _body
				GetInstructionBuffer()->EmitJmpOnTrue(case_expr_reg, body_jmp_target);

				GetInstructionBuffer()->FreeRegister(case_expr_reg);
			}
		
			// jmp _next
			GetInstructionBuffer()->EmitJmp(next_jmp_target);

			// _body:
			GetInstructionBuffer()->PatchJmpTarget(body_jmp_target);

			// <body>			
			caseStmt->BodyStatement->Translate(this);

			// jmp _end
			GetInstructionBuffer()->EmitJmp(end_jmp_target);

			// _next:
			GetInstructionBuffer()->PatchJmpTarget(next_jmp_target);
		}
		else if (defaultStmt != NULL)
		{
			defaultStmt->BodyStatement->Translate(this);
		}
		else
		{
			DBG_ASSERT(false);
		}
	}

	// _end:
	GetInstructionBuffer()->PatchJmpTarget(end_jmp_target);
	PopBreakJumpTarget();

	GetInstructionBuffer()->FreeRegister(expr_reg);
}

// =================================================================
//	Translates a while statement.
// =================================================================	
void CVMBinaryTranslator::TranslateWhileStatement(CWhileStatementASTNode* node)
{
	UpdateDebugTrace(node);

	int break_jump_target = PushBreakJmpTarget();
	int continue_jump_target = PushContinueJmpTarget();

	// _start:
	GetInstructionBuffer()->PatchJmpTarget(continue_jump_target);

	// <expr>
	int output_reg = node->ExpressionStatement->TranslateExpr(this);

	// jmponfalse expr_result, _start
	GetInstructionBuffer()->EmitJmpOnFalse(output_reg, break_jump_target);
	GetInstructionBuffer()->FreeRegister(output_reg);
	
	// <body>
	node->BodyStatement->Translate(this);

	// jmp _start
	GetInstructionBuffer()->EmitJmp(continue_jump_target);

	// _break:
	GetInstructionBuffer()->PatchJmpTarget(break_jump_target);

	PopBreakJumpTarget();
	PopContinueJumpTarget();
}

// =================================================================
//	Translates an expression statement.
// =================================================================
void CVMBinaryTranslator::TranslateExpressionStatement(CExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	int reg = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->TranslateExpr(this);

	// Do not care about result of top-level expressions.
	GetInstructionBuffer()->FreeRegister(reg);
}

// =================================================================
//	Translates an expression.
// =================================================================
int CVMBinaryTranslator::TranslateExpression(CExpressionASTNode* node)
{
	return dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->TranslateExpr(this);
}

// =================================================================
//	Translates a binary math expression.
// =================================================================
int CVMBinaryTranslator::TranslateBinaryMathExpression(CBinaryMathExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CDataType* left_dt = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->ExpressionResultType;

	int lreg = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->TranslateExpr(this);
	int rreg = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue)->TranslateExpr(this);

	switch (node->Token.Type)
	{
	case TokenIdentifier::OP_OR:		GetInstructionBuffer()->EmitOr (lreg, rreg);			break;
	case TokenIdentifier::OP_AND:		GetInstructionBuffer()->EmitAnd (lreg, rreg);			break;
	case TokenIdentifier::OP_XOR:		GetInstructionBuffer()->EmitXOr(lreg, rreg);			break;
	case TokenIdentifier::OP_SHL:		GetInstructionBuffer()->EmitSHL(lreg, rreg);			break;
	case TokenIdentifier::OP_SHR:		GetInstructionBuffer()->EmitSHR(lreg, rreg);			break;
	case TokenIdentifier::OP_MOD:		GetInstructionBuffer()->EmitMod(lreg, rreg);			break;
	case TokenIdentifier::OP_ADD:		GetInstructionBuffer()->EmitAdd(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_SUB:		GetInstructionBuffer()->EmitSub(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_MUL:		GetInstructionBuffer()->EmitMul(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_DIV:		GetInstructionBuffer()->EmitDiv(left_dt, lreg, rreg);	break;
	default:
		{
			DBG_ASSERT(false);
		}
	}

	GetInstructionBuffer()->FreeRegister(rreg);

	return lreg;
}

// =================================================================
//	Translates a comparison expression.
// =================================================================
int CVMBinaryTranslator::TranslateComparisonExpression(CComparisonExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CDataType* left_dt = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->ExpressionResultType;

	int lreg = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->TranslateExpr(this);
	int rreg = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue)->TranslateExpr(this);

	switch (node->Token.Type)
	{			
	case TokenIdentifier::OP_EQUAL:			GetInstructionBuffer()->EmitCmpEQ(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_NOT_EQUAL:		GetInstructionBuffer()->EmitCmpNE(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_GREATER:		GetInstructionBuffer()->EmitCmpG(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_LESS:			GetInstructionBuffer()->EmitCmpL(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_GREATER_EQUAL:	GetInstructionBuffer()->EmitCmpGE(left_dt, lreg, rreg);	break;
	case TokenIdentifier::OP_LESS_EQUAL:	GetInstructionBuffer()->EmitCmpLE(left_dt, lreg, rreg);	break;
	default:
		{
			DBG_ASSERT(false);
		}
	}

	GetInstructionBuffer()->FreeRegister(rreg);

	return lreg;
}

// =================================================================
//	Translates a literal expression.
// =================================================================
int CVMBinaryTranslator::TranslateLiteralExpression(CLiteralExpressionASTNode* node, bool push_to_stack)
{	
	UpdateDebugTrace(node);

	String lit = node->Literal;

	int reg = GetInstructionBuffer()->NewRegister();

	if (dynamic_cast<CBoolDataType*>(node->ExpressionResultType) != NULL)
	{
		bool value = !(lit == "0" || CStringHelper::ToLower(lit) == "false" || lit == "");

		if (push_to_stack)
		{
			GetInstructionBuffer()->EmitPushLiteral(value ? 1 : 0);
		}
		else
		{
			GetInstructionBuffer()->EmitLoadLiteral(reg, value ? 1 : 0);
		}
	}
	else if (dynamic_cast<CIntDataType*>(node->ExpressionResultType) != NULL)
	{
		int value = atoi(lit.c_str());
		if (push_to_stack)
		{
			GetInstructionBuffer()->EmitPushLiteral(value);
		}
		else
		{
			GetInstructionBuffer()->EmitLoadLiteral(reg, value);
		}
	}
	else if (dynamic_cast<CFloatDataType*>(node->ExpressionResultType) != NULL)
	{
		float value = (float)atof(lit.c_str());
		if (push_to_stack)
		{
			GetInstructionBuffer()->EmitPushLiteral(value);
		}
		else
		{
			GetInstructionBuffer()->EmitLoadLiteral(reg, value);
		}
	}
	else if (dynamic_cast<CStringDataType*>(node->ExpressionResultType) != NULL)
	{
		if (push_to_stack)
		{
			GetInstructionBuffer()->EmitPushStringLiteral(node->StringTableIndex);
		}
		else
		{
			GetInstructionBuffer()->EmitLoadStringLiteral(reg, node->StringTableIndex);
		}
	}
	else if (dynamic_cast<CNullDataType*>(node->ExpressionResultType) != NULL)
	{
		if (push_to_stack)
		{
			GetInstructionBuffer()->EmitPushNull();
		}
		else
		{
			GetInstructionBuffer()->EmitLoadNull(reg);
		}
	}
	else
	{
		DBG_ASSERT(false);
	}

	return reg;
}

// =================================================================
//	Translates a comma expression.
// =================================================================
int CVMBinaryTranslator::TranslateCommaExpression(CCommaExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	CExpressionBaseASTNode* right_base  = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue);

	int rreg = right_base->TranslateExpr(this);
	GetInstructionBuffer()->FreeRegister(rreg);

	int lreg = left_base->TranslateExpr(this);

	return lreg;
}

// =================================================================
//	Translates a this expression.
// =================================================================
int CVMBinaryTranslator::TranslateThisExpression(CThisExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);
	int retreg = GetInstructionBuffer()->NewRegister();
	GetInstructionBuffer()->EmitMov(retreg, methodSymbol->ThisRegister, node->ExpressionResultType);
	return retreg;
}

// =================================================================
//	Translates a base expression.
// =================================================================
int CVMBinaryTranslator::TranslateBaseExpression(CBaseExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);
	int retreg = GetInstructionBuffer()->NewRegister();
	GetInstructionBuffer()->EmitMov(retreg, methodSymbol->ThisRegister, node->ExpressionResultType);
	return retreg;
}

// =================================================================
//	Translates a logical expression.
// =================================================================
int CVMBinaryTranslator::TranslateLogicalExpression(CLogicalExpressionASTNode* node)
{	
	UpdateDebugTrace(node);

	// AND:
		// <expr1>
		// jmponfalse cmp_reg, _false
		// <expr2>
		// jmponfalse cmp_reg, _false
		// result_reg = true;
		// jmp _true
		// _false:
		// result_reg = false;
		// _true:
	
	// OR:
		// <expr1>
		// jmpontrue cmp_reg, _true
		// <expr2>
		// jmpontrue cmp_reg, _true
		// result_reg = false;
		// jmp _false
		// _true:
		// result_reg = true;
		// _false:
	
	int first_jump_target = GetInstructionBuffer()->CreateJmpTarget();
	int second_jump_target = GetInstructionBuffer()->CreateJmpTarget();

	bool is_and = (node->Token.Type == TokenIdentifier::OP_LOGICAL_AND);

	// <expr1>
	int lreg = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue)->TranslateExpr(this);

	if (is_and == true)
	{
		// jmponfalse cmp_reg, _false
		GetInstructionBuffer()->EmitJmpOnFalse(lreg, first_jump_target);
	}
	else
	{
		// jmpontrue cmp_reg, _false
		GetInstructionBuffer()->EmitJmpOnTrue(lreg, first_jump_target);
	}

	GetInstructionBuffer()->FreeRegister(lreg);
	
	// <expr2>
	int rreg = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue)->TranslateExpr(this);
	
	if (is_and == true)
	{
		// jmponfalse cmp_reg, _false
		GetInstructionBuffer()->EmitJmpOnFalse(lreg, first_jump_target);
	}
	else
	{
		// jmpontrue cmp_reg, _false
		GetInstructionBuffer()->EmitJmpOnTrue(lreg, first_jump_target);
	}
	
	GetInstructionBuffer()->FreeRegister(rreg);
	
	int result_reg = GetInstructionBuffer()->NewRegister();
	
	if (is_and == true)
	{
		// iloadliteral result, 1
		GetInstructionBuffer()->EmitLoadLiteral(result_reg, 1);
	}
	else
	{
		// iloadliteral result, 0
		GetInstructionBuffer()->EmitLoadLiteral(result_reg, 0);
	}

	// jmp _true
	GetInstructionBuffer()->EmitJmp(second_jump_target);

	// _first:
	GetInstructionBuffer()->PatchJmpTarget(first_jump_target);
	
	if (is_and == true)
	{
		// iloadliteral result, 0
		GetInstructionBuffer()->EmitLoadLiteral(result_reg, 0);
	}
	else
	{
		// iloadliteral result, 1
		GetInstructionBuffer()->EmitLoadLiteral(result_reg, 1);
	}
	
	// _second:
	GetInstructionBuffer()->PatchJmpTarget(second_jump_target);
	
	return result_reg;
}

// =================================================================
//	Translates a ternary expression.
// =================================================================
int CVMBinaryTranslator::TranslateTernaryExpression(CTernaryExpressionASTNode* node)
{	
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	CExpressionBaseASTNode* right_base = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue);
	CExpressionBaseASTNode* expr_base  = dynamic_cast<CExpressionBaseASTNode*>(node->Expression);
	
	int first_jump_target = GetInstructionBuffer()->CreateJmpTarget();
	int second_jump_target = GetInstructionBuffer()->CreateJmpTarget();

	// <expr>
	int ereg = expr_base->TranslateExpr(this);
	// ereg=result_reg as well.

	// jmpiffalse ereg, _first
	GetInstructionBuffer()->EmitJmpOnFalse(ereg, first_jump_target);
	
	// <expr-1>
	int lreg = left_base->TranslateExpr(this);

	// mov result_reg, lreg
	GetInstructionBuffer()->EmitMov(ereg, lreg, left_base->ExpressionResultType);
	GetInstructionBuffer()->FreeRegister(lreg);

	// jmp _second
	GetInstructionBuffer()->EmitJmp(second_jump_target);

	// _first:
	GetInstructionBuffer()->PatchJmpTarget(first_jump_target);

	// <expr-2>
	int rreg = right_base->TranslateExpr(this);

	// mov result_reg, rreg
	GetInstructionBuffer()->EmitMov(ereg, rreg, right_base->ExpressionResultType);
	GetInstructionBuffer()->FreeRegister(rreg);

	// _second:
	GetInstructionBuffer()->PatchJmpTarget(second_jump_target);

	return ereg;
}

// =================================================================
//	Translates a cast expression.
// =================================================================
int CVMBinaryTranslator::TranslateCastExpression(CCastExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* expr = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue);
	int expr_reg = expr->TranslateExpr(this);

	CDataType* fromType = expr->ExpressionResultType;
	CDataType* toType   = node->Type;

	// Object->String calls ToString.
	CObjectDataType* obj_from_type = dynamic_cast<CObjectDataType*>(fromType);
	CArrayDataType* array_from_type = dynamic_cast<CArrayDataType*>(fromType);
	CStringDataType* string_to_type = dynamic_cast<CStringDataType*>(toType);

	CDeclarationIdentifier s_tostring_ident("ToString");

	if (obj_from_type != NULL && string_to_type != NULL)
	{
		std::vector<CDataType*> args;
		CClassMemberASTNode* tostring_member = obj_from_type->GetClass(m_semanter)->FindClassMethod(m_semanter, s_tostring_ident, args, true);
		DBG_ASSERT(tostring_member != NULL);
		
		GetInstructionBuffer()->EmitInvokeMethod(expr_reg, expr_reg, tostring_member->TranslatedSymbol->Index);
	}
	else if (array_from_type != NULL && string_to_type != NULL)
	{
		std::vector<CDataType*> args;
		CClassMemberASTNode* tostring_member = array_from_type->GetClass(m_semanter)->FindClassMethod(m_semanter, s_tostring_ident, args, true);
		DBG_ASSERT(tostring_member != NULL);
		
		GetInstructionBuffer()->EmitInvokeMethod(expr_reg, expr_reg, tostring_member->TranslatedSymbol->Index);
	}
	else
	{
		GetInstructionBuffer()->EmitCast(expr_reg, fromType, toType, !node->ExceptionOnFail);
	}

	return expr_reg;
}

// =================================================================
//	Translates a function argument list. The arguments are pushed
//  onto the stack.
// =================================================================
void CVMBinaryTranslator::TranslateArguments(std::vector<CASTNode*>& nodes)
{
	for (std::vector<CASTNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		CExpressionBaseASTNode* expr = dynamic_cast<CExpressionBaseASTNode*>(*iter);

		// Optimization:
		//	If we have a literal argument, just push it straight to the stack, rather than load/push.
		CLiteralExpressionASTNode* lit_expr = dynamic_cast<CLiteralExpressionASTNode*>(expr);
		if (lit_expr != NULL)
		{
			TranslateLiteralExpression(lit_expr, true);
		}
		else
		{
			int reg = expr->TranslateExpr(this);
			GetInstructionBuffer()->EmitPush(expr->ExpressionResultType, reg);
			GetInstructionBuffer()->FreeRegister(reg);
		}
	}
}

// =================================================================
//	Translates a new expression.
// =================================================================
int CVMBinaryTranslator::TranslateNewExpression(CNewExpressionASTNode* node)
{	
	UpdateDebugTrace(node);

	if (node->IsArray == true)
	{
		if (node->ArrayInitializer != NULL)
		{
			return node->ArrayInitializer->TranslateExpr(this);
		}
		else
		{
			CExpressionASTNode* expr = dynamic_cast<CExpressionASTNode*>(node->ArgumentExpressions.at(0));
			CArrayDataType* arrayType = dynamic_cast<CArrayDataType*>(node->DataType);
			CDataType* element_type = arrayType->ElementType;

			int ret_reg = expr->TranslateExpr(this);
			
			GetInstructionBuffer()->EmitNewArray(ret_reg, ret_reg, arrayType->GetClass(m_semanter)->TranslatedSymbol->Index);

			return ret_reg;
		}
	}
	else
	{
		// Create new object.
		int ret_reg = GetInstructionBuffer()->NewRegister();

		GetInstructionBuffer()->EmitNew(ret_reg, node->DataType->GetClass(m_semanter)->TranslatedSymbol->Index);

		// Push construction arguments.
		TranslateArguments(node->ArgumentExpressions);

		// Invoke method.
		GetInstructionBuffer()->EmitInvokeMethod(ret_reg, ret_reg, node->ResolvedConstructor->TranslatedSymbol->Index);

		return ret_reg;
	}

	return -1;
}

// =================================================================
//	Translates a type expression.
// =================================================================
int CVMBinaryTranslator::TranslateTypeExpression(CTypeExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* expr = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	int expr_reg = expr->TranslateExpr(this);

	switch (node->Token.Type)
	{
		case TokenIdentifier::KEYWORD_IS:
			{		
				CDataType*		 fromType	= expr->ExpressionResultType;
				CObjectDataType* toType		= NULL;

				if (dynamic_cast<CBoolDataType*>(node->Type)	!= NULL ||	
					dynamic_cast<CIntDataType*>(node->Type)		!= NULL ||		
					dynamic_cast<CFloatDataType*>(node->Type)	!= NULL ||	
					dynamic_cast<CStringDataType*>(node->Type)	!= NULL ||	
					dynamic_cast<CArrayDataType*>(node->Type)	!= NULL)
				{
					toType = node->Type->GetBoxClass(m_semanter)->ObjectDataType;
				}
				else if (dynamic_cast<CObjectDataType*>(node->Type)	!= NULL)	
				{
					toType = node->Type->GetClass(m_semanter)->ObjectDataType;
				}

				// Converting interface to object.
				if (fromType->GetClass(m_semanter)->IsInterface == true &&
					toType->GetClass(m_semanter)->IsInterface == false)
				{
					int comp_reg = GetInstructionBuffer()->NewRegister();

					// Cast to new type.
					GetInstructionBuffer()->EmitCast(comp_reg, fromType, toType, false);
					
					// oLoadNull
					GetInstructionBuffer()->EmitLoadNull(comp_reg);

					// oCmpNE
					GetInstructionBuffer()->EmitCmpNE(toType, expr_reg, comp_reg);

					GetInstructionBuffer()->FreeRegister(comp_reg);
				}

				// Upcasting
				else if (fromType->GetClass(m_semanter)->InheritsFromClass(m_semanter, toType->GetClass(m_semanter)))
				{
					int comp_reg = GetInstructionBuffer()->NewRegister();

					// oLoadNull
					GetInstructionBuffer()->EmitLoadNull(comp_reg);

					// oCmpNE
					GetInstructionBuffer()->EmitCmpNE(toType, expr_reg, comp_reg);

					GetInstructionBuffer()->FreeRegister(comp_reg);
				}

				// Downcasting
				else
				{
					int comp_reg = GetInstructionBuffer()->NewRegister();

					// Cast to new type.
					GetInstructionBuffer()->EmitCast(comp_reg, fromType, toType, false);
					
					// oLoadNull
					GetInstructionBuffer()->EmitLoadNull(comp_reg);

					// oCmpNE
					GetInstructionBuffer()->EmitCmpNE(toType, expr_reg, comp_reg);

					GetInstructionBuffer()->FreeRegister(comp_reg);
				}

				break;
			}
		case TokenIdentifier::KEYWORD_AS:
			{
				// Nothing to do for this.
				// The parser generates a cast instruction as part of the lvalue expression to do this.
				break;
			}
	}

	return expr_reg;
}

// =================================================================
//	Translates a foreach statement.
// =================================================================	
void CVMBinaryTranslator::TranslateForEachStatement(CForEachStatementASTNode* node)
{
	UpdateDebugTrace(node);

	// We can do some optimization for arrays.
	CArrayDataType* adt = dynamic_cast<CArrayDataType*>(node->ExpressionStatement->ExpressionResultType);
	if (adt != NULL)
	{
		UpdateDebugTrace(node);

		int break_jump_target = PushBreakJmpTarget();
		int continue_jump_target = PushContinueJmpTarget();
		int start_jump_target = GetInstructionBuffer()->CreateJmpTarget();

		// Downside: Modifying array mid-foreach is not allowed :(. 
		// TODO: Compile error for this.

		// array_reg		= <inter-expr>
		// length_reg		= array_reg->Length
		// iterator_reg		= 0
		// _start:
		//		mov			comp_reg,	iterator_reg
		//		cmpl		comp_reg,	length_reg
		//		jmponfalse	comp_reg,	_break
		//		getindex	comp_reg,	array_reg,		iterator_reg
		//		cast		comp_reg,	element_type,	var_type
		//		cmpnull		comp_reg
		//		jmpontrue	comp_reg,	_continue
		//		storelocal	comp_reg,	local_index
		//		<body>
		// _continue:
		//		inc			iterator_reg
		//		jmp			_start
		// _break:
		//

		// array_reg		= <inter-expr>
		int array_reg = node->ExpressionStatement->TranslateExpr(this);

		// length_reg		= array_reg->Length
		int length_reg = GetInstructionBuffer()->NewRegister();
		GetInstructionBuffer()->EmitGetLength(length_reg, array_reg);

		// iterator_reg = 0
		int iterator_reg = GetInstructionBuffer()->NewRegister();
		GetInstructionBuffer()->EmitLoadLiteral(iterator_reg, 0);

		// _start:
		GetInstructionBuffer()->PatchJmpTarget(start_jump_target);

		// mov			comp_reg,	iterator_reg
		int comp_reg = GetInstructionBuffer()->NewRegister();
		GetInstructionBuffer()->EmitMov(comp_reg, iterator_reg, CIntDataType::StaticInstance);

		// cmpl			comp_reg,	length_reg
		GetInstructionBuffer()->EmitCmpL(CIntDataType::StaticInstance, comp_reg, length_reg);

		// jmponfalse expr_result, _break
		GetInstructionBuffer()->EmitJmpOnFalse(comp_reg, break_jump_target);

		// getindex	comp_reg,	array_reg, iterator_reg
		GetInstructionBuffer()->EmitGetIndex(comp_reg, array_reg, iterator_reg);

		// cast		comp_reg,	element_type,	var_type
		CVariableStatementASTNode* varExpr = dynamic_cast<CVariableStatementASTNode*>(node->VariableStatement);
		GetInstructionBuffer()->EmitCast(comp_reg, adt->ElementType, varExpr->Type, true);

		// null check.
		CDataType* var_type = varExpr->Type;
		if (dynamic_cast<CObjectDataType*>(var_type) != NULL)
		{
			// cmpnull comp_reg
			GetInstructionBuffer()->EmitCmpNull(CIntDataType::StaticInstance, comp_reg);

			// jmpontrue comp_reg, _break
			GetInstructionBuffer()->EmitJmpOnTrue(comp_reg, continue_jump_target);
		}

		// storelocal comp_reg, local_Index
		GetInstructionBuffer()->EmitStoreLocal(comp_reg, varExpr->TranslatedSymbol);

		// <body>
		node->OriginalBodyStatement->Translate(this);

		// _continue:
		GetInstructionBuffer()->PatchJmpTarget(continue_jump_target);

		//		<inc-expr>
		GetInstructionBuffer()->EmitInc(iterator_reg);		

		//		jmp _start	
		GetInstructionBuffer()->EmitJmp(start_jump_target);

		// _break:
		GetInstructionBuffer()->PatchJmpTarget(break_jump_target);

		GetInstructionBuffer()->FreeRegister(comp_reg);
		GetInstructionBuffer()->FreeRegister(array_reg);
		GetInstructionBuffer()->FreeRegister(length_reg);
		GetInstructionBuffer()->FreeRegister(iterator_reg);

		PopBreakJumpTarget();
		PopContinueJumpTarget();
	}
	else
	{
		node->BodyStatement->Translate(this);
	}
}

// =================================================================
//	Translates a class ref expression.
// =================================================================
int CVMBinaryTranslator::TranslateClassRefExpression(CClassRefExpressionASTNode* node)
{
	// Class ref dosen't need to return anything, TranslateIdentifier/FieldAccess should
	// deal correctly with these.
	return -1;
}

// =================================================================
//	Translates an assignment expression.
// =================================================================
int CVMBinaryTranslator::TranslateAssignmentExpression(CAssignmentExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	CExpressionBaseASTNode* right_base = dynamic_cast<CExpressionBaseASTNode*>(node->RightValue);

	return TranslateSubAssignmentExpression(&node->Token, left_base, right_base);
}

// =================================================================
//	Translates a sub-assignment.
// =================================================================
int CVMBinaryTranslator::TranslateSubAssignmentExpression(CToken* token, CExpressionBaseASTNode* left_base, CExpressionBaseASTNode* right_base, bool postfix)
{
	CFieldAccessExpressionASTNode* field_access_node = dynamic_cast<CFieldAccessExpressionASTNode*>(left_base);
	CIdentifierExpressionASTNode*  identifier_node   = dynamic_cast<CIdentifierExpressionASTNode*>(left_base);
	CIndexExpressionASTNode*	   index_node		 = dynamic_cast<CIndexExpressionASTNode*>(left_base);

	// Work out RHS assignment.
	int rhs_reg = right_base == NULL ? -1 : right_base->TranslateExpr(this);

	bool return_old_value = false;
	int old_val_reg = -1;

	if ((token->Type == TokenIdentifier::OP_INCREMENT || token->Type == TokenIdentifier::OP_DECREMENT) &&
		postfix == false)
	{
		return_old_value = true;
	}

	// Assign an index
	if (index_node != NULL)
	{	
		CExpressionBaseASTNode* index_left_base = dynamic_cast<CExpressionBaseASTNode*>(index_node->LeftValue);
		CExpressionBaseASTNode* index_index_base = dynamic_cast<CExpressionBaseASTNode*>(index_node->IndexExpression);

		CDeclarationIdentifier s_operator_ident("__operator[]");

		// Find the setIndex method.
		std::vector<CDataType*> argument_types;
		argument_types.push_back(index_index_base->ExpressionResultType);
		argument_types.push_back(left_base->ExpressionResultType);
		int set_index_symbol_index = index_left_base->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_operator_ident, argument_types, false)->TranslatedSymbol->Index;

		// Parse object expression.
		int lhs_reg = index_left_base->TranslateExpr(this);		

		// We need to do some extra work for specific operators.		
		if (token->Type != TokenIdentifier::OP_ASSIGN)
		{
			int value_reg = index_node->TranslateExpr(this);

			// If postfix inc/dec, store old value so we can return it.
			if (return_old_value)
			{
				old_val_reg = GetInstructionBuffer()->NewRegister();
				GetInstructionBuffer()->EmitMov(old_val_reg, value_reg, index_node->ExpressionResultType);
			}

			switch (token->Type)
			{
				case TokenIdentifier::OP_ASSIGN_AND:	GetInstructionBuffer()->EmitAdd(index_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_OR:		GetInstructionBuffer()->EmitOr(value_reg, rhs_reg);											break;
				case TokenIdentifier::OP_ASSIGN_XOR:	GetInstructionBuffer()->EmitXOr(value_reg, rhs_reg);										break;
				case TokenIdentifier::OP_ASSIGN_SHL:	GetInstructionBuffer()->EmitSHL(value_reg, rhs_reg);										break;
				case TokenIdentifier::OP_ASSIGN_SHR:	GetInstructionBuffer()->EmitSHR(value_reg, rhs_reg);										break;
				case TokenIdentifier::OP_ASSIGN_MOD:	GetInstructionBuffer()->EmitMod(value_reg, rhs_reg);										break;
				case TokenIdentifier::OP_ASSIGN_ADD:	GetInstructionBuffer()->EmitAdd(index_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_SUB:	GetInstructionBuffer()->EmitSub(index_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_MUL:	GetInstructionBuffer()->EmitMul(index_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_DIV:	GetInstructionBuffer()->EmitDiv(index_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_INCREMENT:		GetInstructionBuffer()->EmitInc(value_reg);										break;
				case TokenIdentifier::OP_DECREMENT:		GetInstructionBuffer()->EmitDec(value_reg);										break;
			}

			GetInstructionBuffer()->FreeRegister(rhs_reg);
			rhs_reg = value_reg;
		}
		
		// Parse index expression.
		int ihs_reg = index_index_base->TranslateExpr(this);	

		// Push index_reg
		GetInstructionBuffer()->EmitPush(index_index_base->ExpressionResultType, ihs_reg);

		// Push value_reg
		GetInstructionBuffer()->EmitPush(left_base->ExpressionResultType, rhs_reg);

		// InvokeMethod value_reg, array_reg, SetIndex
		GetInstructionBuffer()->EmitInvokeMethod(lhs_reg, lhs_reg, set_index_symbol_index);

		GetInstructionBuffer()->FreeRegister(lhs_reg);
		GetInstructionBuffer()->FreeRegister(ihs_reg);

		// If postfix inc/dec, store old value so we can return it.
		if (return_old_value)
		{
			GetInstructionBuffer()->FreeRegister(rhs_reg);
			rhs_reg = old_val_reg;
		}
	}

	// Assign a field.
	else if (field_access_node != NULL)
	{
		CExpressionBaseASTNode* field_left_base = dynamic_cast<CExpressionBaseASTNode*>(field_access_node->LeftValue);
		CExpressionBaseASTNode* field_right_base = dynamic_cast<CExpressionBaseASTNode*>(field_access_node->RightValue);

		// We need to do some extra work for specific operators.		
		if (token->Type != TokenIdentifier::OP_ASSIGN)
		{
			int value_reg = field_access_node->TranslateExpr(this);
			
			// If postfix inc/dec, store old value so we can return it.
			if (return_old_value)
			{
				old_val_reg = GetInstructionBuffer()->NewRegister();
				GetInstructionBuffer()->EmitMov(old_val_reg, value_reg, field_access_node->ExpressionResultType);
			}

			switch (token->Type)
			{
				case TokenIdentifier::OP_ASSIGN_AND:	GetInstructionBuffer()->EmitAdd(field_access_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_OR:		GetInstructionBuffer()->EmitOr(value_reg, rhs_reg);													break;
				case TokenIdentifier::OP_ASSIGN_XOR:	GetInstructionBuffer()->EmitXOr(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_SHL:	GetInstructionBuffer()->EmitSHL(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_SHR:	GetInstructionBuffer()->EmitSHR(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_MOD:	GetInstructionBuffer()->EmitMod(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_ADD:	GetInstructionBuffer()->EmitAdd(field_access_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_SUB:	GetInstructionBuffer()->EmitSub(field_access_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_MUL:	GetInstructionBuffer()->EmitMul(field_access_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_ASSIGN_DIV:	GetInstructionBuffer()->EmitDiv(field_access_node->ExpressionResultType, value_reg, rhs_reg);		break;
				case TokenIdentifier::OP_INCREMENT:		GetInstructionBuffer()->EmitInc(value_reg);												break;
				case TokenIdentifier::OP_DECREMENT:		GetInstructionBuffer()->EmitDec(value_reg);												break;
			}

			GetInstructionBuffer()->FreeRegister(rhs_reg);
			rhs_reg = value_reg;
		}

		// Get object reference and store result.
		int lhs_reg = field_left_base->TranslateExpr(this);
		
		if (field_access_node->IsPropertyAccess)
		{
			GetInstructionBuffer()->EmitPush(field_access_node->ExpressionResultType, rhs_reg);
			GetInstructionBuffer()->EmitInvokeMethod(lhs_reg, lhs_reg, field_access_node->Property_SetMethod->TranslatedSymbol->Index);
		}
		else
		{
			GetInstructionBuffer()->EmitStoreField(rhs_reg, lhs_reg, field_access_node->ExpressionResultClassMember->TranslatedSymbol);
		}
		
		GetInstructionBuffer()->FreeRegister(lhs_reg);

		// If postfix inc/dec, store old value so we can return it.
		if (return_old_value)
		{
			GetInstructionBuffer()->FreeRegister(rhs_reg);
			rhs_reg = old_val_reg;
		}
	}

	// Assign an identifier
	else
	{
		CSymbol* member_scope = GetParentScope(SymbolType::ClassMethod);

		CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(identifier_node->ResolvedDeclaration);
		CVariableStatementASTNode* variable = dynamic_cast<CVariableStatementASTNode*>(identifier_node->ResolvedDeclaration);
		
		// We need to do some extra work for specific operators.		
		if (token->Type != TokenIdentifier::OP_ASSIGN)
		{
			int value_reg = identifier_node->TranslateExpr(this);
			
			// If postfix inc/dec, store old value so we can return it.
			if (return_old_value)
			{
				old_val_reg = GetInstructionBuffer()->NewRegister();
				GetInstructionBuffer()->EmitMov(old_val_reg, value_reg, index_node->ExpressionResultType);
			}

			switch (token->Type)
			{
				case TokenIdentifier::OP_ASSIGN_AND:	GetInstructionBuffer()->EmitAdd(identifier_node->ExpressionResultType, value_reg, rhs_reg);			break;
				case TokenIdentifier::OP_ASSIGN_OR:		GetInstructionBuffer()->EmitOr(value_reg, rhs_reg);													break;
				case TokenIdentifier::OP_ASSIGN_XOR:	GetInstructionBuffer()->EmitXOr(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_SHL:	GetInstructionBuffer()->EmitSHL(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_SHR:	GetInstructionBuffer()->EmitSHR(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_MOD:	GetInstructionBuffer()->EmitMod(value_reg, rhs_reg);												break;
				case TokenIdentifier::OP_ASSIGN_ADD:	GetInstructionBuffer()->EmitAdd(identifier_node->ExpressionResultType, value_reg, rhs_reg);			break;
				case TokenIdentifier::OP_ASSIGN_SUB:	GetInstructionBuffer()->EmitSub(identifier_node->ExpressionResultType, value_reg, rhs_reg);			break;
				case TokenIdentifier::OP_ASSIGN_MUL:	GetInstructionBuffer()->EmitMul(identifier_node->ExpressionResultType, value_reg, rhs_reg);			break;
				case TokenIdentifier::OP_ASSIGN_DIV:	GetInstructionBuffer()->EmitDiv(identifier_node->ExpressionResultType, value_reg, rhs_reg);			break;
				case TokenIdentifier::OP_INCREMENT:		GetInstructionBuffer()->EmitInc(value_reg);												break;
				case TokenIdentifier::OP_DECREMENT:		GetInstructionBuffer()->EmitDec(value_reg);												break;
			}

			GetInstructionBuffer()->FreeRegister(rhs_reg);
			rhs_reg = value_reg;
		}

		// Static field? / Instance Field?
		if (member != NULL)
		{
			GetInstructionBuffer()->EmitStoreField(rhs_reg, member_scope->ThisRegister, member->TranslatedSymbol);
		}

		// Local variable?
		else
		{
			GetInstructionBuffer()->EmitStoreLocal(rhs_reg, variable->TranslatedSymbol);
		}

		// If postfix inc/dec, store old value so we can return it.
		if (return_old_value)
		{
			GetInstructionBuffer()->FreeRegister(rhs_reg);
			rhs_reg = old_val_reg;
		}
	}

	// By the end of the assignment rhs should hold the "new-value" after assignment.
	return rhs_reg;
}

// =================================================================
//	Translates a prefix expression.
// =================================================================
int CVMBinaryTranslator::TranslatePreFixExpression(CPreFixExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);

	switch (node->Token.Type)
	{	
		case TokenIdentifier::OP_NOT:			
			{
				int reg = left_base->TranslateExpr(this);
				GetInstructionBuffer()->EmitNot(reg);
				return reg;
			}
		case TokenIdentifier::OP_LOGICAL_NOT:	
			{
				int reg = left_base->TranslateExpr(this);
				GetInstructionBuffer()->EmitLogicalNot(reg);
				return reg;
			}

		case TokenIdentifier::OP_ADD:			
			{
				int reg = left_base->TranslateExpr(this);
				GetInstructionBuffer()->EmitPos(left_base->ExpressionResultType, reg);
				return reg;
			}
		case TokenIdentifier::OP_SUB:			
			{
				int reg = left_base->TranslateExpr(this);
				GetInstructionBuffer()->EmitNeg(left_base->ExpressionResultType, reg);
				return reg;
			}
		case TokenIdentifier::OP_DECREMENT:		
		case TokenIdentifier::OP_INCREMENT:		
			{
				return TranslateSubAssignmentExpression(&node->Token, left_base, NULL, false);
			}
	}

	return -1;
}

// =================================================================
//	Translates a postfix expression.
// =================================================================
int CVMBinaryTranslator::TranslatePostFixExpression(CPostFixExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);

	switch (node->Token.Type)
	{	
		case TokenIdentifier::OP_DECREMENT:		
		case TokenIdentifier::OP_INCREMENT:		
			{
				return TranslateSubAssignmentExpression(&node->Token, left_base, NULL, true);
			}
	}

	return -1;
}

// =================================================================
//	Translates a field access expression.
// =================================================================
int CVMBinaryTranslator::TranslateFieldAccessExpression(CFieldAccessExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	CIdentifierExpressionASTNode* right_base = dynamic_cast<CIdentifierExpressionASTNode*>(node->RightValue);
	CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(right_base->ResolvedDeclaration);

	CClassReferenceDataType* class_ref = dynamic_cast<CClassReferenceDataType*>(left_base->ExpressionResultType);
	
	int lreg = -1;

	if (member->IsStatic == true && class_ref != NULL)
	{
		lreg = GetInstructionBuffer()->NewRegister();

		CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);

		// Load static data.
		if (node->IsPropertyAccess == true)
		{
			GetInstructionBuffer()->EmitInvokeMethod(lreg, lreg, node->Property_GetMethod->TranslatedSymbol->Index);
		}
		else
		{
			GetInstructionBuffer()->EmitLoadField(lreg, lreg, member->TranslatedSymbol);
		}
	}
	else
	{
		lreg = left_base->TranslateExpr(this);

		CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);

		// Load local data.
		if (node->IsPropertyAccess == true)
		{
			GetInstructionBuffer()->EmitInvokeMethod(lreg, lreg, node->Property_GetMethod->TranslatedSymbol->Index);		
		}
		else
		{
			GetInstructionBuffer()->EmitLoadField(lreg, lreg, member->TranslatedSymbol);
		}
	}

	return lreg;
}

// =================================================================
//	Translates a identifier expression.
// =================================================================
int CVMBinaryTranslator::TranslateIdentifierExpression(CIdentifierExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(node->ResolvedDeclaration);
	CVariableStatementASTNode* variable = dynamic_cast<CVariableStatementASTNode*>(node->ResolvedDeclaration);
	
	if (member != NULL)
	{
		CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);

		int ret_reg = GetInstructionBuffer()->NewRegister();
		
		// Load local data.
		GetInstructionBuffer()->EmitLoadField(ret_reg, methodSymbol->ThisRegister, member->TranslatedSymbol);

		return ret_reg;
	}
	else if (variable != NULL)
	{
		int ret_reg = GetInstructionBuffer()->NewRegister();

		// Load local data.
		GetInstructionBuffer()->EmitLoadLocal(ret_reg, variable->TranslatedSymbol);

		return ret_reg;
	}
	else
	{
		return -1;
	}

	return -1;
}

// =================================================================
//	Translates a method call expression.
// =================================================================
int CVMBinaryTranslator::TranslateMethodCallExpression(CMethodCallExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	int lreg = left_base->TranslateExpr(this);

	CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(node->ResolvedDeclaration);
	
	CBaseExpressionASTNode* base_expr = dynamic_cast<CBaseExpressionASTNode*>(left_base);
	
	// Push RPC arguments.
	if (node->RPCExceptExpression != NULL)
	{
		int rpc_target = node->RPCTargetExpression == NULL ? -1 : node->RPCTargetExpression->Evaluate(GetContext()).GetInt();
		int rpc_except = node->RPCExceptExpression == NULL ? -1 : node->RPCExceptExpression->Evaluate(GetContext()).GetInt();

		int tmp_reg = GetInstructionBuffer()->NewRegister();

		GetInstructionBuffer()->EmitLoadLiteral(tmp_reg, rpc_target);
		GetInstructionBuffer()->EmitPush(CIntDataType::StaticInstance, tmp_reg);

		GetInstructionBuffer()->EmitLoadLiteral(tmp_reg, rpc_except);
		GetInstructionBuffer()->EmitPush(CIntDataType::StaticInstance, tmp_reg);

		GetInstructionBuffer()->FreeRegister(tmp_reg);
	}

	// Push construction arguments.
	TranslateArguments(node->ArgumentExpressions);

	// Static call.
	if (member->IsStatic == true)
	{
		// register is null if its a class-ref on the lvalue. So create one for our
		// return value to go in.
		if (lreg < 0)
		{
			lreg = GetInstructionBuffer()->NewRegister();
		}

		GetInstructionBuffer()->EmitInvokeStatic(lreg, member->TranslatedSymbol->Index);
	}

	// Base instance call.
	else if (base_expr != NULL)
	{
		CSymbol* methodSymbol = GetParentScope(SymbolType::ClassMethod);

		// Invoke as non-virtual.
		GetInstructionBuffer()->EmitInvokeNonVirtualMethod(lreg, lreg, member->TranslatedSymbol->Index);
	}

	// Instance call.
	else
	{
		// If this is an interface method we need to cast to interface first! If we don't
		// then we won't have the correct vf-table offset to call it.
		if (member->IsOverride == true)
		{
			CClassASTNode* interface_class = member->BaseVirtualMethod->FindClassScope(m_semanter);
			if (interface_class->IsInterface == true)
			{
				GetInstructionBuffer()->EmitCast(lreg, left_base->ExpressionResultType, interface_class->ObjectDataType, true);

				// We always call the base-method for interfaces, so we have the non-offset vf-table index.
				member = member->BaseVirtualMethod;
			}
		}

		GetInstructionBuffer()->EmitInvokeMethod(lreg, lreg, member->TranslatedSymbol->Index);
	}

	return lreg;
}

// =================================================================
//	Translates a array initializer expression.
// =================================================================
int CVMBinaryTranslator::TranslateArrayInitializerExpression(CArrayInitializerASTNode* node)
{
	UpdateDebugTrace(node);

	CArrayDataType* array_type = static_cast<CArrayDataType*>(node->ExpressionResultType);
	CDataType* element_type = array_type->ElementType;

	// iLoadLiteral array_reg, size
	int array_reg = GetInstructionBuffer()->NewRegister();
	GetInstructionBuffer()->EmitLoadLiteral(array_reg, (int)node->Expressions.size());

	// newArray array_reg
	GetInstructionBuffer()->EmitNewArray(array_reg, array_reg, array_type->GetClass(m_semanter)->TranslatedSymbol->Index);

	// Find the setIndex method.
	std::vector<CDataType*> argument_types;
	argument_types.push_back(CIntDataType::StaticInstance);
	argument_types.push_back(element_type);

	CDeclarationIdentifier s_operator_ident("__operator[]");

	int set_index_symbol_index = node->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_operator_ident, argument_types, false)->TranslatedSymbol->Index;

	int index = 0;
	for (std::vector<CASTNode*>::iterator iter = node->Expressions.begin(); iter != node->Expressions.end(); iter++, index++)
	{
		CExpressionBaseASTNode* expr = dynamic_cast<CExpressionBaseASTNode*>(*iter);

		// <expr>
		int value_reg = expr->TranslateExpr(this);
		
		// Push index.
		int index_reg = GetInstructionBuffer()->NewRegister();
		GetInstructionBuffer()->EmitLoadLiteral(index_reg, index);
		GetInstructionBuffer()->EmitPush(CIntDataType::StaticInstance, index_reg);

		// Push value_reg
		GetInstructionBuffer()->EmitPush(expr->ExpressionResultType, value_reg);

		// InvokeMethod value_reg, array_reg, SetIndex
		GetInstructionBuffer()->EmitInvokeMethod(value_reg, array_reg, set_index_symbol_index);
		
		GetInstructionBuffer()->FreeRegister(index_reg);
		GetInstructionBuffer()->FreeRegister(value_reg);
	}

	return array_reg;
}

// =================================================================
//	Translates an index expression.
// =================================================================
int CVMBinaryTranslator::TranslateIndexExpression(CIndexExpressionASTNode* node)
{	
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);
	CExpressionBaseASTNode* index_base  = dynamic_cast<CExpressionBaseASTNode*>(node->IndexExpression);

	// <object-expr>	
	int array_reg = left_base->TranslateExpr(this);
	
	// <expr>
	int expr_reg = index_base->TranslateExpr(this);

	// Push value_reg
	GetInstructionBuffer()->EmitPush(index_base->ExpressionResultType, expr_reg);
	GetInstructionBuffer()->FreeRegister(expr_reg);

	std::vector<CDataType*> argument_types;
	argument_types.push_back(CIntDataType::StaticInstance);

	CDeclarationIdentifier s_operator_ident("__operator[]");

	int set_index_symbol_index = left_base->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_operator_ident, argument_types, false)->TranslatedSymbol->Index;

	// InvokeMethod value_reg, array_reg, GetIndex
	GetInstructionBuffer()->EmitInvokeMethod(array_reg, array_reg, set_index_symbol_index);

	return array_reg;
}

// =================================================================
//	Translates a slice expression.
// =================================================================
int CVMBinaryTranslator::TranslateSliceExpression(CSliceExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	CExpressionBaseASTNode* start_base  = dynamic_cast<CExpressionBaseASTNode*>(node->StartExpression);
	CExpressionBaseASTNode* end_base  = dynamic_cast<CExpressionBaseASTNode*>(node->EndExpression);
	CExpressionBaseASTNode* left_base  = dynamic_cast<CExpressionBaseASTNode*>(node->LeftValue);

	// COOOOOOOOOOOOOODE DUPLICATION
	// Fix plz

	if (start_base != NULL && end_base == NULL)
	{
		// <object-expr>	
		int array_reg = left_base->TranslateExpr(this);
	
		// <expr>
		int expr_reg = start_base->TranslateExpr(this);

		// Push value_reg
		GetInstructionBuffer()->EmitPush(start_base->ExpressionResultType, expr_reg);
		GetInstructionBuffer()->FreeRegister(expr_reg);

		CDeclarationIdentifier s_getslice_ident("GetSlice");

		std::vector<CDataType*> argument_types;
		argument_types.push_back(CIntDataType::StaticInstance);
		int get_slice_symbol_index = left_base->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_getslice_ident, argument_types, true)->TranslatedSymbol->Index;
		
		// InvokeMethod value_reg, array_reg, GetSlice
		GetInstructionBuffer()->EmitInvokeMethod(array_reg, array_reg, get_slice_symbol_index);

		return array_reg;
	}
	else if (start_base == NULL && end_base != NULL)
	{
		// <object-expr>	
		int array_reg = left_base->TranslateExpr(this);
	
		// Push value_reg
		int expr_reg = GetInstructionBuffer()->NewRegister();
		GetInstructionBuffer()->EmitLoadLiteral(expr_reg, 0);
		GetInstructionBuffer()->EmitPush(CIntDataType::StaticInstance, expr_reg);
		GetInstructionBuffer()->FreeRegister(expr_reg);
			
		// <expr>
		expr_reg = end_base->TranslateExpr(this);

		// Push value_reg
		GetInstructionBuffer()->EmitPush(end_base->ExpressionResultType, expr_reg);
		GetInstructionBuffer()->FreeRegister(expr_reg);

		CDeclarationIdentifier s_getslice_ident("GetSlice");

		std::vector<CDataType*> argument_types;
		argument_types.push_back(CIntDataType::StaticInstance);
		argument_types.push_back(CIntDataType::StaticInstance);
		int get_slice_symbol_index = left_base->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_getslice_ident, argument_types, true)->TranslatedSymbol->Index;
		
		// InvokeMethod value_reg, array_reg, GetSlice
		GetInstructionBuffer()->EmitInvokeMethod(array_reg, array_reg, get_slice_symbol_index);

		return array_reg;
	}
	else if (start_base != NULL && end_base != NULL)
	{		
		// <object-expr>	
		int array_reg = left_base->TranslateExpr(this);
	
		// <expr>
		int expr_reg = start_base->TranslateExpr(this);

		// Push value_reg
		GetInstructionBuffer()->EmitPush(start_base->ExpressionResultType, expr_reg);
		GetInstructionBuffer()->FreeRegister(expr_reg);
			
		// <expr>
		expr_reg = end_base->TranslateExpr(this);

		// Push value_reg
		GetInstructionBuffer()->EmitPush(end_base->ExpressionResultType, expr_reg);
		GetInstructionBuffer()->FreeRegister(expr_reg);

		CDeclarationIdentifier s_getslice_ident("GetSlice");

		std::vector<CDataType*> argument_types;
		argument_types.push_back(CIntDataType::StaticInstance);
		argument_types.push_back(CIntDataType::StaticInstance);
		int get_slice_symbol_index = left_base->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_getslice_ident, argument_types, true)->TranslatedSymbol->Index;
		
		// InvokeMethod value_reg, array_reg, GetSlice
		GetInstructionBuffer()->EmitInvokeMethod(array_reg, array_reg, get_slice_symbol_index);

		return array_reg;
	}
	else
	{
		// <object-expr>	
		int array_reg = left_base->TranslateExpr(this);
	
		// <expr>
		int expr_reg = GetInstructionBuffer()->NewRegister();
		GetInstructionBuffer()->EmitLoadLiteral(expr_reg, 0);
		GetInstructionBuffer()->EmitPush(CIntDataType::StaticInstance, expr_reg);
		GetInstructionBuffer()->FreeRegister(expr_reg);

		CDeclarationIdentifier s_getslice_ident("GetSlice");

		std::vector<CDataType*> argument_types;
		argument_types.push_back(CIntDataType::StaticInstance);
		int get_slice_symbol_index = left_base->ExpressionResultType->GetClass(m_semanter)->FindClassMethod(m_semanter, s_getslice_ident, argument_types, true)->TranslatedSymbol->Index;
		
		// InvokeMethod value_reg, array_reg, GetSlice
		GetInstructionBuffer()->EmitInvokeMethod(array_reg, array_reg, get_slice_symbol_index);

		return array_reg;
	}
}

// =================================================================
//	Translates a typeof expression.
// =================================================================
int CVMBinaryTranslator::TranslateTypeOfExpression(CTypeOfExpressionASTNode* node)
{
	UpdateDebugTrace(node);
	
	int comp_reg = GetInstructionBuffer()->NewRegister();

	// oLoadTypeOf
	GetInstructionBuffer()->EmitLoadType(comp_reg, node->Type->GetClass(m_semanter)->TranslatedSymbol);

	return comp_reg;
}

// =================================================================
//	Translates a indexof expression.
// =================================================================
int CVMBinaryTranslator::TranslateIndexOfExpression(CIndexOfExpressionASTNode* node)
{
	UpdateDebugTrace(node);

	int comp_reg = GetInstructionBuffer()->NewRegister();

	// oLoadSymbolIndex
	GetInstructionBuffer()->EmitLoadLiteral(comp_reg, node->ClassMember->TranslatedSymbol->FieldOffset);

	return comp_reg;
}