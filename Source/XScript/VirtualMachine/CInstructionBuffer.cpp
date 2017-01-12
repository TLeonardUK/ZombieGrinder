/* *****************************************************************

		CInstructionBuffer.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CInstructionBuffer.h"
#include "XScript/VirtualMachine/CSymbol.h"
#include "XScript/Parser/Types/Helper/CDataType.h"
#include "XScript/Parser/Types/Helper/CNullDataType.h"
#include "XScript/Parser/Types/CArrayDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CBoolDataType.h"
#include "XScript/Parser/Types/CFloatDataType.h"
#include "XScript/Parser/Types/CIntDataType.h"
#include "XScript/Parser/Types/CObjectDataType.h"
#include "XScript/Parser/Types/CStringDataType.h"
#include "XScript/Parser/Nodes/TopLevel/CClassASTNode.h"
#include "XScript/Parser/Nodes/TopLevel/CClassMemberASTNode.h"

CInstructionBuffer::CInstructionBuffer()
	: m_semanter(false)
	, m_free_register_count(max_registers)
	, m_max_used_registers(0)
{
	for (int i = 0; i < max_registers; i++)
		m_free_registers[i] = max_registers - (i + 1);
}

void CInstructionBuffer::SetSemanter(CSemanter* semanater)
{
	m_semanter = semanater;
}

int CInstructionBuffer::Get_Instruction_Count()
{
	return m_instructions.size();
}

int CInstructionBuffer::GetMaxUsedRegisterCount()
{
	return m_max_used_registers;
}

CInstruction& CInstructionBuffer::Get_Instruction(int index)
{
	return m_instructions[index];
}

int CInstructionBuffer::Get_Debug_Info_Count()
{
	return m_debug_info.size();
}

CDebugInfo& CInstructionBuffer::Get_Debug_Info(int index)
{
	return m_debug_info[index];
}

int CInstructionBuffer::Get_JmpTarget_Count()
{
	return m_jmp_target_offsets.size();
}

int CInstructionBuffer::Get_JmpTarget(int index)
{
	return m_jmp_target_offsets.at(index);
}

int CInstructionBuffer::NewRegister()
{
	static int counter = 0;
	DBG_ASSERT_STR(m_free_register_count > 0, "Ran out of free registers!");
	int reg = m_free_registers[--m_free_register_count];
	counter++;

	if (reg + 1 > m_max_used_registers)
	{
		m_max_used_registers = reg + 1;
	}

	return reg;
}

void CInstructionBuffer::FreeRegister(int reg)
{
	if (reg < 0)
		return;

	DBG_ASSERT(reg >= 0 && reg < max_registers);
	DBG_ASSERT(m_free_register_count < max_registers);
	for (int i = 0; i < m_free_register_count; i++)
	{
		DBG_ASSERT(m_free_registers[i] != reg);
	}

	m_free_registers[m_free_register_count] = reg;
	m_free_register_count++;
}

int CInstructionBuffer::CreateJmpTarget()
{
	m_jmp_target_offsets.push_back(0);
	return m_jmp_target_offsets.size() - 1;
}

void CInstructionBuffer::PatchJmpTarget(int target)
{
	m_jmp_target_offsets.at(target) = m_instructions.size();
}

InstructionOpCode::Type CInstructionBuffer::GetOpCodeByTypeIFSO(CDataType* type, 
																InstructionOpCode::Type int_opcode, 
																InstructionOpCode::Type float_opcode, 
																InstructionOpCode::Type string_opcode, 
																InstructionOpCode::Type object_opcode)
{
	if (dynamic_cast<CIntDataType*>(type) ||
		dynamic_cast<CBoolDataType*>(type))
	{
		return int_opcode;
	}
	else if (dynamic_cast<CFloatDataType*>(type))
	{
		return float_opcode;
	}
	else if (dynamic_cast<CStringDataType*>(type))
	{
		return string_opcode;
	}
	else if (dynamic_cast<CObjectDataType*>(type)	||
			 dynamic_cast<CNullDataType*>(type)		||
			 dynamic_cast<CArrayDataType*>(type))
	{
		return object_opcode;
	}
	else
	{
		DBG_ASSERT(false);
	}
	return InstructionOpCode::COUNT;
}

InstructionOpCode::Type CInstructionBuffer::GetOpCodeByTypeIFS(CDataType* type, 
															   InstructionOpCode::Type int_opcode, 
															   InstructionOpCode::Type float_opcode, 
															   InstructionOpCode::Type string_opcode)
{
	if (dynamic_cast<CIntDataType*>(type) ||
		dynamic_cast<CBoolDataType*>(type))
	{
		return int_opcode;
	}
	else if (dynamic_cast<CFloatDataType*>(type))
	{
		return float_opcode;
	}
	else if (dynamic_cast<CStringDataType*>(type))
	{
		return string_opcode;
	}
	else
	{
		DBG_ASSERT(false);
	}
	return InstructionOpCode::COUNT;
}

InstructionOpCode::Type CInstructionBuffer::GetOpCodeByTypeIF(CDataType* type, 
															  InstructionOpCode::Type int_opcode, 
															  InstructionOpCode::Type float_opcode)
{
	if (dynamic_cast<CIntDataType*>(type) ||
		dynamic_cast<CBoolDataType*>(type))
	{
		return int_opcode;
	}
	else if (dynamic_cast<CFloatDataType*>(type))
	{
		return float_opcode;
	}
	else
	{
		DBG_ASSERT(false);
	}
	return InstructionOpCode::COUNT;
}

void CInstructionBuffer::EmitCast(int reg, CDataType* from_type, CDataType* to_type, bool safe)
{
	InstructionOpCode::Type opcode;
	int class_symbol_index = 0;

	if (dynamic_cast<CBoolDataType*>(to_type) != NULL)
	{
		if (dynamic_cast<CBoolDataType*>(from_type) != NULL)		return;	
		else if (dynamic_cast<CIntDataType*>(from_type) != NULL)	opcode = InstructionOpCode::ibCast;
		else if (dynamic_cast<CFloatDataType*>(from_type) != NULL)	opcode = InstructionOpCode::fiCast;
		else if (dynamic_cast<CArrayDataType*>(from_type) != NULL)	opcode = InstructionOpCode::obCast;
		else if (dynamic_cast<CStringDataType*>(from_type) != NULL)	opcode = InstructionOpCode::sbCast;
		else if (dynamic_cast<CObjectDataType*>(from_type) != NULL)	opcode = InstructionOpCode::obCast;
		else DBG_ASSERT(false);
	}
	else if (dynamic_cast<CIntDataType*>(to_type) != NULL)
	{
		if (dynamic_cast<CBoolDataType*>(from_type) != NULL)		return;
		else if (dynamic_cast<CIntDataType*>(from_type) != NULL)	return;
		else if (dynamic_cast<CFloatDataType*>(from_type) != NULL)	opcode = InstructionOpCode::fiCast;
		else if (dynamic_cast<CStringDataType*>(from_type) != NULL)	opcode = InstructionOpCode::siCast;
		else if (dynamic_cast<CArrayDataType*>(from_type) != NULL)	opcode = InstructionOpCode::obCast;
		else if (dynamic_cast<CObjectDataType*>(from_type) != NULL)	opcode = InstructionOpCode::obCast;
		else DBG_ASSERT(false);
	}
	else if (dynamic_cast<CFloatDataType*>(to_type) != NULL)
	{
		if (dynamic_cast<CBoolDataType*>(from_type) != NULL)		opcode = InstructionOpCode::ifCast;
		else if (dynamic_cast<CIntDataType*>(from_type) != NULL)	opcode = InstructionOpCode::ifCast;
		else if (dynamic_cast<CFloatDataType*>(from_type) != NULL)	return;
		else if (dynamic_cast<CStringDataType*>(from_type) != NULL)	opcode = InstructionOpCode::sfCast;
		else if (dynamic_cast<CArrayDataType*>(from_type) != NULL)	opcode = InstructionOpCode::ofCast;
		else if (dynamic_cast<CObjectDataType*>(from_type) != NULL)	opcode = InstructionOpCode::ofCast;
		else DBG_ASSERT(false);
	}
	else if (dynamic_cast<CStringDataType*>(to_type) != NULL)
	{
		if (dynamic_cast<CBoolDataType*>(from_type) != NULL)		opcode = InstructionOpCode::isCast;
		else if (dynamic_cast<CIntDataType*>(from_type) != NULL)	opcode = InstructionOpCode::isCast;
		else if (dynamic_cast<CFloatDataType*>(from_type) != NULL)	opcode = InstructionOpCode::fsCast;
		else if (dynamic_cast<CStringDataType*>(from_type) != NULL)	return;
		else DBG_ASSERT(false);
	}
	else if (dynamic_cast<CObjectDataType*>(to_type))// != NULL &&
			 //dynamic_cast<CObjectDataType*>(from_type) != NULL)
	{
		CClassASTNode* from_class = from_type->GetClass(m_semanter);
		CClassASTNode* to_class = to_type->GetClass(m_semanter);

		// Converting interface to object.
		if (from_class->IsInterface == true ||
			to_class->IsInterface == true)
		{
			opcode				= safe ? InstructionOpCode::ooSafeCast : InstructionOpCode::ooCast;
			class_symbol_index	= to_class->TranslatedSymbol->Index;
		}

		// Upcasting (make sure we are not an array, arrays are special cases).
		else if (//dynamic_cast<CArrayDataType*>(from_type) == NULL &&
				 //dynamic_cast<CArrayDataType*>(to_type) == NULL &&
				 from_class->InheritsFromClass(m_semanter, to_class))
		{
			return;
		}

		// Downcasting
		else
		{			
			opcode				= safe ? InstructionOpCode::ooSafeCast : InstructionOpCode::ooCast;
			class_symbol_index	= to_class->TranslatedSymbol->Index;
		}
	}
	else
	{
		DBG_ASSERT(false);
	}

	m_instructions.push_back(CInstruction::Create(opcode, reg, class_symbol_index));
}

void CInstructionBuffer::EmitMov(int dest_reg, int src_reg, CDataType* type)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(type,
						InstructionOpCode::iMov,
						InstructionOpCode::fMov,
						InstructionOpCode::sMov,
						InstructionOpCode::oMov);

	m_instructions.push_back(CInstruction::Create(opcode, dest_reg, src_reg));
}

void CInstructionBuffer::EmitStoreLocal(int reg, CSymbol* symbol)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(symbol->LocalDataType,
						InstructionOpCode::iStoreLocal,
						InstructionOpCode::fStoreLocal,
						InstructionOpCode::sStoreLocal,
						InstructionOpCode::oStoreLocal);

	m_instructions.push_back(CInstruction::Create(opcode, reg, symbol->LocalOffset));
}	

void CInstructionBuffer::EmitLoadLocal(int reg, CSymbol* symbol)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(symbol->LocalDataType,
		InstructionOpCode::iLoadLocal,
		InstructionOpCode::fLoadLocal,
		InstructionOpCode::sLoadLocal,
		InstructionOpCode::oLoadLocal);

	m_instructions.push_back(CInstruction::Create(opcode, reg, symbol->LocalOffset));
}

void CInstructionBuffer::EmitLoadType(int reg, CSymbol* symbol)
{
	InstructionOpCode::Type opcode = InstructionOpCode::oLoadType;
	m_instructions.push_back(CInstruction::Create(opcode, reg, symbol->Index));
}

void CInstructionBuffer::EmitStoreField(int value_reg, int object_reg, CSymbol* symbol)
{
	CClassMemberASTNode* member_node = dynamic_cast<CClassMemberASTNode*>(symbol->Node);
	if (member_node->IsStatic == true)
	{
		int class_symbol_index = member_node->FindClassScope(m_semanter)->TranslatedSymbol->Index;

		InstructionOpCode::Type opcode =
			GetOpCodeByTypeIFSO(symbol->LocalDataType,
			InstructionOpCode::iStoreStatic,
			InstructionOpCode::fStoreStatic,
			InstructionOpCode::sStoreStatic,
			InstructionOpCode::oStoreStatic);

		m_instructions.push_back(CInstruction::Create(opcode, value_reg, class_symbol_index, symbol->FieldOffset));	
	}
	else
	{
		InstructionOpCode::Type opcode =
			GetOpCodeByTypeIFSO(symbol->LocalDataType,
			InstructionOpCode::iStoreField,
			InstructionOpCode::fStoreField,
			InstructionOpCode::sStoreField,
			InstructionOpCode::oStoreField);

		m_instructions.push_back(CInstruction::Create(opcode, value_reg, object_reg, symbol->FieldOffset));	
	}
}

void CInstructionBuffer::EmitLoadField(int value_reg, int object_reg, CSymbol* symbol)
{
	CClassMemberASTNode* member_node = dynamic_cast<CClassMemberASTNode*>(symbol->Node);
	if (member_node->IsStatic == true)
	{
		int class_symbol_index = member_node->FindClassScope(m_semanter)->TranslatedSymbol->Index;

		InstructionOpCode::Type opcode =
			GetOpCodeByTypeIFSO(symbol->LocalDataType,
			InstructionOpCode::iLoadStatic,
			InstructionOpCode::fLoadStatic,
			InstructionOpCode::sLoadStatic,
			InstructionOpCode::oLoadStatic);

		m_instructions.push_back(CInstruction::Create(opcode, value_reg, class_symbol_index, symbol->FieldOffset));	
	}
	else
	{
		InstructionOpCode::Type opcode =
			GetOpCodeByTypeIFSO(symbol->FieldDataType,
			InstructionOpCode::iLoadField,
			InstructionOpCode::fLoadField,
			InstructionOpCode::sLoadField,
			InstructionOpCode::oLoadField);

		m_instructions.push_back(CInstruction::Create(opcode, value_reg, object_reg, symbol->FieldOffset));
	}
}



void CInstructionBuffer::EmitPushLiteral(int value)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iPushLiteral, value));
}

void CInstructionBuffer::EmitPushLiteral(float value)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::fPushLiteral, value));
}

void CInstructionBuffer::EmitPushStringLiteral(int string_table_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::sPushLiteral, string_table_index));
}

void CInstructionBuffer::EmitPushNull()
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::oPushNull));
}


void CInstructionBuffer::EmitLoadLiteral(int reg, int value)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iLoadLiteral, reg, value));
}

void CInstructionBuffer::EmitLoadLiteral(int reg, float value)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::fLoadLiteral, reg, value));
}

void CInstructionBuffer::EmitLoadStringLiteral(int reg, int string_table_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::sLoadLiteral, reg, string_table_index));
}

void CInstructionBuffer::EmitLoadNull(int reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::oLoadNull, reg));
}

void CInstructionBuffer::EmitLoadThis(int reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::oLoadThis, reg));
}

void CInstructionBuffer::EmitOr(int lvalue_reg, int rvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iOr, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitAnd(int lvalue_reg, int rvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iAnd, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitXOr(int lvalue_reg, int rvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iXOr, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitSHL(int lvalue_reg, int rvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iSHL, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitSHR(int lvalue_reg, int rvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iSHR, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitMod(int lvalue_reg, int rvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iMod, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitNot(int lvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iNot, lvalue_reg));
}

void CInstructionBuffer::EmitLogicalNot(int lvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iLogicalNot, lvalue_reg));
}

void CInstructionBuffer::EmitInc(int lvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iInc, lvalue_reg));
}

void CInstructionBuffer::EmitDec(int lvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::iDec, lvalue_reg));
}

void CInstructionBuffer::EmitPos(CDataType* type, int lvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iPos,
		InstructionOpCode::fPos);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg));
}

void CInstructionBuffer::EmitNeg(CDataType* type, int lvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iNeg,
		InstructionOpCode::fNeg);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg));
}

void CInstructionBuffer::EmitAdd(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFS(type,
		InstructionOpCode::iAdd,
		InstructionOpCode::fAdd,
		InstructionOpCode::sAdd);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitPush(CDataType* type, int lvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(type,
		InstructionOpCode::iPush,
		InstructionOpCode::fPush,
		InstructionOpCode::sPush,
		InstructionOpCode::oPush);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg));
}

void CInstructionBuffer::EmitPop(CDataType* type, int lvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(type,
		InstructionOpCode::iPop,
		InstructionOpCode::fPop,
		InstructionOpCode::sPop,
		InstructionOpCode::oPop);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg));
}

void CInstructionBuffer::EmitSub(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iSub,
		InstructionOpCode::fSub);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitMul(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iMul,
		InstructionOpCode::fMul);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitDiv(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iDiv,
		InstructionOpCode::fDiv);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitCmpEQ(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(type,
		InstructionOpCode::iCmpEQ,
		InstructionOpCode::fCmpEQ,
		InstructionOpCode::sCmpEQ,
		InstructionOpCode::oCmpEQ);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitCmpNE(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(type,
		InstructionOpCode::iCmpNE,
		InstructionOpCode::fCmpNE,
		InstructionOpCode::sCmpNE,
		InstructionOpCode::oCmpNE);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitCmpG(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iCmpG,
		InstructionOpCode::fCmpG);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitCmpGE(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iCmpGE,
		InstructionOpCode::fCmpGE);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitCmpL(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iCmpL,
		InstructionOpCode::fCmpL);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitCmpNull(CDataType* type, int lvalue_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::oCmpNull, lvalue_reg));
}

void CInstructionBuffer::EmitCmpLE(CDataType* type, int lvalue_reg, int rvalue_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIF(type,
		InstructionOpCode::iCmpLE,
		InstructionOpCode::fCmpLE);

	m_instructions.push_back(CInstruction::Create(opcode, lvalue_reg, rvalue_reg));
}

void CInstructionBuffer::EmitRet(CDataType* type, int result_reg)
{
	InstructionOpCode::Type opcode =
		GetOpCodeByTypeIFSO(type,
		InstructionOpCode::iRet,
		InstructionOpCode::fRet,
		InstructionOpCode::sRet,
		InstructionOpCode::oRet);

	m_instructions.push_back(CInstruction::Create(opcode, result_reg));
}

void CInstructionBuffer::EmitRet()
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::vRet));
}

void CInstructionBuffer::EmitJmp(int jmp_target)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::Jmp, jmp_target));
}

void CInstructionBuffer::EmitJmpOnTrue(int cmp_reg, int jmp_target)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::JmpOnTrue, cmp_reg, jmp_target));
}

void CInstructionBuffer::EmitJmpOnFalse(int cmp_reg, int jmp_target)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::JmpOnFalse, cmp_reg, jmp_target));
}

void CInstructionBuffer::EmitNew(int ret_reg, int class_symbol_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::NewObject, ret_reg, class_symbol_index));
}

void CInstructionBuffer::EmitNewArray(int ret_reg, int size_reg, int class_symbol_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::NewArray, ret_reg, size_reg, class_symbol_index));
}

void CInstructionBuffer::EmitInvokeMethod(int result_reg, int object_reg, int function_symbol_index)
{
	// void dosen't require register so will use -1, encoded instructions are unsigned, so lets make that 0 :).
	if (result_reg == -1) result_reg = 0;
	if (object_reg == -1) object_reg = 0;
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::InvokeMethod, result_reg, object_reg, function_symbol_index));
}

void CInstructionBuffer::EmitInvokeNonVirtualMethod(int result_reg, int object_reg, int function_symbol_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::InvokeNonVirtualMethod, result_reg, object_reg, function_symbol_index));
}

void CInstructionBuffer::EmitInvokeStatic(int result_reg, int function_symbol_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::InvokeStatic, result_reg, function_symbol_index));
}	

void CInstructionBuffer::EmitDebugTrace(int row, int column, int file_name_index)
{
	if (row < 0 || column < 0 || file_name_index < 0)
	{
		DBG_ASSERT(false);
	}
	m_debug_info.push_back(CDebugInfo::Create(m_instructions.size(), file_name_index, row, column));//CInstruction::Create(InstructionOpCode::DebugTrace, file_name_index, row, column));
//	m_instructions.push_back(CInstruction::Create(InstructionOpCode::DebugTrace, file_name_index, row, column));
}

void CInstructionBuffer::EmitPushState(int state_symbol_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::PushState, state_symbol_index));
}

void CInstructionBuffer::EmitChangeState(int state_symbol_index)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::ChangeState, state_symbol_index));
}

void CInstructionBuffer::EmitPopState()
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::PopState));
}

void CInstructionBuffer::EmitGetLength(int ret_reg, int array_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::GetLength, ret_reg, array_reg));
}

void CInstructionBuffer::EmitGetIndex(int ret_reg, int array_reg, int index_reg)
{
	m_instructions.push_back(CInstruction::Create(InstructionOpCode::GetIndex, ret_reg, array_reg, index_reg));
}