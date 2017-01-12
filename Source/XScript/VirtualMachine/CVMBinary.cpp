/* *****************************************************************

		CInstructionBuffer.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CSymbol.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"
 
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassStateASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassReplicationASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassPropertyASTNode.h"
#include "XScript/Parser/Nodes/Statements/CVariableStatementASTNode.h"

#include "XScript/Parser/Nodes/Expressions/Values/CConstDictionaryASTNode.h"
#include "XScript/Parser/Nodes/Expressions/CExpressionBaseASTNode.h"
#include "XScript/Parser/Nodes/EvaluationResult.h"

#include "XScript/Compiler/CTranslationUnit.h"

#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Types/CVoidDataType.h"

#include "XScript/Semanter/CSemanter.h"

CVMBinary::CVMBinary()
	: m_resource_data(NULL)
{
}

CVMBinary::~CVMBinary()
{
	SAFE_DELETE(m_resource_data);
}

String CVMBinary::Get_Unique_Symbol_ID(CSymbol* symbol)
{
	CASTNode* parent = symbol->Node->Parent;
	CClassASTNode* parent_class_node = NULL;
	CClassMemberASTNode* parent_method_node = NULL;
	CClassStateASTNode* parent_state_node = NULL;
	while (parent != NULL)
	{
		parent_class_node = dynamic_cast<CClassASTNode*>(parent);
		parent_method_node = dynamic_cast<CClassMemberASTNode*>(parent);
		parent_state_node = dynamic_cast<CClassStateASTNode*>(parent);
		if (parent_class_node != NULL ||
			parent_method_node != NULL ||
			parent_state_node != NULL)
		{
			break;
		}
		parent = parent->Parent;
	}

	String parent_name = "root";
	if (parent_class_node != NULL)
	{
		parent_name = Get_Unique_Symbol_ID(parent_class_node->TranslatedSymbol).c_str();
	}
	if (parent_method_node != NULL)
	{
		parent_name = Get_Unique_Symbol_ID(parent_method_node->TranslatedSymbol).c_str();
	}
	if (parent_state_node != NULL)
	{
		parent_name = Get_Unique_Symbol_ID(parent_state_node->TranslatedSymbol).c_str();
	}

	String parameters = "";
	String type = "";
	CClassMemberASTNode* member = dynamic_cast<CClassMemberASTNode*>(symbol->Node);
	if (member != NULL)
	{
		parameters += "(";
		for (std::vector<CVariableStatementASTNode*>::iterator iter = member->Arguments.begin(); iter != member->Arguments.end(); iter++)
		{
			CVariableStatementASTNode* var = *iter;
			parameters += var->Type->ToString();
			if (iter != member->Arguments.end() - 1)
			{
				parameters += ",";
			}
		}
		parameters += ")";
		type += "[";
		type += member->ReturnType->ToString();
		type += "]";
	}

	String native_ident = "";
	CDeclarationASTNode* decl = dynamic_cast<CDeclarationASTNode*>(symbol->Node);
	if (decl != NULL)
	{
		native_ident = decl->MangledIdentifier.c_str();
	}

	String value = StringHelper::Format
	(
		"%s.%s[%s]%s%s",
		parent_name.c_str(),
		symbol->Identifier.c_str(),
		native_ident.c_str(),
		parameters.c_str(),
		type.c_str()
	);

	return value;
}

void CVMBinary::Setup(CSemanter* semanter, std::vector<String> string_table, std::vector<CSymbol*> symbol_table)
{
	m_string_table = string_table;
	m_symbol_table = symbol_table;
	m_semanter = semanter;

	/*
	// Bake up some nice symbol table entries that are independent of CSymbol instances
	for (std::vector<CSymbol*>::iterator iter = symbol_table.begin(); iter != symbol_table.end(); iter++)
	{
		CSymbol* symbol = *iter;

		CVMBinarySymbolEntry entry;
		entry.unique_id = StringHelper::Hash(Get_Unique_Symbol_ID(symbol).c_str());
		entry.type = symbol->Type;
		entry.native_name = "";

		if (symbol->Type == SymbolType::Class)
		{
			CClassASTNode* class_sym = static_cast<CClassASTNode*>(symbol->Node);
			entry.native_name = class_sym->MangledIdentifier;
		}
		else if (symbol->Type == SymbolType::ClassMethod)
		{
			CClassMemberASTNode* member_sym = static_cast<CClassMemberASTNode*>(symbol->Node);
			entry.native_name = member_sym->MangledIdentifier;
		}
		else if (symbol->Type == SymbolType::ClassState)
		{
			CClassStateASTNode* member_sym = static_cast<CClassStateASTNode*>(symbol->Node);
			entry.native_name = member_sym->MangledIdentifier;
		}

		CClassASTNode* class_scope = symbol->Node->FindClassScope(m_semanter);
		if (class_scope != NULL)
		{
			entry.class_scope_index = class_scope->TranslatedSymbol->Index;
		}
		else
		{
			entry.class_scope_index = 0;
		}

		for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
		{
			CSymbol* child = *iter;
			entry.child_indexes.push_back(child->Index);
		}
		
		for (std::vector<CASTNodeMetaData>::iterator iter = symbol->Node->MetaData.begin(); iter != symbol->Node->MetaData.end(); iter++)
		{
			CASTNodeMetaData& child = *iter;
			entry.meta_data.push_back(child);
		}

		m_symbol_table.push_back(entry);
	}
	*/
}

