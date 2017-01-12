/* *****************************************************************

		CInstructionBuffer.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CINSTRUCTIONBUFFER_H_
#define _CINSTRUCTIONBUFFER_H_

#include "XScript/VirtualMachine/CInstruction.h"

#include "Generic/Types/String.h"
#include <vector>

class CDataType;
class CSemanter;
class CSymbol;

// =================================================================
//	Used to construct a buffer of opcode instructions. Keeps track 
//	of registers, and deals with a lot of the boring stuff.
// =================================================================
struct CInstructionBuffer
{
public:
	enum
	{
		max_registers = 255
	};

private:
	int m_free_register_count;
	int m_free_registers[max_registers];

	std::vector<int>			m_jmp_target_offsets;
	std::vector<CInstruction>	m_instructions;
	std::vector<CDebugInfo>		m_debug_info;

	CSemanter*					m_semanter;

	int							m_max_used_registers;

private:
	InstructionOpCode::Type GetOpCodeByTypeIFSO(CDataType* type, InstructionOpCode::Type int_opcode, InstructionOpCode::Type float_opcode, InstructionOpCode::Type string_opcode, InstructionOpCode::Type object_opcode);
	InstructionOpCode::Type GetOpCodeByTypeIFS(CDataType* type, InstructionOpCode::Type int_opcode, InstructionOpCode::Type float_opcode, InstructionOpCode::Type string_opcode);
	InstructionOpCode::Type GetOpCodeByTypeIF(CDataType* type, InstructionOpCode::Type int_opcode, InstructionOpCode::Type float_opcode);

public:

	CInstructionBuffer();

	// Gets the information this buffer holds.
	int Get_Instruction_Count();
	CInstruction& Get_Instruction(int index);
	int GetMaxUsedRegisterCount();

	// Gets the information this buffer holds.
	int Get_Debug_Info_Count();
	CDebugInfo& Get_Debug_Info(int index);

	// Gets information on the jump table.
	int Get_JmpTarget_Count();
	int Get_JmpTarget(int index);

	// Gets string table information for this instruction buffer.

	// Sets the semanter required to deal with type-casting
	// and other fun aspects of instruction generation.
	void SetSemanter(CSemanter* semanater);

	// Register management.
	int NewRegister();
	void FreeRegister(int reg);
	void SetRegisterType(int reg, CDataType* type);
	CDataType* GetRegisterType(int reg);

	// Jump target management.
	int CreateJmpTarget();
	void PatchJmpTarget(int target);

	// Instruction emission.
	void EmitMov		(int dest_reg, int src_reg, CDataType* type);

	void EmitCast		(int reg, CDataType* from_type, CDataType* to_type, bool safe);

	void EmitStoreLocal	(int reg, CSymbol* variable);
	void EmitLoadLocal	(int reg, CSymbol* variable);
	void EmitStoreField	(int value_reg, int object_reg, CSymbol* variable);
	void EmitLoadField	(int value_reg, int object_reg, CSymbol* variable);

	void EmitLoadLiteral(int reg, int value);
	void EmitLoadLiteral(int reg, float value);
	void EmitLoadStringLiteral(int reg, int string_table_index);
	void EmitLoadNull	(int reg);
	void EmitLoadThis	(int reg);
	void EmitLoadType	(int ret_reg, CSymbol* class_type);

	void EmitPushLiteral(int value);
	void EmitPushLiteral(float value);
	void EmitPushStringLiteral(int string_table_index);
	void EmitPushNull	();

	void EmitAnd		(int lvalue_reg, int rvalue_reg);
	void EmitOr			(int lvalue_reg, int rvalue_reg);
	void EmitXOr		(int lvalue_reg, int rvalue_reg);
	void EmitSHL		(int lvalue_reg, int rvalue_reg);
	void EmitSHR		(int lvalue_reg, int rvalue_reg);
	void EmitMod		(int lvalue_reg, int rvalue_reg);

	void EmitNot		(int lvalue_reg);
	void EmitLogicalNot	(int lvalue_reg);
	void EmitInc		(int lvalue_reg);
	void EmitDec		(int lvalue_reg);
	void EmitPos		(CDataType* type, int lvalue_reg);
	void EmitNeg		(CDataType* type, int lvalue_reg);

	void EmitAdd		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitSub		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitMul		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitDiv		(CDataType* type, int lvalue_reg, int rvalue_reg);

	void EmitCmpEQ		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitCmpNE		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitCmpG		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitCmpGE		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitCmpL		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitCmpLE		(CDataType* type, int lvalue_reg, int rvalue_reg);
	void EmitCmpNull	(CDataType* type, int lvalue_reg);

	void EmitPush		(CDataType* type, int lvalue_reg);
	void EmitPop		(CDataType* type, int lvalue_reg);

	void EmitRet		(CDataType* type, int result_reg);
	void EmitRet		();

	void EmitJmp		(int jmp_target);
	void EmitJmpOnTrue	(int cmp_reg, int jmp_target);
	void EmitJmpOnFalse	(int cmp_reg, int jmp_target);

	void EmitNew		(int ret_reg, int class_symbol_index);
	void EmitNewArray	(int ret_reg, int size_reg, int class_symbol_index);
	void EmitGetLength	(int ret_reg, int array_reg);
	void EmitGetIndex	(int ret_reg, int array_reg, int index_reg);

	void EmitInvokeMethod(int result_reg, int object_reg, int function_symbol_index);
	void EmitInvokeNonVirtualMethod(int result_reg, int object_reg, int function_symbol_index);
	void EmitInvokeStatic(int result_reg, int function_symbol_index);

	void EmitDebugTrace(int row, int column, int file_name_index);

	void EmitPushState	(int state_symbol_index);
	void EmitChangeState(int state_symbol_index);
	void EmitPopState	();

};

#endif