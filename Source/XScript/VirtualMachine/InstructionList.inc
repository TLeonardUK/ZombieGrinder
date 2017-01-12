// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef INSTRUCTION
#error "INSTRUCTION must be defined when including this file!"
#endif

#define OP_INT			OperandType::Int
#define OP_FLOAT		OperandType::Float
#define OP_REGISTER		OperandType::Register
#define OP_SYMBOL		OperandType::Symbol
#define OP_JUMP_TARGET  OperandType::JumpTarget
#define OP_STRING		OperandType::String

//			name			operand-count	operand-types

// Stack manipulation.
INSTRUCTION(iPush,			1,				OP_REGISTER)							/* iPush		input_register */
INSTRUCTION(sPush,			1,				OP_REGISTER)							/* sPush		input_register */
INSTRUCTION(fPush,			1,				OP_REGISTER)							/* fPush		input_register */
INSTRUCTION(oPush,			1,				OP_REGISTER)							/* oPush		input_register */
INSTRUCTION(iPop,			1,				OP_REGISTER)							/* iPop			input_register */
INSTRUCTION(sPop,			1,				OP_REGISTER)							/* sPop			input_register */
INSTRUCTION(fPop,			1,				OP_REGISTER)							/* fPop			input_register */
INSTRUCTION(oPop,			1,				OP_REGISTER)							/* oPop			input_register */

// Moving
INSTRUCTION(iMov,			2,				OP_REGISTER,OP_REGISTER)					/* iMov			output_register, input_register */
INSTRUCTION(sMov,			2,				OP_REGISTER,OP_REGISTER)					/* sMov			output_register, input_register */
INSTRUCTION(fMov,			2,				OP_REGISTER,OP_REGISTER)					/* fMov			output_register, input_register */
INSTRUCTION(oMov,			2,				OP_REGISTER,OP_REGISTER)					/* oMov			output_register, input_register */