void CVMBinary::Serialize(PatchedBinaryStream* stream)
{
#define STRING_PTR(val) string_ptrs.push_back(stream->Create_Pointer()); string_values.push_back(val); string_patched.push_back(false);
	std::vector<int> string_ptrs;
	std::vector<int> string_patched;
	std::vector<String> string_values;

//	Stream* tmp_stream = StreamFactory::Open("D:\\Perforce\\TLeonard_Main\\TLeonard_Main\\ZombieGrinder\\C++\\Data\\Base\\script.manifest", StreamMode::Write);

	// CVMBinaryData
	STRING_PTR(m_semanter->GetContext()->GetFilePath().c_str());
	stream->Write<int>(m_string_table.size());
	stream->Write<int>(m_symbol_table.size());
	int string_table_ptr = stream->Create_Pointer();
	int symbol_table_ptr = stream->Create_Pointer();
	stream->Align_To_Pointer();

	// String table.
	stream->Patch_Pointer(string_table_ptr);
	for (unsigned int i = 0; i < m_string_table.size(); i++)
	{
		STRING_PTR(m_string_table.at(i).c_str());
	}

	// Symbol table.
	stream->Patch_Pointer(symbol_table_ptr);
	std::vector<int> symbol_table_symbol_ptr;
	std::vector<int> symbol_table_children_ptr;
	std::vector<int> symbol_table_meta_data_ptr;
	std::vector<int> symbol_table_type_specific_data_ptr;
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		symbol_table_symbol_ptr.push_back(stream->Create_Pointer());
	}
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		int ptr = symbol_table_symbol_ptr.at(i);
		stream->Patch_Pointer(ptr);

		CSymbol* symbol = m_symbol_table.at(i);
		
		// CVMBinarySymbol		
		stream->Write<int>(StringHelper::Hash(Get_Unique_Symbol_ID(symbol).c_str()));
		stream->Write<SymbolType::Type>(symbol->Type);
		STRING_PTR(symbol->Identifier.c_str());

		String native_name = "";

		if (symbol->Type == SymbolType::Class)
		{
			CClassASTNode* class_sym = static_cast<CClassASTNode*>(symbol->Node);
			native_name = class_sym->MangledIdentifier;
		}
		else if (symbol->Type == SymbolType::ClassMethod)
		{
			CClassMemberASTNode* member_sym = static_cast<CClassMemberASTNode*>(symbol->Node);
			native_name = member_sym->MangledIdentifier;
		}
		else if (symbol->Type == SymbolType::ClassState)
		{
			CClassStateASTNode* member_sym = static_cast<CClassStateASTNode*>(symbol->Node);
			native_name = member_sym->MangledIdentifier;
		}

		STRING_PTR(native_name.c_str());

		CClassASTNode* class_scope = symbol->Node->FindClassScope(m_semanter);
		if (class_scope != NULL)
		{
			stream->Write<int>(class_scope->TranslatedSymbol->Index);
		}
		else
		{
			stream->Write<int>(0);
		}

		// CSymbolDebugInfo
		stream->Write<int>(symbol->Node->TranslatedDebugInfo.Row);
		stream->Write<int>(symbol->Node->TranslatedDebugInfo.Column);
		stream->Write<int>(symbol->Node->TranslatedDebugInfo.FileNameIndex);
		stream->Align_To_Pointer();

		stream->Write<int>(symbol->Children.size());
		symbol_table_children_ptr.push_back(stream->Create_Pointer());

		// Meta data.
		stream->Write<int>(symbol->Node->MetaData.size());
		symbol_table_meta_data_ptr.push_back(stream->Create_Pointer());

		// Type-Specific Data
		symbol_table_type_specific_data_ptr.push_back(stream->Create_Pointer());

		stream->Align_To_Pointer();
	}

	// Symbol children.
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		CSymbol* symbol = m_symbol_table.at(i);
		int ptr = symbol_table_children_ptr.at(i);

		stream->Patch_Pointer(ptr);
		for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
		{
			CSymbol* child = *iter;
			stream->Write<int>(child->Index);
		}
	}

	// Symbol metadata.
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		CSymbol* symbol = m_symbol_table.at(i);
		int ptr = symbol_table_meta_data_ptr.at(i);

		stream->Patch_Pointer(ptr);
		for (std::vector<CASTNodeMetaData>::iterator iter = symbol->Node->MetaData.begin(); iter != symbol->Node->MetaData.end(); iter++)
		{
			CASTNodeMetaData& child = *iter;

//			unsigned int start = stream->Position();
			stream->Write<CVMMetaDataType::Type>((CVMMetaDataType::Type)child.type);
			STRING_PTR(child.name);
			stream->Write<float>(child.float_value);
			stream->Write<int>(child.int_value);
			STRING_PTR(child.string_value);
			stream->Align_To_Pointer();

//			unsigned int diff = stream->Position() - start;
//			DBG_ASSERT(diff == sizeof(CVMMetaData));
		}
	}

	int total_bytecode = 0;
	int total_encoded_bytecode = 0;
	/*
	DBG_LOG("============================ SYMBOL TABLE ============================");
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		CSymbol* sym = m_symbol_table[i];
		DBG_LOG("[%i] %s", i, sym->Identifier.c_str());
	}

	DBG_LOG("============================ REPLICATED SYMBOLS ============================");
	*/
	int check_index = -1;

	// Class specific-data.
	std::vector<int> symbol_table_datatype_ptr;
	std::vector<CDataType*> symbol_table_datatype;
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		CSymbol* symbol = m_symbol_table.at(i);
		switch (symbol->Type)
		{
		case SymbolType::ClassState:
			{
				CClassStateASTNode* state_sym = static_cast<CClassStateASTNode*>(symbol->Node);
				
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));

				// CVMStateData
				stream->Write<int>(state_sym->IsDefault);
				stream->Write<int>(state_sym->Events.size());
				int event_ptr = stream->Create_Pointer();
				stream->Align_To_Pointer();

				// Events.
				stream->Patch_Pointer(event_ptr);
				for (std::vector<CClassMemberASTNode*>::iterator iter = state_sym->Events.begin(); iter != state_sym->Events.end(); iter++)
				{
					CClassMemberASTNode* member = *iter;
					stream->Write<int>(member->TranslatedSymbol->Index);
				}

				break;
			}
		case SymbolType::ClassProperty:
			{
				CClassPropertyASTNode* prop_sym = static_cast<CClassPropertyASTNode*>(symbol->Node);
				
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));

				// CVMPropertyData
				stream->Write<int>(prop_sym->IsSerialized);
				stream->Write<int>(prop_sym->SerializeVersion);
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(prop_sym->ReturnType);
				stream->Write<int>(prop_sym->Get_Method == NULL ? 0 : prop_sym->Get_Method->TranslatedSymbol->Index);
				stream->Write<int>(prop_sym->Set_Method == NULL ? 0 : prop_sym->Set_Method->TranslatedSymbol->Index);
				stream->Align_To_Pointer();

				break;
			}
		case SymbolType::Class:
			{
				CClassASTNode* class_sym = static_cast<CClassASTNode*>(symbol->Node);
				CClassMemberASTNode* default_constructor = class_sym->FindClassMethod(m_semanter, class_sym->Identifier, std::vector<CDataType*>(), true, NULL, NULL);

				// CVMClassData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));

				stream->Write<int>(symbol->Interfaces.size());
				int interfaces_ptr = stream->Create_Pointer();

				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(class_sym->ObjectDataType);

				stream->Write<int>(symbol->StaticCount);

				if (class_sym->SuperClass != NULL)
					stream->Write<int>(class_sym->SuperClass->TranslatedSymbol->Index);
				else
					stream->Write<int>(0);

				if (default_constructor != NULL)
					stream->Write<int>(default_constructor->TranslatedSymbol->Index);
				else
					stream->Write<int>(0);

				// state data
				stream->Write<int>(class_sym->DefaultState == NULL ? -1 : class_sym->DefaultState->TranslatedSymbol->Index);
				stream->Write<int>(class_sym->TranslatedSymbol->StateCount);
				int state_indexes_ptr = stream->Create_Pointer();

				// replication data.
				stream->Write<int>(class_sym->IsReplicated);
				int replication_info_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->FieldCount);
				int field_indexes_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->MethodCount);
				int method_indexes_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->VirtualMethodCount);
				int vftable_ptr = stream->Create_Pointer();
				
				stream->Write<int>(class_sym->IsInterface);
				stream->Write<int>(class_sym->IsNative);

				stream->Align_To_Pointer();
				
				// Replication information.
				if (class_sym->IsReplicated == true)
				{
					//DBG_LOG("[Class=%s]", symbol->Identifier.c_str());

					bool has_client_owned_vars = false;
					for (std::vector<CClassReplicationVariable>::iterator iter = class_sym->ReplicationNode->Variables.begin(); iter != class_sym->ReplicationNode->Variables.end(); iter++)
					{
						CClassReplicationVariable& var = *iter;
						int owner = var.OwnerExpression->Evaluate(m_semanter->GetContext()).GetInt();
						if (owner != 0)
						{
							has_client_owned_vars = true;
						}
					}

					// CVMReplicationInfo
					stream->Patch_Pointer(replication_info_ptr);
					stream->Write<int>(class_sym->ReplicationNode->PriorityExpression->Evaluate(m_semanter->GetContext()).GetInt());
					stream->Write<int>(has_client_owned_vars ? 1 : 0);
					stream->Write<int>(class_sym->ReplicationNode->Variables.size());
					int variable_ptr = stream->Create_Pointer();
					stream->Align_To_Pointer();

					stream->Patch_Pointer(variable_ptr);
					for (std::vector<CClassReplicationVariable>::iterator iter = class_sym->ReplicationNode->Variables.begin(); iter != class_sym->ReplicationNode->Variables.end(); iter++)
					{
						CClassReplicationVariable& var = *iter;
						stream->Write<int>(var.Resolved == NULL ? -1 : var.Resolved->TranslatedSymbol->Index);
						stream->Write<int>(var.ResolvedProperty == NULL ? -1 : var.ResolvedProperty->TranslatedSymbol->Index);
						stream->Write<int>(var.OwnerExpression->Evaluate(m_semanter->GetContext()).GetInt());
						stream->Write<int>(var.ModeExpression->Evaluate(m_semanter->GetContext()).GetInt());

						//DBG_LOG("Symbol[%i]=%s", var.Resolved == NULL ? var.ResolvedProperty->TranslatedSymbol->Index : var.Resolved->TranslatedSymbol->Index, var.Resolved == NULL ? var.ResolvedProperty->TranslatedSymbol->Identifier.c_str() : var.Resolved->TranslatedSymbol->Identifier.c_str());

						stream->Align_To_Pointer();
					}
				}

				// CVMInterface's
				if (symbol->Interfaces.size() > 0)
				{
					stream->Patch_Pointer(interfaces_ptr);
					for (std::vector<CInterfaceSymbol>::iterator iter = symbol->Interfaces.begin(); iter != symbol->Interfaces.end(); iter++)
					{
						CInterfaceSymbol& inter = *iter;
						stream->Write<int>(inter.InterfaceClass->Index);
						stream->Write<int>(inter.VFTableOffset);
						stream->Align_To_Pointer();
					}
				}
				
				// State indexes.
				if (symbol->StateCount > 0)
				{
					stream->Patch_Pointer(state_indexes_ptr);
					int counter = 0;
					for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
					{
						CSymbol* node = *iter;
						if (node->Type == SymbolType::ClassState)
						{
							stream->Write<int>(node->Index);
							counter++;
						}
					}
					//DBG_ASSERT(counter == symbol->StateCount);
				}

				// Field indexes.
				if (symbol->FieldCount > 0)
				{
					stream->Patch_Pointer(field_indexes_ptr);
					int counter = 0;
					for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
					{
						CSymbol* node = *iter;
						if (node->Type == SymbolType::ClassField)
						{
							stream->Write<int>(node->Index);
							counter++;
						}
					}
					DBG_ASSERT(counter == symbol->FieldCount);
				}

				// Method indexes.
				if (symbol->MethodCount > 0)
				{
					stream->Patch_Pointer(method_indexes_ptr);
					int counter = 0;
					for (std::vector<CSymbol*>::iterator iter = symbol->Children.begin(); iter != symbol->Children.end(); iter++)
					{
						CSymbol* node = *iter;
						if (node->Type == SymbolType::ClassMethod)
						{
							stream->Write<int>(node->Index);
							counter++;
						}
					}
					DBG_ASSERT(counter == symbol->MethodCount);
				}

				// VFTable indexes
				if (symbol->VirtualMethodCount > 0)
				{
					stream->Patch_Pointer(vftable_ptr);
					for (int i = 0; i < symbol->VirtualMethodCount; i++)
					{
						stream->Write<int>(symbol->VFTable.at(i)->Index);
					}
				}

				break;
			}

		case SymbolType::ClassField:
			{
				CClassMemberASTNode* method = static_cast<CClassMemberASTNode*>(symbol->Node);
				
				// CVMFieldData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->FieldDataType);

				stream->Write<int>(symbol->FieldOffset);
				stream->Write<int>(symbol->IsSerialized);
				stream->Write<int>(symbol->SerializeVersion);
				stream->Write<int>(method->IsStatic);
				stream->Align_To_Pointer();

				break;
			}

		case SymbolType::ClassMethod:
			{
				CClassMemberASTNode* method = static_cast<CClassMemberASTNode*>(symbol->Node);

				// Calculate byte code size.
				// + Precalculate bytecode offsets to speed up jump target resolving.
				int bytecode_size = 0;
				for (int j = 0; j < symbol->Instructions.Get_Instruction_Count(); j++)
				{
					CInstruction& instr = symbol->Instructions.Get_Instruction(j);
					instr.ByteOffset = bytecode_size;
					bytecode_size += instr.Encoded_Size();
				}

				// CVMMethodData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));
				CClassASTNode* return_class = method->ReturnType->GetClass(m_semanter);
				stream->Write<int>(return_class == NULL ? -1 : return_class->TranslatedSymbol->Index);
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->ReturnDataType);
				stream->Write<int>(method->Arguments.size());
				int argument_indexes_ptr = stream->Create_Pointer();
				stream->Write<int>(method->IsVirtual);
				stream->Write<int>(method->IsNative);
				stream->Write<int>(method->IsStatic);
				stream->Write<int>(method->IsAbstract);
				stream->Write<int>(method->IsConstructor);
				stream->Write<int>(method->IsRPC);
				stream->Write<int>(symbol->VFTableOffset);
				stream->Write<int>(symbol->Instructions.GetMaxUsedRegisterCount());
				stream->Write<int>(symbol->LocalCount);
				int locals_indexes_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->Instructions.Get_Debug_Info_Count());
				int debug_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->Instructions.Get_Instruction_Count());
				stream->Write<int>(bytecode_size);
				int bytecode_ptr = stream->Create_Pointer();
			//	stream->Write<int>(symbol->Instructions.Get_Instruction_Count());
			//	int instructions_ptr = stream->Create_Pointer();

				stream->Align_To_Pointer();

				// Parameter indexes.
				stream->Patch_Pointer(argument_indexes_ptr);
				for (std::vector<CVariableStatementASTNode*>::iterator iter = method->Arguments.begin(); iter != method->Arguments.end(); iter++)
				{
					CVariableStatementASTNode* node = *iter;
					stream->Write<int>(node->TranslatedSymbol->Index);
				}

				// Local indexes.
				stream->Patch_Pointer(locals_indexes_ptr);
				for (std::vector<CSymbol*>::iterator iter = method->TranslatedSymbol->Children.begin(); iter != method->TranslatedSymbol->Children.end(); iter++)
				{
					CSymbol* node = *iter;
					if (node->Type == SymbolType::LocalVariable)
					{
						stream->Write<int>(node->Index);
					}
				}

				// Debug info
				stream->Patch_Pointer(debug_ptr);
				for (int j = 0; j < symbol->Instructions.Get_Debug_Info_Count(); j++)
				{
					CDebugInfo& instr = symbol->Instructions.Get_Debug_Info(j);

					// CVMDebugInfo
					stream->Write<int>(symbol->Instructions.Get_Instruction(instr.Instruction_Offset).ByteOffset);
					//stream->Write<int>(instr.Instruction_Offset);
					stream->Write<int>(instr.File_Index);
					stream->Write<int>(instr.Row);
					stream->Write<int>(instr.Column);

					// Force pack.
					stream->Align_To_Pointer();
				}

				// Write out encoded data.
				stream->Patch_Pointer(bytecode_ptr);
				char* data = new char[bytecode_size];
				char* encode_ptr = data;

				// Resolve jump targets to bytecode offsets.
			//	DBG_LOG("[%s]", method->TranslatedSymbol->Identifier.c_str());
				for (int j = 0; j < symbol->Instructions.Get_Instruction_Count(); j++)
				{
					CInstruction& instr = symbol->Instructions.Get_Instruction(j);
					CInstructionMetaInfo info = InstructionMetaInfo[(int)instr.OpCode];

			//		DBG_LOG("%i: %s", j, info.Name);
					for (int k = 0; k < info.Operand_Count; k++)
					{
						if (info.Operand_Types[k] == OperandType::JumpTarget)
						{
							int jmp_target = symbol->Instructions.Get_JmpTarget(instr.Operands[k].int_value);
							CInstruction& target_instr = symbol->Instructions.Get_Instruction(jmp_target);
			//				DBG_LOG("Patched %i to %i", jmp_target, target_instr.ByteOffset);

							instr.Operands[k].int_value = target_instr.ByteOffset;
						}
					}
				}

				//bool should_diss = (symbol->Identifier == "On_Tick");

				//if (should_diss)
				//	tmp_stream->WriteLine(StringHelper::Format("[ %s : %i instructions ]", symbol->Identifier.c_str(), symbol->Instructions.Get_Instruction_Count()).c_str());
	
				//DBG_LOG("Symbol: %s", symbol->Identifier.c_str());
				for (int j = 0; j < symbol->Instructions.Get_Instruction_Count(); j++)
				{
					CInstruction& instr = symbol->Instructions.Get_Instruction(j);
					//String ret = instr.Disassemble(m_symbol_table);
					//DBG_LOG("%i: %s", j, ret.c_str());

					instr.Encode(encode_ptr);

				//	if (should_diss)
				//	{
				//		String ret = instr.Disassemble(m_symbol_table);
				//		tmp_stream->WriteLine(ret.C_Str());
				//	}
				}

				stream->WriteBuffer(data, 0, bytecode_size);

				SAFE_DELETE_ARRAY(data);

				/*
				std::vector<int> operand_ptrs;

				int start_offset = stream->Position();

				// Instructions
				stream->Patch_Pointer(instructions_ptr);
				for (int j = 0; j < symbol->Instructions.Get_Instruction_Count(); j++)
				{
					CInstruction& instr = symbol->Instructions.Get_Instruction(j);
					CInstructionMetaInfo& info = InstructionMetaInfo[instr.OpCode];

					// CVMInstruction
					stream->Write<InstructionOpCode::Type>(instr.OpCode);
					operand_ptrs.push_back(stream->Create_Pointer());
					
					// Force pack.
					stream->Align_To_Pointer();
				}

				// Operands.				
				for (int j = 0; j < symbol->Instructions.Get_Instruction_Count(); j++)
				{
					CInstruction& instr = symbol->Instructions.Get_Instruction(j);
					CInstructionMetaInfo& info = InstructionMetaInfo[instr.OpCode];

					stream->Patch_Pointer(operand_ptrs.at(j));
					
					for (int k = 0; k < info.Operand_Count; k++)
					{
						// CVMOperand
						if (info.Operand_Types[k] == OperandType::JumpTarget)
						{
							stream->Write<int>(symbol->Instructions.Get_JmpTarget(instr.Operands[k].int_value));
						}
						else
						{
							stream->Write<int>(instr.Operands[k].int_value);
						}

						// Force pack.
						stream->Align_To_Pointer();
					}
				}

				int real_size = stream->Position() - start_offset;
				int encoded_size = 0;

				for (int j = 0; j < symbol->Instructions.Get_Instruction_Count(); j++)
				{
					CInstruction& instr = symbol->Instructions.Get_Instruction(j);
					encoded_size += instr.Encoded_Size();
				}

				DBG_LOG("[%s : %i Instructions] Unencoded=%i Encoded=%i", 
					symbol->Identifier.c_str(), 
					symbol->Instructions.Get_Instruction_Count(), 
					real_size, 
					encoded_size
				);

				total_bytecode += real_size;
				total_encoded_bytecode += encoded_size;
				*/

				break;
			}

		case SymbolType::LocalVariable:
			{
				CVariableStatementASTNode* variable = static_cast<CVariableStatementASTNode*>(symbol->Node);
				
				// CVMVariableData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->LocalDataType);
				stream->Write<int>(variable->IsParameter);

				stream->Align_To_Pointer();
				break;
			}
		}
	}

