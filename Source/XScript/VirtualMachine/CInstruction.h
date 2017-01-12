/* *****************************************************************

		CInstruction.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CINSTRUCTION_H_
#define _CINSTRUCTION_H_

#include "Generic/Types/String.h"
#include <vector>
#include <stdarg.h>

#include "Generic/Helper/StringHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"

class CSymbol;
class CActivationContext;
struct CVMInstruction;

// =================================================================
//	Struct stores information on an instruction operand.
// =================================================================
struct COperand
{
	enum 
	{
		max_operands_per_instruction	= 3
	};

	union
	{
		float float_value;
		int   int_value;
	};
};

struct OperandType
{
	enum Type
	{
		Float,
		Int,
		Register,
		Symbol,
		JumpTarget,
		String
	};
};

// =================================================================
//	Enumeration of different instruction types.
// =================================================================
struct InstructionOpCode 
{
	enum Type
	{
		#define INSTRUCTION(name, operand_count, ...) name,
		#include "XScript/VirtualMachine/InstructionList.inc"
		#undef INSTRUCTION

		COUNT
	};
};

// =================================================================
//	Enumeration of different instruction types.
// =================================================================
class CInstructionMetaInfo
{
public:
	const char*			Name;
	int					Operand_Count;
	OperandType::Type	Operand_Types[COperand::max_operands_per_instruction];

public:
	CInstructionMetaInfo()
	{
	}

	static CInstructionMetaInfo Create(const char* name, int operand_count, ...)
	{
		CInstructionMetaInfo instr;
		instr.Name			= name;
		instr.Operand_Count	= operand_count;

		va_list va;
		va_start(va, operand_count); 

		for (int i = 0; i < operand_count; i++)
		{
			instr.Operand_Types[i] = (OperandType::Type)va_arg(va, int); 
		}

		va_end(va);

		return instr;
	}
};

static CInstructionMetaInfo InstructionMetaInfo[] = {
	#define INSTRUCTION(name, operand_count, ...) CInstructionMetaInfo::Create(#name, operand_count, __VA_ARGS__),
	#include "XScript/VirtualMachine/InstructionList.inc"
	#undef INSTRUCTION
};

// =================================================================
//	Stores debug info for a script.
// =================================================================
struct CDebugInfo
{
public:
	int Instruction_Offset;
	int File_Index;
	int Row;
	int Column;

	static CDebugInfo Create(int offset, int file, int row, int column)
	{
		CDebugInfo i;
		i.Instruction_Offset = offset;
		i.File_Index = file;
		i.Row = row;
		i.Column = column;
		return i;
	}
};

// =================================================================
//	Struct stores information on an individual vm instruction.
// =================================================================
struct CInstruction
{
public:
	InstructionOpCode::Type OpCode;
	COperand				Operands[COperand::max_operands_per_instruction];

	int						ByteOffset;

	CInstruction()
	{
		memset(this, 0, sizeof(CInstruction));
	}

	String Disassemble(std::vector<CSymbol*> symbol_table);

	int Encoded_Size();
	void Encode(char*& buffer);
	void Decode(char*& buffer);

	INLINE static u8 Decode_OpCode(u8*& buffer)
	{
		u8 val = *reinterpret_cast<u8*>(buffer);
		buffer += sizeof(u8);
		return val;
	}

	INLINE static f32 Decode_Float(u8*& buffer)
	{
		f32 val = *reinterpret_cast<f32*>(buffer);
		buffer += sizeof(f32);
		return val;
	}

	INLINE static s32 Decode_Int(u8*& buffer)
	{
		s32 val = *reinterpret_cast<s32*>(buffer);
		buffer += sizeof(s32);
		return val;
	}

	INLINE static u8 Decode_Register(u8*& buffer)
	{
		u8 val = *reinterpret_cast<u8*>(buffer);
		buffer += sizeof(u8);
		return val;
	}

	INLINE static u16 Decode_Symbol(u8*& buffer)
	{
		u16 val = *reinterpret_cast<u16*>(buffer);
		buffer += sizeof(u16);
		return val;
	}

	INLINE static u16 Decode_JumpTarget(u8*& buffer)
	{
		u16 val = *reinterpret_cast<u16*>(buffer);
		buffer += sizeof(u16);
		return val;
	}

	INLINE static u16 Decode_String(u8*& buffer)
	{
		u16 val = *reinterpret_cast<u16*>(buffer);
		buffer += sizeof(u16);
		return val;
	}

	static COperand Create_Operand(int op)
	{
		COperand operand;
		operand.int_value = op;
		return operand;
	}
	static COperand Create_Operand(float op)
	{
		COperand operand;
		operand.float_value = op;
		return operand;
	}

	static CInstruction Create(InstructionOpCode::Type opcode)
	{
		CInstruction i;
		i.OpCode		= opcode;
		return i;
	}

	template<typename T1>
	static CInstruction Create(InstructionOpCode::Type opcode, T1 op1)
	{
		CInstruction i;
		i.OpCode		= opcode;
		i.Operands[0]	= Create_Operand(op1); 
		return i;
	}
	template<typename T1, typename T2>
	static CInstruction Create(InstructionOpCode::Type opcode, T1 op1, T2 op2)
	{
		CInstruction i;
		i.OpCode		= opcode;
		i.Operands[0]	= Create_Operand(op1); 
		i.Operands[1]	= Create_Operand(op2); 
		return i;
	}
	template<typename T1, typename T2, typename T3>
	static CInstruction Create(InstructionOpCode::Type opcode, T1 op1, T2 op2, T3 op3)
	{
		CInstruction i;
		i.OpCode		= opcode;
		i.Operands[0]	= Create_Operand(op1); 
		i.Operands[1]	= Create_Operand(op2); 
		i.Operands[2]	= Create_Operand(op3); 
		return i;
	}
};

#endif