// Loading
INSTRUCTION(iLoadStatic,	3,				OP_REGISTER,OP_SYMBOL,OP_INT)			/* iLoadMember	output_register, object_register, field_index */
INSTRUCTION(sLoadStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)			/* sLoadMember	output_register, object_register, field_index */
INSTRUCTION(fLoadStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)			/* fLoadMember	output_register, object_register, field_index */
INSTRUCTION(oLoadStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)			/* oLoadMember	output_register, object_register, field_index */
INSTRUCTION(iLoadField,		3,				OP_REGISTER,OP_REGISTER,OP_INT)			/* iLoadMember	output_register, object_register, field_index */
INSTRUCTION(sLoadField,		3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* sLoadMember	output_register, object_register, field_index */
INSTRUCTION(fLoadField,		3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* fLoadMember	output_register, object_register, field_index */
INSTRUCTION(oLoadField,		3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* oLoadMember	output_register, object_register, field_index */
INSTRUCTION(iLoadLocal,		2, 				OP_REGISTER,OP_INT)						/* iLoadLocal	output_register, local_index */
INSTRUCTION(sLoadLocal,		2, 				OP_REGISTER,OP_INT)						/* sLoadLocal	output_register, local_index */
INSTRUCTION(fLoadLocal,		2, 				OP_REGISTER,OP_INT)						/* fLoadLocal	output_register, local_index */
INSTRUCTION(oLoadLocal,		2, 				OP_REGISTER,OP_INT)						/* oLoadLocal	output_register, local_index */
INSTRUCTION(iLoadLiteral,	2, 				OP_REGISTER,OP_INT)						/* iLoadLiteral	output_register, value */
INSTRUCTION(sLoadLiteral,	2, 				OP_REGISTER,OP_STRING)					/* sLoadLiteral	output_register, string_table_index */
INSTRUCTION(fLoadLiteral,	2, 				OP_REGISTER,OP_FLOAT)					/* fLoadLiteral	output_register, value */
INSTRUCTION(oLoadNull,		1, 				OP_REGISTER)							/* oLoadNull	output_register */
INSTRUCTION(oLoadThis,		1, 				OP_REGISTER)							/* oLoadThis	output_register */
INSTRUCTION(oLoadType,		2, 				OP_REGISTER,OP_SYMBOL)					/* oLoadType	output_register */

INSTRUCTION(iPushLiteral,	1, 				OP_INT)								/* iPushLiteral	output_register, value */
INSTRUCTION(sPushLiteral,	1, 				OP_STRING)							/* sPushLiteral	output_register, string_table_index */
INSTRUCTION(fPushLiteral,	1, 				OP_FLOAT)							/* fPushLiteral	output_register, value */
INSTRUCTION(oPushNull,		1,				OP_INT)								/* oPushNull	<arg-only-to-prevent-0-args> */

// Storing
INSTRUCTION(iStoreStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)						/* iStoreField	input_register, object_register, field_index */
INSTRUCTION(sStoreStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)						/* sStoreField	input_register, object_register, field_index */
INSTRUCTION(fStoreStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)						/* fStoreField	input_register, object_register, field_index */
INSTRUCTION(oStoreStatic,	3, 				OP_REGISTER,OP_SYMBOL,OP_INT)						/* oStoreField	input_register, object_register, field_index */
INSTRUCTION(iStoreField,	3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* iStoreField	input_register, object_register, field_index */
INSTRUCTION(sStoreField,	3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* sStoreField	input_register, object_register, field_index */
INSTRUCTION(fStoreField,	3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* fStoreField	input_register, object_register, field_index */
INSTRUCTION(oStoreField,	3, 				OP_REGISTER,OP_REGISTER,OP_INT)			/* oStoreField	input_register, object_register, field_index */
INSTRUCTION(iStoreLocal,	2, 				OP_REGISTER,OP_INT)						/* iStoreLocal	input_register, local_index */
INSTRUCTION(sStoreLocal,	2, 				OP_REGISTER,OP_INT)						/* sStoreLocal	input_register, local_index */
INSTRUCTION(fStoreLocal,	2, 				OP_REGISTER,OP_INT)						/* fStoreLocal	input_register, local_index */
INSTRUCTION(oStoreLocal,	2, 				OP_REGISTER,OP_INT)						/* oStoreLocal	input_register, local_index */

// Casting
INSTRUCTION(ifCast,			1, 				OP_REGISTER)							/* ifCast		input_output_register */
INSTRUCTION(isCast,			1, 				OP_REGISTER)							/* isCast		input_output_register */
INSTRUCTION(ibCast,			1, 				OP_REGISTER)							/* ibCast		input_output_register */
INSTRUCTION(fiCast,			1, 				OP_REGISTER)							/* fiCast		input_output_register */
INSTRUCTION(fsCast,			1, 				OP_REGISTER)							/* fsCast		input_output_register */
INSTRUCTION(siCast,			1, 				OP_REGISTER)							/* siCast		input_output_register */
INSTRUCTION(sbCast,			1, 				OP_REGISTER)							/* sbCast		input_output_register */
INSTRUCTION(sfCast,			1, 				OP_REGISTER)							/* siCast		input_output_register */
INSTRUCTION(obCast,			1, 				OP_REGISTER)							/* obCast		input_output_register */
INSTRUCTION(ofCast,			1, 				OP_REGISTER)							/* obCast		input_output_register */
INSTRUCTION(ooCast,			2, 				OP_REGISTER,OP_SYMBOL)					/* ooCast		input_register, class_symbol_index */
INSTRUCTION(ooSafeCast,		2, 				OP_REGISTER,OP_SYMBOL)					/* ooSafeCast	input_register, class_symbol_index		Performs a cast like ooCast except returns null on failure, rather than raising an exception. */

// Arithmatic
INSTRUCTION(iAdd,			2, 				OP_REGISTER,OP_REGISTER)					/* iAdd		input_output_register, add_register */
INSTRUCTION(iSub,			2, 				OP_REGISTER,OP_REGISTER)					/* iSub		input_output_register, add_register */
INSTRUCTION(iMul,			2, 				OP_REGISTER,OP_REGISTER)					/* iMul		input_output_register, add_register */
INSTRUCTION(iDiv,			2, 				OP_REGISTER,OP_REGISTER)					/* iDiv		input_output_register, add_register */
INSTRUCTION(iAnd,			2, 				OP_REGISTER,OP_REGISTER)					/* iAnd		input_output_register, add_register */
INSTRUCTION(iOr,			2, 				OP_REGISTER,OP_REGISTER)					/* iOr		input_output_register, add_register */
INSTRUCTION(iXOr,			2, 				OP_REGISTER,OP_REGISTER)					/* iXOr		input_output_register, add_register */
INSTRUCTION(iSHL,			2, 				OP_REGISTER,OP_REGISTER)					/* iSHL		input_output_register, add_register */
INSTRUCTION(iSHR,			2, 				OP_REGISTER,OP_REGISTER)					/* iSHR		input_output_register, add_register */
INSTRUCTION(iMod,			2, 				OP_REGISTER,OP_REGISTER)					/* iMod		input_output_register, add_register */
INSTRUCTION(fAdd,			2, 				OP_REGISTER,OP_REGISTER)					/* fAdd		input_output_register, add_register */
INSTRUCTION(fSub,			2, 				OP_REGISTER,OP_REGISTER)					/* fSub		input_output_register, add_register */
INSTRUCTION(fMul,			2, 				OP_REGISTER,OP_REGISTER)					/* fMul		input_output_register, add_register */
INSTRUCTION(fDiv,			2, 				OP_REGISTER,OP_REGISTER)					/* fDiv		input_output_register, add_register */
INSTRUCTION(sAdd,			2, 				OP_REGISTER,OP_REGISTER)					/* sDiv		input_output_register, add_register */

// Unary
INSTRUCTION(iNot,			1, 				OP_REGISTER)								/* iNot		input_output_register */
INSTRUCTION(iInc,			1, 				OP_REGISTER)								/* iInc		input_output_register */
INSTRUCTION(iDec,			1, 				OP_REGISTER)								/* iDec		input_output_register */
INSTRUCTION(iLogicalNot,	1, 				OP_REGISTER)								/* iLogicalNot	lvalue_output_register */
INSTRUCTION(fPos,			1, 				OP_REGISTER)								/* fPos		input_output_register */
INSTRUCTION(iPos,			1, 				OP_REGISTER)								/* iPos		input_output_register */
INSTRUCTION(fNeg,			1, 				OP_REGISTER)								/* fNeg		input_output_register */
INSTRUCTION(iNeg,			1, 				OP_REGISTER)								/* iNeg		input_output_register */
	
// Comparisons
INSTRUCTION(iCmpEQ,			2, 				OP_REGISTER,OP_REGISTER)					/* iCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION(fCmpEQ,			2, 				OP_REGISTER,OP_REGISTER)					/* fCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION(sCmpEQ,			2, 				OP_REGISTER,OP_REGISTER)					/* sCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION(oCmpEQ,			2, 				OP_REGISTER,OP_REGISTER)					/* oCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION(iCmpNE,			2, 				OP_REGISTER,OP_REGISTER)					/* iCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION(fCmpNE,			2, 				OP_REGISTER,OP_REGISTER)					/* fCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION(sCmpNE,			2, 				OP_REGISTER,OP_REGISTER)					/* sCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION(oCmpNE,			2, 				OP_REGISTER,OP_REGISTER)					/* oCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION(iCmpG,			2, 				OP_REGISTER,OP_REGISTER)					/* iCmpG	lvalue_output_register, rvalue_register */
INSTRUCTION(fCmpG,			2, 				OP_REGISTER,OP_REGISTER)					/* fCmpG	lvalue_output_register, rvalue_register */
INSTRUCTION(iCmpGE,			2, 				OP_REGISTER,OP_REGISTER)					/* iCmpGE	lvalue_output_register, rvalue_register */
INSTRUCTION(fCmpGE,			2, 				OP_REGISTER,OP_REGISTER)					/* fCmpGE	lvalue_output_register, rvalue_register */
INSTRUCTION(iCmpL,			2, 				OP_REGISTER,OP_REGISTER)					/* iCmpL	lvalue_output_register, rvalue_register */
INSTRUCTION(fCmpL,			2, 				OP_REGISTER,OP_REGISTER)					/* fCmpL	lvalue_output_register, rvalue_register */
INSTRUCTION(iCmpLE,			2, 				OP_REGISTER,OP_REGISTER)					/* iCmpLE	lvalue_output_register, rvalue_register */
INSTRUCTION(fCmpLE,			2, 				OP_REGISTER,OP_REGISTER)					/* fCmpLE	lvalue_output_register, rvalue_register */
INSTRUCTION(oCmpNull,		1, 				OP_REGISTER)								/* oCmpNull	lvalue_output_register */

// Branching
INSTRUCTION(Jmp,					1, 				OP_JUMP_TARGET)								/* Jmp			jump_target_table_index */
INSTRUCTION(JmpOnTrue,				2, 				OP_REGISTER,OP_JUMP_TARGET)					/* JmpOnTrue	compare_register, jump_target_table_index */
INSTRUCTION(JmpOnFalse,				2, 				OP_REGISTER,OP_JUMP_TARGET)					/* JmpOnFalse	compare_register, jump_target_table_index */
INSTRUCTION(vRet,					1,				OP_INT)										/* Ret <arg-only-to-prevent-0-args> */
INSTRUCTION(iRet,					1,				OP_REGISTER)								/* Ret result_register */
INSTRUCTION(fRet,					1,				OP_REGISTER)								/* Ret result_register */
INSTRUCTION(sRet,					1,				OP_REGISTER)								/* Ret result_register */
INSTRUCTION(oRet,					1,				OP_REGISTER)								/* Ret result_register */
INSTRUCTION(InvokeNonVirtualMethod,	3,				OP_REGISTER,OP_REGISTER,OP_SYMBOL)			/* InvokeMethodIgnoreVirtual result_register, object, symbol_index */ // Same as InvokeMethod except it dosen't go through the vftable if method is virtual.
INSTRUCTION(InvokeMethod,			3,				OP_REGISTER,OP_REGISTER,OP_SYMBOL)			/* InvokeMethod result_register, object, symbol_index */
INSTRUCTION(InvokeStatic,			2,				OP_REGISTER,OP_SYMBOL)						/* InvokeStatic result_register, symbol_index */

// Objects
INSTRUCTION(NewObject,		2,						OP_REGISTER,OP_SYMBOL)						/* NewObject result_register, class_symbol_index */
INSTRUCTION(NewArray,		3,						OP_REGISTER,OP_REGISTER,OP_SYMBOL)			/* NewArray  result_register, size_register, class_symbol_index */
INSTRUCTION(GetLength,		2,						OP_REGISTER,OP_REGISTER)					/* GetLength  result_register, array_register */
INSTRUCTION(GetIndex,		3,						OP_REGISTER,OP_REGISTER,OP_REGISTER)		/* GetIndex  result_register, array_register, index_register */

// Debug
//INSTRUCTION(DebugTrace,		3,						OP_STRING,OP_INT,OP_INT)			/* DebugTrace file_string_index, column, row  */

// State
INSTRUCTION(PushState,		1,						OP_SYMBOL)								/* PushState state_symbol_index  */
INSTRUCTION(ChangeState,	1,						OP_SYMBOL)								/* ChangeState state_symbol_index  */
INSTRUCTION(PopState,		1,						OP_INT)									/* PopState <arg-only-to-prevent-0-args> */

#undef OP_INT
#undef OP_FLOAT
#undef OP_REGISTER		
#undef OP_SYMBOL		
#undef OP_JUMP_TARGET 
#undef OP_STRING		