//	DBG_LOG("TOTAL: %i", total_bytecode);
//	DBG_LOG("TOTAL ENCODED: %i", total_encoded_bytecode);
	
	// Write data types.
	unsigned int start_count = symbol_table_datatype.size();
	for (unsigned int i = 0; i < symbol_table_datatype.size(); i++)
	{
		CDataType* type = symbol_table_datatype.at(i);
		int ptr = symbol_table_datatype_ptr.at(i);

		//if (ptr == check_index)
		//{
		//	DBG_LOG("PATCHING DAT ASS!");
		//}

		stream->Patch_Pointer(ptr);
		CVMBaseDataType::Type base = DataTypeToVMDataType(type);

		stream->Write<CVMBaseDataType::Type>(base);
		int array_ptr = stream->Create_Pointer();
		STRING_PTR(type->ToString().c_str());
		CClassASTNode* class_node = type->GetClass(m_semanter);
		stream->Write<int>(class_node != NULL ? class_node->TranslatedSymbol->Index : -1);
		stream->Write<int>(0);
		stream->Align_To_Pointer();

		String ident = type->ToString();


		CDataType* element_type = NULL;
		if (dynamic_cast<CArrayDataType*>(type) != NULL)
		{
			element_type = dynamic_cast<CArrayDataType*>(type)->ElementType;
		}
		if (dynamic_cast<CObjectDataType*>(type) != NULL)
		{
			CClassASTNode* sym = dynamic_cast<CObjectDataType*>(type)->GetClass(m_semanter);
			if (sym && sym->IsGeneric == true && std::string(sym->GenericInstanceOf->Identifier.c_str()) == "array")
			{
				element_type = sym->GenericInstanceTypes[0];
				CClassASTNode* element_node = element_type->GetClass(m_semanter);
				//DBG_LOG("fuck: %s", element_node->Identifier.c_str());
			}
		}

		// If we are an array we need to assign element type.
		if (element_type != NULL)
		{
			bool found = false;

			// Look for type in previously written data types.
			for (unsigned int j = 0; j < i; j++)
			{
				CDataType* other_type = symbol_table_datatype.at(j);
				int other_ptr = symbol_table_datatype_ptr.at(j);
				
				if (element_type->IsEqualTo(m_semanter, other_type))
				{
					// We don't want to merge derp[] and array<derp>
					if ((dynamic_cast<CArrayDataType*>(element_type) != NULL && dynamic_cast<CObjectDataType*>(other_type) != NULL) ||
						(dynamic_cast<CObjectDataType*>(element_type) != NULL && dynamic_cast<CArrayDataType*>(other_type) != NULL))
					{
						continue;
					}

					//if (array_ptr == check_index)
					//{
					//	DBG_LOG("PATCHING DAT ASS!");
					//}

					stream->Patch_Pointer(array_ptr, other_ptr, true);
					found = true;
					break;
				}
			}

			// Add data type to list.
			if (found == false)
			{
				symbol_table_datatype_ptr.push_back(array_ptr);
				symbol_table_datatype.push_back(element_type);
			}
		}

		// If any other similar data types exist, point them to this one.	
		for (unsigned int j = i + 1; j < symbol_table_datatype.size(); j++)
		{
			CDataType* other_type = symbol_table_datatype.at(j);
			int other_ptr = symbol_table_datatype_ptr.at(j);

			if (type->IsEqualTo(m_semanter, other_type))
			{
				// We don't want to merge derp[] and array<derp>	
				if ((dynamic_cast<CArrayDataType*>(type) != NULL && dynamic_cast<CObjectDataType*>(other_type) != NULL) ||
					(dynamic_cast<CObjectDataType*>(type) != NULL && dynamic_cast<CArrayDataType*>(other_type) != NULL))
				{
					continue;
				}

				stream->Patch_Pointer(other_ptr, ptr, true);
				symbol_table_datatype.erase(symbol_table_datatype.begin() + j);
				symbol_table_datatype_ptr.erase(symbol_table_datatype_ptr.begin() + j);
				j--;
				continue;
			}
		}
	}

	// Dump string data.
	std::vector<int> string_hashes;
	string_hashes.reserve(string_ptrs.size());

	for (unsigned int i = 0; i < string_ptrs.size(); i++)
	{
		String& val = string_values.at(i);
		string_hashes.push_back(StringHelper::Hash(val.c_str()));
	}

	for (unsigned int i = 0; i < string_ptrs.size(); i++)
	{
		int ptr = string_ptrs.at(i);
		String& val = string_values.at(i);
		int& patched = string_patched.at(i);
		int& hash = string_hashes.at(i);

		if (patched == 1)
		{
			continue;
		}

		stream->Patch_Pointer(ptr);
		stream->WriteNullTerminatedString(val.c_str());

		for (unsigned int j = i + 1; j < string_ptrs.size(); j++)
		{
			int other_ptr = string_ptrs.at(j);
			int& other_patched = string_patched.at(j);
			int& other_hash = string_hashes.at(j);

			if (other_patched == 0 && hash == other_hash)
			{
				stream->Patch_Pointer(other_ptr, ptr, true);
				other_patched = 1;
				continue;
			}
		}

		patched = 1;
	}

	//SAFE_DELETE(tmp_stream);

	//DBG_LOG("VMBinary Size: %i", stream->Length() / 1024);

