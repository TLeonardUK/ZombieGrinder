/* *****************************************************************

		CInstruction.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CInstruction.h"
#include "XScript/VirtualMachine/CSymbol.h"

int CInstruction::Encoded_Size()
{
	CInstructionMetaInfo& info = InstructionMetaInfo[this->OpCode];
	int size = sizeof(u8);
	for (int i = 0; i < info.Operand_Count; i++)
	{
		switch (info.Operand_Types[i])
		{
		case OperandType::Float:		size += sizeof(f32);	break;
		case OperandType::Int:			size += sizeof(s32);	break;
		case OperandType::Register:		size += sizeof(u8);		break;
		case OperandType::Symbol:		size += sizeof(u16);	break;
		case OperandType::JumpTarget:	size += sizeof(u16);	break;
		case OperandType::String:		size += sizeof(u16);	break;
		}
	}
	return size;
}

void CInstruction::Encode(char*& buffer)
{	
	CInstructionMetaInfo& info = InstructionMetaInfo[this->OpCode];

	*buffer = (u8)this->OpCode;
	buffer += sizeof(u8);

	for (int i = 0; i < info.Operand_Count; i++)
	{
		switch (info.Operand_Types[i])
		{
		case OperandType::Float:	
			{
				*reinterpret_cast<float*>(buffer) = this->Operands[i].float_value;
				buffer += sizeof(f32);
				break;
			}
		case OperandType::Int:			
			{
				*reinterpret_cast<int*>(buffer) = this->Operands[i].int_value;
				buffer += sizeof(s32);
				break;
			}
		case OperandType::Register:		
			{
				DBG_ASSERT(this->Operands[i].int_value >= 0 && this->Operands[i].int_value < 255);
				*reinterpret_cast<u8*>(buffer) = (u8)this->Operands[i].int_value;
				buffer += sizeof(u8);
				break;
			}
		case OperandType::Symbol:		
		case OperandType::JumpTarget:	
		case OperandType::String:		
			{
				DBG_ASSERT(this->Operands[i].int_value >= 0 && this->Operands[i].int_value < 65536);
				*reinterpret_cast<u16*>(buffer) = (u16)this->Operands[i].int_value;
				buffer += sizeof(u16);
				break;
			}
		}
	}
}

void CInstruction::Decode(char*& buffer)
{
	this->OpCode = (InstructionOpCode::Type)(*reinterpret_cast<u8*>(buffer));
	buffer += sizeof(u8);

	CInstructionMetaInfo& info = InstructionMetaInfo[this->OpCode];

	for (int i = 0; i < info.Operand_Count; i++)
	{
		switch (info.Operand_Types[i])
		{
		case OperandType::Float:	
			{
				this->Operands[i].float_value = *reinterpret_cast<float*>(buffer);
				buffer += sizeof(f32);
				break;
			}
		case OperandType::Int:			
			{
				this->Operands[i].int_value = *reinterpret_cast<int*>(buffer);
				buffer += sizeof(s32);
				break;
			}
		case OperandType::Register:		
			{
				this->Operands[i].int_value = (int)*reinterpret_cast<u8*>(buffer);
				DBG_ASSERT(this->Operands[i].int_value < 255);
				buffer += sizeof(u8);
				break;
			}
		case OperandType::Symbol:		
		case OperandType::JumpTarget:	
		case OperandType::String:		
			{
				this->Operands[i].int_value = (int)*reinterpret_cast<u16*>(buffer);
				DBG_ASSERT(this->Operands[i].int_value < 65536);
				buffer += sizeof(u16);
				break;
			}
		}
	}
}

String CInstruction::Disassemble(std::vector<CSymbol*> symbol_table)
{
	CInstructionMetaInfo info = InstructionMetaInfo[(int)OpCode];

	String result = info.Name;

	if (info.Operand_Count > 0)
		result += " ";

	for (int i = 0; i < info.Operand_Count; i++)
	{
		if (i > 0)
			result += ", ";

		switch (info.Operand_Types[i])
		{
		case OperandType::Int:			result += StringHelper::To_String(Operands[i].int_value);				break;
		case OperandType::Float:		result += StringHelper::To_String(Operands[i].float_value);				break;
		case OperandType::Register:		result += StringHelper::Format("r%i", Operands[i].int_value);			break;
		case OperandType::JumpTarget:	result += StringHelper::Format("label:%i", Operands[i].int_value);		break;
		case OperandType::String:		result += StringHelper::Format("string[%i]", Operands[i].int_value);	break;
		case OperandType::Symbol:		
			{
				CSymbol* symbol = symbol_table.at(Operands[i].int_value);
				result += StringHelper::Format("sym[%i] (%s)", Operands[i].int_value, symbol->Identifier.c_str());	
				break;
			}
		}
	}

	return result;
}