#undef STRING_PTR
}

// ------------------------------------------------------------------------------
// TODO:
// This is so much dickfuck. We need to de-duplicate this, there is no need for
// this and serialize :S.
// ---------------------------------------------------------------------------------
void CVMBinary::Serialize_Merged(PatchedBinaryStream* stream)
{
#define STRING_PTR(val) string_ptrs.push_back(stream->Create_Pointer()); string_values.push_back(val); string_patched.push_back(false);
	std::vector<int> string_ptrs;
	std::vector<int> string_patched;
	std::vector<String> string_values;

	// CVMBinaryData
	STRING_PTR("Merged");
	stream->Write<int>(m_merge_string_table.size());
	stream->Write<int>(m_merge_symbol_table.size());
	int string_table_ptr = stream->Create_Pointer();
	int symbol_table_ptr = stream->Create_Pointer();
	stream->Align_To_Pointer();

	// String table.
	stream->Patch_Pointer(string_table_ptr);
	for (unsigned int i = 0; i < m_merge_string_table.size(); i++)
	{
		STRING_PTR(m_merge_string_table.at(i));
	}

	// Symbol table.
	stream->Patch_Pointer(symbol_table_ptr);
	std::vector<int> symbol_table_symbol_ptr;
	std::vector<int> symbol_table_children_ptr;
	std::vector<int> symbol_table_meta_data_ptr;
	std::vector<int> symbol_table_type_specific_data_ptr;
	for (unsigned int i = 0; i < m_merge_symbol_table.size(); i++)
	{
		symbol_table_symbol_ptr.push_back(stream->Create_Pointer());
	}
	for (unsigned int i = 0; i < m_merge_symbol_table.size(); i++)
	{
		int ptr = symbol_table_symbol_ptr.at(i);
		stream->Patch_Pointer(ptr);

		MergeBinarySymbol& symbol = m_merge_symbol_table.at(i);
		
		// CVMBinarySymbol		
		stream->Write<int>(symbol.symbol->unique_id);
		stream->Write<SymbolType::Type>(symbol.symbol->type);
		STRING_PTR(symbol.symbol->name);
		STRING_PTR(symbol.symbol->native_name);

		stream->Write<int>(symbol.symbol->class_scope_index);

		// CSymbolDebugInfo
		stream->Write<int>(symbol.symbol->debug.Row);
		stream->Write<int>(symbol.symbol->debug.Column);
		stream->Write<int>(symbol.symbol->debug.FileNameIndex);
		stream->Align_To_Pointer();

		stream->Write<int>(symbol.symbol->children_count);
		symbol_table_children_ptr.push_back(stream->Create_Pointer());

		// Meta data.
		stream->Write<int>(symbol.symbol->metadata_count);
		symbol_table_meta_data_ptr.push_back(stream->Create_Pointer());

		// Type-Specific Data
		symbol_table_type_specific_data_ptr.push_back(stream->Create_Pointer());

		stream->Align_To_Pointer();
	}
	
	// Symbol children.
	for (unsigned int i = 0; i < m_merge_symbol_table.size(); i++)
	{
		MergeBinarySymbol& symbol = m_merge_symbol_table.at(i);
		int ptr = symbol_table_children_ptr.at(i);

		stream->Patch_Pointer(ptr);
		for (int j = 0; j < symbol.symbol->children_count; j++)
		{
			stream->Write<int>(symbol.symbol->children_indexes[j]);
		}
	}
	
	// Symbol metadata.
	for (unsigned int i = 0; i < m_merge_symbol_table.size(); i++)
	{
		MergeBinarySymbol& symbol = m_merge_symbol_table.at(i);
		int ptr = symbol_table_meta_data_ptr.at(i);

		stream->Patch_Pointer(ptr);
		for (int j = 0; j < symbol.symbol->metadata_count; j++)
		{
			CVMMetaData& child = symbol.symbol->metadata[j];

			stream->Write<CVMMetaDataType::Type>(child.type);
			STRING_PTR(child.name);
			stream->Write<float>(child.float_value);
			stream->Write<int>(child.int_value);
			STRING_PTR(child.string_value);
			stream->Align_To_Pointer();
		}
	}

	// Class specific-data.
	std::vector<int> symbol_table_datatype_ptr;
	std::vector<CVMDataType*> symbol_table_datatype;
	for (unsigned int i = 0; i < m_merge_symbol_table.size(); i++)
	{
		CVMBinarySymbol* symbol = m_merge_symbol_table.at(i).symbol;
		switch (symbol->type)
		{
		case SymbolType::ClassState:
			{
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));

				// CVMStateData
				stream->Write<int>(symbol->state_data->is_default);
				stream->Write<int>(symbol->state_data->event_count);
				int event_ptr = stream->Create_Pointer();
				stream->Align_To_Pointer();

				//DBG_LOG("State %s", symbol->name);

				// Events.
				stream->Patch_Pointer(event_ptr);
				for (int j = 0; j < symbol->state_data->event_count; j++)
				{
				//	DBG_LOG("Event %i", symbol->state_data->event_indexes[j]);
					stream->Write<int>(symbol->state_data->event_indexes[j]);
				}

				break;
			}
		case SymbolType::ClassProperty:
			{
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));

				// CVMPropertyData
				stream->Write<int>(symbol->property_data->is_serialized);
				stream->Write<int>(symbol->property_data->serialized_version);
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->property_data->data_type);
				stream->Write<int>(symbol->property_data->get_property_symbol);
				stream->Write<int>(symbol->property_data->set_property_symbol);
				stream->Align_To_Pointer();

				break;
			}
		case SymbolType::Class:
			{
				// CVMClassData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));

				stream->Write<int>(symbol->class_data->interface_count);
				int interfaces_ptr = stream->Create_Pointer();

				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->class_data->data_type);

				stream->Write<int>(symbol->class_data->static_table_size);
				stream->Write<int>(symbol->class_data->super_class_index);
				stream->Write<int>(symbol->class_data->default_constructor_index);

				// state data
				stream->Write<int>(symbol->class_data->default_state_index);
				stream->Write<int>(symbol->class_data->state_count);
				int state_indexes_ptr = stream->Create_Pointer();

				// replication data.
				stream->Write<int>(symbol->class_data->is_replicated);
				int replication_info_ptr = stream->Create_Pointer();
				
				stream->Write<int>(symbol->class_data->field_count);
				int field_indexes_ptr = stream->Create_Pointer();
				
				stream->Write<int>(symbol->class_data->method_count);
				int method_indexes_ptr = stream->Create_Pointer();
				
				stream->Write<int>(symbol->class_data->virtual_table_size);
				int vftable_ptr = stream->Create_Pointer();
				
				stream->Write<int>(symbol->class_data->is_interface);
				stream->Write<int>(symbol->class_data->is_native);

				stream->Align_To_Pointer();
				
				// Replication information.
				if (symbol->class_data->is_replicated)
				{
					// CVMReplicationInfo
					stream->Patch_Pointer(replication_info_ptr);
					stream->Write<int>(symbol->class_data->replication_info->priority);
					stream->Write<int>(symbol->class_data->replication_info->has_client_controlled_vars);
					stream->Write<int>(symbol->class_data->replication_info->variable_count);
					int variable_ptr = stream->Create_Pointer();
					stream->Align_To_Pointer();

					stream->Patch_Pointer(variable_ptr);
					for (int j = 0; j < symbol->class_data->replication_info->variable_count; j++)
					{
						stream->Write<int>(symbol->class_data->replication_info->variables[j].var_symbol_index);
						stream->Write<int>(symbol->class_data->replication_info->variables[j].prop_symbol_index);
						stream->Write<int>(symbol->class_data->replication_info->variables[j].owner);
						stream->Write<int>(symbol->class_data->replication_info->variables[j].mode);
						stream->Align_To_Pointer();
					}
				}

				// CVMInterface's
				if (symbol->class_data->interface_count > 0)
				{
					stream->Patch_Pointer(interfaces_ptr);
					for (int j = 0; j < symbol->class_data->interface_count; j++)
					{
						stream->Write<int>(symbol->class_data->interfaces[j].interface_symbol_index);
						stream->Write<int>(symbol->class_data->interfaces[j].vf_table_offset);
						stream->Align_To_Pointer();
					}
				}
				
				// State indexes.
				if (symbol->class_data->state_count > 0)
				{
					stream->Patch_Pointer(state_indexes_ptr);
					for (int j = 0; j < symbol->class_data->state_count; j++)
					{
						stream->Write<int>(symbol->class_data->state_indexes[j]);
					}
				}

				// Field indexes.
				if (symbol->class_data->field_count > 0)
				{
					stream->Patch_Pointer(field_indexes_ptr);
					for (int j = 0; j < symbol->class_data->field_count; j++)
					{
						stream->Write<int>(symbol->class_data->fields_indexes[j]);
					}
				}

				// Method indexes.
				if (symbol->class_data->method_count > 0)
				{
					stream->Patch_Pointer(method_indexes_ptr);
					for (int j = 0; j < symbol->class_data->method_count; j++)
					{
						stream->Write<int>(symbol->class_data->method_indexes[j]);
					}
				}

				// VFTable indexes
				if (symbol->class_data->virtual_table_size > 0)
				{
					stream->Patch_Pointer(vftable_ptr);
					for (int j = 0; j < symbol->class_data->virtual_table_size; j++)
					{
						stream->Write<int>(symbol->class_data->virtual_table_indexes[j]);
					}
				}

				break;
			}

		case SymbolType::ClassField:
			{
				// CVMFieldData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->field_data->data_type);
				stream->Write<int>(symbol->field_data->offset);
				stream->Write<int>(symbol->field_data->is_serialized);
				stream->Write<int>(symbol->field_data->serialized_version);
				stream->Write<int>(symbol->field_data->is_static);
				stream->Align_To_Pointer();
				break;
			}

		case SymbolType::ClassMethod:
			{
				// CVMMethodData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));
				stream->Write<int>(symbol->method_data->return_data_type_class_index);
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->method_data->return_data_type);
				stream->Write<int>(symbol->method_data->parameter_count);
				int argument_indexes_ptr = stream->Create_Pointer();
				stream->Write<int>(symbol->method_data->is_virtual);
				stream->Write<int>(symbol->method_data->is_native);
				stream->Write<int>(symbol->method_data->is_static);
				stream->Write<int>(symbol->method_data->is_abstract);
				stream->Write<int>(symbol->method_data->is_constructor);
				stream->Write<int>(symbol->method_data->is_rpc);
				stream->Write<int>(symbol->method_data->vf_table_index);
				stream->Write<int>(symbol->method_data->register_count);
				stream->Write<int>(symbol->method_data->local_count);
				int locals_indexes_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->method_data->debug_count);
				int debug_ptr = stream->Create_Pointer();

				//stream->Write<int>(symbol->method_data->instruction_count);
				//int instructions_ptr = stream->Create_Pointer();

				stream->Write<int>(symbol->method_data->instruction_count);
				stream->Write<int>(symbol->method_data->bytecode_size);
				int bytecode_ptr = stream->Create_Pointer();

				stream->Align_To_Pointer();

				// Parameter indexes.
				stream->Patch_Pointer(argument_indexes_ptr);
				for (int j = 0; j < symbol->method_data->parameter_count; j++)
				{
					stream->Write<int>(symbol->method_data->parameter_indexes[j]);
				}

				// Local indexes.
				stream->Patch_Pointer(locals_indexes_ptr);
				for (int j = 0; j < symbol->method_data->local_count; j++)
				{
					stream->Write<int>(symbol->method_data->local_indexes[j]);
				}

				// Debug info
				stream->Patch_Pointer(debug_ptr);
				for (int j = 0; j < symbol->method_data->debug_count; j++)
				{
					CVMDebugInfo& instr = symbol->method_data->debug[j];

					// CVMDebugInfo
					stream->Write<int>(instr.bytecode_offset);
					stream->Write<int>(instr.file_index);
					stream->Write<int>(instr.row);
					stream->Write<int>(instr.column);

					// Force pack.
					stream->Align_To_Pointer();
				}

				std::vector<int> operand_ptrs;

				stream->Patch_Pointer(bytecode_ptr);
				stream->WriteBuffer(symbol->method_data->bytecode, 0, symbol->method_data->bytecode_size);

				/*
				// Instructions
				stream->Patch_Pointer(instructions_ptr);
				for (int j = 0; j < symbol->method_data->instruction_count; j++)
				{
					CVMInstruction* instr = &symbol->method_data->instructions[j];
					CInstructionMetaInfo& info = InstructionMetaInfo[instr->op_code];

					// CVMInstruction
					stream->Write<InstructionOpCode::Type>(instr->op_code);
					operand_ptrs.push_back(stream->Create_Pointer());
					
					// Force pack.
					stream->Align_To_Pointer();
				}

				// Operands.
				//DBG_LOG("Instructions For %s", symbol->name);
				for (int j = 0; j < symbol->method_data->instruction_count; j++)
				{
					CVMInstruction* instr = &symbol->method_data->instructions[j];
					CInstructionMetaInfo& info = InstructionMetaInfo[instr->op_code];

					//String value = info.Name;

					stream->Patch_Pointer(operand_ptrs.at(j));
					
					for (int k = 0; k < info.Operand_Count; k++)
					{
						//value = StringHelper::Format("%s %i", value.c_str(), instr->operands[k].int_value);
						stream->Write<int>(instr->operands[k].int_value);
						stream->Align_To_Pointer();
					}

					//DBG_LOG("[%i] %s", j, value.c_str());
				}
				*/

				break;
			}

		case SymbolType::LocalVariable:
			{
				// CVMVariableData
				stream->Patch_Pointer(symbol_table_type_specific_data_ptr.at(i));
				symbol_table_datatype_ptr.push_back(stream->Create_Pointer());
				symbol_table_datatype.push_back(symbol->variable_data->data_type);
				stream->Write<int>(symbol->variable_data->is_parameter);

				stream->Align_To_Pointer();
				break;
			}
		}
	}
	
	// Write data types.
	for (unsigned int i = 0; i < symbol_table_datatype.size(); i++)
	{
		CVMDataType* type = symbol_table_datatype.at(i);
		int ptr = symbol_table_datatype_ptr.at(i);

		stream->Patch_Pointer(ptr);
		stream->Write<CVMBaseDataType::Type>(type->type);
		int array_ptr = stream->Create_Pointer();
		STRING_PTR(type->name);
		stream->Write<int>(type->class_index);
		stream->Write<int>(0);
		stream->Align_To_Pointer();

		// If we are an array we need to assign element type.
		if (type->element_type != NULL)
		{
			bool found = false;

			// Look for type in previously written data types.
			for (unsigned int j = 0; j < i; j++)
			{
				CVMDataType* other_type = symbol_table_datatype.at(j);
				int other_ptr = symbol_table_datatype_ptr.at(j);
				
				if (type->element_type->Equal_To(other_type))
				{
					stream->Patch_Pointer(array_ptr, other_ptr, true);
					found = true;
					break;
				}
			}

			// Add data type to list.
			if (found == false)
			{
				symbol_table_datatype_ptr.push_back(array_ptr);
				symbol_table_datatype.push_back(type->element_type);
			}
		}

		// If any other similar data types exist, point them to this one.	
		for (unsigned int j = i + 1; j < symbol_table_datatype.size(); j++)
		{
			CVMDataType* other_type = symbol_table_datatype.at(j);
			int other_ptr = symbol_table_datatype_ptr.at(j);

			if (type->Equal_To(other_type))
			{
				stream->Patch_Pointer(other_ptr, ptr, true);
				symbol_table_datatype.erase(symbol_table_datatype.begin() + j);
				symbol_table_datatype_ptr.erase(symbol_table_datatype_ptr.begin() + j);
				j--;
				continue;
			}
		}
	}

	// Dump string data.
	std::vector<int> string_hashes;
	string_hashes.reserve(string_ptrs.size());

	for (unsigned int i = 0; i < string_ptrs.size(); i++)
	{
		String& val = string_values.at(i);
		string_hashes.push_back(StringHelper::Hash(val.c_str()));
	}

	for (unsigned int i = 0; i < string_ptrs.size(); i++)
	{
		int ptr = string_ptrs.at(i);
		String& val = string_values.at(i);
		int& patched = string_patched.at(i);
		int& hash = string_hashes.at(i);

		if (patched == 1)
		{
			continue;
		}

		stream->Patch_Pointer(ptr);
		stream->WriteNullTerminatedString(val.c_str());

		for (unsigned int j = i + 1; j < string_ptrs.size(); j++)
		{
			int other_ptr = string_ptrs.at(j);
			int& other_patched = string_patched.at(j);
			int& other_hash = string_hashes.at(j);

			if (other_patched == 0 && hash == other_hash)
			{
				stream->Patch_Pointer(other_ptr, ptr, true);
				other_patched = 1;
				continue;
			}
		}

		patched = 1;
	}

	//DBG_LOG("Merged VMBinary Size: %i", stream->Length() / 1024);

#undef STRING_PTR
}

CVMBaseDataType::Type CVMBinary::DataTypeToVMDataType(CDataType* type)
{
	if (dynamic_cast<CIntDataType*>(type) != NULL)
	{
		return CVMBaseDataType::Int;
	}
	else if (dynamic_cast<CBoolDataType*>(type) != NULL)
	{
		return CVMBaseDataType::Bool;
	}
	else if (dynamic_cast<CFloatDataType*>(type) != NULL)
	{
		return CVMBaseDataType::Float;
	}
	else if (dynamic_cast<CStringDataType*>(type) != NULL)
	{
		return CVMBaseDataType::String;
	}
	else if (dynamic_cast<CVoidDataType*>(type) != NULL)
	{
		return CVMBaseDataType::Void;
	}
	else if (dynamic_cast<CArrayDataType*>(type) != NULL)
	{
		return CVMBaseDataType::Array;
	}
	else if (dynamic_cast<CObjectDataType*>(type) != NULL)
	{
		return CVMBaseDataType::Object;
	}

	DBG_ASSERT(false);
	return CVMBaseDataType::Void;
}

void CVMBinary::Deserialize(PatchedBinaryStream* stream)
{
	m_resource_data = (CVMBinaryData*)stream->Take_Data();
}

int CVMBinary::Add_String_To_Merge_Table(String str)
{
	int hash = StringHelper::Hash(str.c_str());

	int index = 0;
	if (m_merge_string_table_lookup.Get(hash, index))
	{
		return index;
	}

	int i = m_merge_string_table.size();

	m_merge_string_table.push_back(str);
	m_merge_string_table_lookup.Set(hash, i);

	return i;
}

int CVMBinary::Add_Symbol_To_Merge_Table(CVMBinarySymbol* symbol)
{
	int index = 0;
	if (m_merge_symbol_table_lookup.Get(symbol->unique_id, index))
	{
		return index;
	}

	MergeBinarySymbol sym;
	sym.symbol = symbol;
	sym.index = m_merge_symbol_table.size();
	sym.have_instructions_been_patched = false;
	sym.have_symbols_been_patched = false;

	m_merge_symbol_table.push_back(sym);
	m_merge_symbol_table_lookup.Set(symbol->unique_id, sym.index);

	return sym.index;
}

void CVMBinary::Patch_Index_Array(int* src_array, int src_count, std::vector<int> remap)
{
	for (int i = 0; i < src_count; i++)
	{
		if (src_array[i] >= 0)
			src_array[i] = remap.at(src_array[i]);
	}
}

void CVMBinary::Patch_Data_Type(CVMDataType* dt, std::vector<int> symbol_remaps)
{
	if (dt->class_index_patched == false)
	{
		Patch_Index_Array(&dt->class_index, 1, symbol_remaps);
		dt->class_index_patched = true;
	}

	if (dt->element_type != NULL)
	{
		Patch_Data_Type(dt->element_type, symbol_remaps);
	}
}

void CVMBinary::Patch_Symbols(std::vector<int> symbol_remaps, std::vector<int> string_remaps)
{
	// Keep in sync with CVirtualMachine::Patch_Symbols

	int index = 0;
	for (std::vector<MergeBinarySymbol>::iterator iter = m_merge_symbol_table.begin(); iter != m_merge_symbol_table.end(); iter++, index++)
	{
		MergeBinarySymbol& sym = *iter;
		if (sym.have_symbols_been_patched == false)
		{
			// Patch children indexes.
			Patch_Index_Array(&sym.symbol->class_scope_index, 1, symbol_remaps);
			Patch_Index_Array(sym.symbol->children_indexes, sym.symbol->children_count, symbol_remaps);

			Patch_Index_Array(&sym.symbol->debug.FileNameIndex, 1, string_remaps);

			// Patch type-specifc data.
			switch (sym.symbol->type)
			{
			case SymbolType::ClassProperty:
				{
					Patch_Data_Type(sym.symbol->property_data->data_type, symbol_remaps);
					Patch_Index_Array(&sym.symbol->property_data->get_property_symbol, 1, symbol_remaps);
					Patch_Index_Array(&sym.symbol->property_data->set_property_symbol, 1, symbol_remaps);
					break;
				}
			case SymbolType::ClassMethod:
				{
					Patch_Data_Type(sym.symbol->method_data->return_data_type, symbol_remaps);
					Patch_Index_Array(&sym.symbol->method_data->return_data_type_class_index, 1, symbol_remaps);
					Patch_Index_Array(sym.symbol->method_data->parameter_indexes, sym.symbol->method_data->parameter_count, symbol_remaps);
					Patch_Index_Array(sym.symbol->method_data->local_indexes, sym.symbol->method_data->local_count, symbol_remaps);
					break;
				}
			case SymbolType::ClassField:
				{
					Patch_Data_Type(sym.symbol->field_data->data_type, symbol_remaps);
					break;
				}
			case SymbolType::LocalVariable:
				{
					Patch_Data_Type(sym.symbol->variable_data->data_type, symbol_remaps);
					break;
				}
			case SymbolType::Class:
				{
					Patch_Data_Type(sym.symbol->class_data->data_type, symbol_remaps);
					Patch_Index_Array(&sym.symbol->class_data->default_constructor_index, 1, symbol_remaps);
					Patch_Index_Array(&sym.symbol->class_data->super_class_index, 1, symbol_remaps);
					Patch_Index_Array(&sym.symbol->class_data->default_state_index, 1, symbol_remaps);
					Patch_Index_Array(sym.symbol->class_data->state_indexes, sym.symbol->class_data->state_count, symbol_remaps);
					Patch_Index_Array(sym.symbol->class_data->fields_indexes, sym.symbol->class_data->field_count, symbol_remaps);
					Patch_Index_Array(sym.symbol->class_data->method_indexes, sym.symbol->class_data->method_count, symbol_remaps);
					Patch_Index_Array(sym.symbol->class_data->virtual_table_indexes, sym.symbol->class_data->virtual_table_size, symbol_remaps);
					
					if (sym.symbol->class_data->is_replicated != 0)
					{
						for (int i = 0; i < sym.symbol->class_data->replication_info->variable_count; i++)
						{
							Patch_Index_Array(&sym.symbol->class_data->replication_info->variables[i].var_symbol_index, 1, symbol_remaps);
							Patch_Index_Array(&sym.symbol->class_data->replication_info->variables[i].prop_symbol_index, 1, symbol_remaps);					
						}
					}

					for (int i = 0; i < sym.symbol->class_data->interface_count; i++)
					{
						CVMInterface* inter = sym.symbol->class_data->interfaces + i;
						Patch_Index_Array(&inter->interface_symbol_index, 1, symbol_remaps);						
					}

					break;
				}
			case SymbolType::ClassState:
				{
					Patch_Index_Array(sym.symbol->state_data->event_indexes, sym.symbol->state_data->event_count, symbol_remaps);					
					break;
				}
			}
		}

		sym.have_symbols_been_patched = true;
	}
}

void CVMBinary::Patch_Instructions(std::vector<int> symbol_remaps, std::vector<int> string_remaps)
{
	// Keep in sync with CVirtualMachine::Patch_Instructions

	for (std::vector<MergeBinarySymbol>::iterator iter = m_merge_symbol_table.begin(); iter != m_merge_symbol_table.end(); iter++)
	{
		MergeBinarySymbol& sym = *iter;
		if (sym.have_instructions_been_patched == false &&
			sym.symbol->type == SymbolType::ClassMethod)
		{
			char* buffer = sym.symbol->method_data->bytecode;
			char* end_buffer = sym.symbol->method_data->bytecode + sym.symbol->method_data->bytecode_size;
			int index = 0;
			while (buffer < end_buffer)
			{
				char* instruction_start = buffer;

				// Decode next instruction.
				CInstruction instruction;
				instruction.Decode(buffer);				

				CInstruction original_instruction = instruction;

				CInstructionMetaInfo info = InstructionMetaInfo[(int)instruction.OpCode];
			//	DBG_LOG("[0x%08x/0x%08x - %i/%i] %s", buffer, end_buffer, index++, sym.symbol->method_data->instruction_count, info.Name);

				// Patch operands.
				for (int j = 0; j < info.Operand_Count; j++)
				{
					if (info.Operand_Types[j] == OperandType::Symbol)
					{
						instruction.Operands[j].int_value = symbol_remaps.at(instruction.Operands[j].int_value);
					}
					else if (info.Operand_Types[j] == OperandType::String)
					{
						instruction.Operands[j].int_value = string_remaps.at(instruction.Operands[j].int_value);
					}
				}

				// Re-encode into the byte code.
				instruction.Encode(instruction_start);
				DBG_ASSERT(instruction_start == buffer);
			}

			/*for (int i = 0; i < sym.symbol->method_data->instruction_count; i++)
			{
				CVMInstruction* instruction = sym.symbol->method_data->instructions + i;
				CInstructionMetaInfo info = InstructionMetaInfo[(int)instruction->op_code];
				for (int j = 0; j < info.Operand_Count; j++)
				{
					if (info.Operand_Types[j] == OperandType::Symbol)
					{
					//	DBG_LOG("Patched symbol index (%i -> %i) on instruction.", instruction->operands[j].int_value, symbol_remaps.at(instruction->operands[j].int_value));
						instruction->operands[j].int_value = symbol_remaps.at(instruction->operands[j].int_value);
					}
					else if (info.Operand_Types[j] == OperandType::String)
					{
					//	DBG_LOG("Patched string index (%i -> %i) on instruction.", instruction->operands[j].int_value, string_remaps.at(instruction->operands[j].int_value));
						instruction->operands[j].int_value = string_remaps.at(instruction->operands[j].int_value);
					}
				}
			}
			*/

			for (int i = 0; i < sym.symbol->method_data->debug_count; i++)
			{
				CVMDebugInfo* instruction = sym.symbol->method_data->debug + i;
				instruction->file_index = string_remaps.at(instruction->file_index);
			}
		}
	
		sym.have_instructions_been_patched = true;
	}
}

void CVMBinary::Merge(CVMBinary* binary)
{
	CVMBinaryData* data = binary->m_resource_data;

	std::vector<int> string_index_remaps;
	std::vector<int> symbol_index_remaps;

	string_index_remaps.reserve(data->string_table_count);
	symbol_index_remaps.reserve(data->symbol_table_count);

	m_merge_symbol_table.reserve(m_merge_symbol_table.size() + data->symbol_table_count);
	m_merge_string_table.reserve(m_merge_string_table.size() + data->string_table_count);
	
	// Add binary's strings to master string table.
	for (int i = 0; i < data->string_table_count; i++)
	{
		const char* str = data->string_table[i];
		string_index_remaps.push_back(Add_String_To_Merge_Table(str));
	}

	// Add binary's symbols to master symbol table.
	for (int i = 0; i < data->symbol_table_count; i++)
	{
		CVMBinarySymbol* sym = data->symbols[i];
		symbol_index_remaps.push_back(Add_Symbol_To_Merge_Table(sym));
	}
	
	// Go through symbol instructions and update references
	// to symbol/string indexes.
	Patch_Symbols(symbol_index_remaps, string_index_remaps);
	Patch_Instructions(symbol_index_remaps, string_index_remaps);
}