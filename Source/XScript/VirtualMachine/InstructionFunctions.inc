/* *****************************************************************

		InstructionFunctions.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

// Operand manipulation.
#define DECODE_INT()										CInstruction::Decode_Int(ip)	
#define DECODE_FLOAT()										CInstruction::Decode_Float(ip)	
#define DECODE_REGISTER()									CInstruction::Decode_Register(ip)	
#define DECODE_SYMBOL()										CInstruction::Decode_Symbol(ip)	
#define DECODE_JUMPTARGET()									CInstruction::Decode_JumpTarget(ip)	
#define DECODE_STRING()										CInstruction::Decode_String(ip)	

// Register manipulation.
#define SET_REGISTER_VALUE(reg, value)						context->Stack[context_register_offset + reg] = value
#define SET_REGISTER_INT(reg, value)						context->Stack[context_register_offset + reg].int_value = value
#define SET_REGISTER_FLOAT(reg, value)						context->Stack[context_register_offset + reg].float_value = value
#define SET_REGISTER_STRING(reg, value)						context->Stack[context_register_offset + reg].string_value = value
#define SET_REGISTER_OBJECT(reg, value)						context->Stack[context_register_offset + reg].object_value = value
#define GET_REGISTER_VALUE(reg)								context->Stack[context_register_offset + reg]
#define GET_REGISTER_INT(reg)								context->Stack[context_register_offset + reg].int_value
#define GET_REGISTER_FLOAT(reg)								context->Stack[context_register_offset + reg].float_value
#define GET_REGISTER_STRING(reg)							context->Stack[context_register_offset + reg].string_value
#define GET_REGISTER_OBJECT(reg)							context->Stack[context_register_offset + reg].object_value

// Local manipulation.
#define SET_LOCAL_VALUE(local_index, value)					context->Stack[context_local_offset + local_index] = value
#define SET_LOCAL_INT(local_index, value)					context->Stack[context_local_offset + local_index].int_value = value
#define SET_LOCAL_FLOAT(local_index, value)					context->Stack[context_local_offset + local_index].float_value = value
#define SET_LOCAL_STRING(local_index, value)				context->Stack[context_local_offset + local_index].string_value = value
#define SET_LOCAL_OBJECT(local_index, value)				context->Stack[context_local_offset + local_index].object_value = value
#define GET_LOCAL_VALUE(local_index)						context->Stack[context_local_offset + local_index]
#define GET_LOCAL_INT(local_index)							context->Stack[context_local_offset + local_index].int_value
#define GET_LOCAL_FLOAT(local_index)						context->Stack[context_local_offset + local_index].float_value
#define GET_LOCAL_STRING(local_index)						context->Stack[context_local_offset + local_index].string_value
#define GET_LOCAL_OBJECT(local_index)						context->Stack[context_local_offset + local_index].object_value

// Field manipulation
#define SET_FIELD_VALUE(handle, field_index, value)			handle.Get()->Get_Slot(field_index) = value
#define SET_FIELD_INT(handle, field_index, value)			handle.Get()->Get_Slot(field_index).int_value = value
#define SET_FIELD_FLOAT(handle, field_index, value)			handle.Get()->Get_Slot(field_index).float_value = value
#define SET_FIELD_STRING(handle, field_index, value)		handle.Get()->Get_Slot(field_index).string_value = value
#define SET_FIELD_OBJECT(handle, field_index, value)		handle.Get()->Get_Slot(field_index).object_value = value
#define GET_FIELD_VALUE(handle, field_index)				handle.Get()->Get_Slot(field_index)
#define GET_FIELD_INT(handle, field_index)					handle.Get()->Get_Slot(field_index).int_value
#define GET_FIELD_FLOAT(handle, field_index)				handle.Get()->Get_Slot(field_index).float_value
#define GET_FIELD_STRING(handle, field_index)				handle.Get()->Get_Slot(field_index).string_value
#define GET_FIELD_OBJECT(handle, field_index)				handle.Get()->Get_Slot(field_index).object_value

// These are the functions that contain code for individual VM opcodes.

/* *****************************************************************
   Stack manipulation.
   ***************************************************************** */

// TODO: Possible optimization for these, they are all the same atm.

/* iPush		input_register */
INSTRUCTION_FUNCTION(iPush)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");
	context->Stack.Push(GET_REGISTER_VALUE(output_reg));
	RETURN_RESULT_CONTINUE();
}

/* sPush		input_register */
INSTRUCTION_FUNCTION(sPush)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");
	context->Stack.Push(GET_REGISTER_VALUE(output_reg));
	RETURN_RESULT_CONTINUE();
}

/* fPush		input_register */
INSTRUCTION_FUNCTION(fPush)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");
	context->Stack.Push(GET_REGISTER_VALUE(output_reg));
	RETURN_RESULT_CONTINUE();
}

/* oPush		input_register */
INSTRUCTION_FUNCTION(oPush)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");
	context->Stack.Push(GET_REGISTER_VALUE(output_reg));
	RETURN_RESULT_CONTINUE();
}

/* iPop			input_register */
INSTRUCTION_FUNCTION(iPop)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Size() > 0, "Stack underflow.");
	SET_REGISTER_VALUE(output_reg, context->Stack.Pop_Fast());
	RETURN_RESULT_CONTINUE();
}

/* sPop			input_register */
INSTRUCTION_FUNCTION(sPop)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Size() > 0, "Stack underflow.");
	SET_REGISTER_VALUE(output_reg, context->Stack.Pop_Fast());
	RETURN_RESULT_CONTINUE();
}

/* fPop			input_register */
INSTRUCTION_FUNCTION(fPop)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Size() > 0, "Stack underflow.");
	SET_REGISTER_VALUE(output_reg, context->Stack.Pop_Fast());
	RETURN_RESULT_CONTINUE();
}

/* oPop			input_register */
INSTRUCTION_FUNCTION(oPop)
{
	int output_reg = DECODE_REGISTER();
	vm->Assert(context->Stack.Size() > 0, "Stack underflow.");
	SET_REGISTER_VALUE(output_reg, context->Stack.Pop_Fast());
	context->Stack.Pop_Fast();
	RETURN_RESULT_CONTINUE();
}

/* *****************************************************************
   Moving
   ***************************************************************** */

/* iMov			output_register, input_register */
INSTRUCTION_FUNCTION(iMov)
{	
	int output_reg = DECODE_REGISTER();
	int input_reg = DECODE_REGISTER();
	
	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* sMov			output_register, input_register */
INSTRUCTION_FUNCTION(sMov)
{
	int output_reg = DECODE_REGISTER();
	int input_reg = DECODE_REGISTER();

	SET_REGISTER_STRING(output_reg, GET_REGISTER_STRING(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* fMov			output_register, input_register */
INSTRUCTION_FUNCTION(fMov)
{
	int output_reg = DECODE_REGISTER();
	int input_reg = DECODE_REGISTER();
	
	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_FLOAT(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* oMov			output_register, input_register */
INSTRUCTION_FUNCTION(oMov)
{
	int output_reg = DECODE_REGISTER();
	int input_reg = DECODE_REGISTER();
	
	SET_REGISTER_OBJECT(output_reg, GET_REGISTER_OBJECT(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* *****************************************************************
   Loading
   ***************************************************************** */

/* iLoadMember	output_register, class_symbol_index, field_index */
INSTRUCTION_FUNCTION(iLoadStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	SET_REGISTER_INT(output_reg, class_data.int_value);

	RETURN_RESULT_CONTINUE();
}

/* sLoadMember	output_register, class_symbol_index, field_index */
INSTRUCTION_FUNCTION(sLoadStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	SET_REGISTER_STRING(output_reg, class_data.string_value);

	RETURN_RESULT_CONTINUE();
}

/* fLoadMember	output_register, class_symbol_index, field_index */
INSTRUCTION_FUNCTION(fLoadStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	SET_REGISTER_FLOAT(output_reg, class_data.float_value);

	RETURN_RESULT_CONTINUE();
}

/* oLoadMember	output_register, class_symbol_index, field_index */
INSTRUCTION_FUNCTION(oLoadStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	SET_REGISTER_OBJECT(output_reg, class_data.object_value);

	RETURN_RESULT_CONTINUE();
}

/* iLoadMember	output_register, object_register, field_index */
INSTRUCTION_FUNCTION(iLoadField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_REGISTER_INT(output_reg, GET_FIELD_INT(object, field_index));

	RETURN_RESULT_CONTINUE();
}

/* sLoadMember	output_register, object_register, field_index */
INSTRUCTION_FUNCTION(sLoadField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_REGISTER_STRING(output_reg, GET_FIELD_STRING(object, field_index));

	RETURN_RESULT_CONTINUE();
}

/* fLoadMember	output_register, object_register, field_index */
INSTRUCTION_FUNCTION(fLoadField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_REGISTER_FLOAT(output_reg, GET_FIELD_FLOAT(object, field_index));

	RETURN_RESULT_CONTINUE();
}

/* oLoadMember	output_register, object_register, field_index */
INSTRUCTION_FUNCTION(oLoadField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_REGISTER_OBJECT(output_reg, GET_FIELD_OBJECT(object, field_index));

	RETURN_RESULT_CONTINUE();
}

/* iLoadLocal	output_register, local_index */
INSTRUCTION_FUNCTION(iLoadLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_REGISTER_INT(output_reg, GET_LOCAL_INT(local_index));
	
	RETURN_RESULT_CONTINUE();
}

/* sLoadLocal	output_register, local_index */
INSTRUCTION_FUNCTION(sLoadLocal)
{	
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_REGISTER_STRING(output_reg, GET_LOCAL_STRING(local_index));
	
	RETURN_RESULT_CONTINUE();
}

/* fLoadLocal	output_register, local_index */
INSTRUCTION_FUNCTION(fLoadLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_REGISTER_FLOAT(output_reg, GET_LOCAL_FLOAT(local_index));
	
	RETURN_RESULT_CONTINUE();
}

/* oLoadLocal	output_register, local_index */
INSTRUCTION_FUNCTION(oLoadLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_REGISTER_OBJECT(output_reg, GET_LOCAL_OBJECT(local_index));

	RETURN_RESULT_CONTINUE();
}

/* iLoadLiteral	output_register, value */
INSTRUCTION_FUNCTION(iLoadLiteral)
{
	int output_reg = DECODE_REGISTER();
	int value	   = DECODE_INT();

	SET_REGISTER_INT(output_reg, value);

	RETURN_RESULT_CONTINUE();
}

/* sLoadLiteral	output_register, string_table_index */
INSTRUCTION_FUNCTION(sLoadLiteral)
{
	int output_reg		= DECODE_REGISTER();
	int string_index	= DECODE_STRING();

	SET_REGISTER_STRING(output_reg, vm->Get_String_Table_Entry(string_index));

	RETURN_RESULT_CONTINUE();
}

/* fLoadLiteral	output_register, value */
INSTRUCTION_FUNCTION(fLoadLiteral)
{
	int output_reg = DECODE_REGISTER();
	float value	   = DECODE_FLOAT();

	SET_REGISTER_FLOAT(output_reg, value);

	RETURN_RESULT_CONTINUE();
}

/* oLoadNull	output_register */
INSTRUCTION_FUNCTION(oLoadNull)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_OBJECT(output_reg, CVMObjectHandle(NULL));
	
	RETURN_RESULT_CONTINUE();
}

/* oLoadType	output_register, symbol */
INSTRUCTION_FUNCTION(oLoadType)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();

	CVMObjectHandle type_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->type_instance;
	
	SET_REGISTER_OBJECT(output_reg, type_data);
	
	RETURN_RESULT_CONTINUE();
}

/* iPushLiteral	output_register, value */
INSTRUCTION_FUNCTION(iPushLiteral)
{
	int value = DECODE_INT();

	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");
	CVMValue val(value);
	context->Stack.Push(val);

	RETURN_RESULT_CONTINUE();
}

/* sPushLiteral	output_register, string_table_index */
INSTRUCTION_FUNCTION(sPushLiteral)
{
	int string_index = DECODE_STRING();

	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");
	CVMValue val(vm->Get_String_Table_Entry(string_index));
	context->Stack.Push(val);

	RETURN_RESULT_CONTINUE();
}

/* fPushLiteral	output_register, value */
INSTRUCTION_FUNCTION(fPushLiteral)
{
	float value = DECODE_FLOAT();

	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");

	CVMValue val(value);
	context->Stack.Push(val);

	RETURN_RESULT_CONTINUE();
}

/* oPushNull	output_register */
INSTRUCTION_FUNCTION(oPushNull)
{
	vm->Assert(context->Stack.Available() > 0, "Stack overflow.");

	CVMValue val(CVMObjectHandle(NULL));
	context->Stack.Push(val);

	RETURN_RESULT_CONTINUE();
}

/* oLoadThis	output_register */
INSTRUCTION_FUNCTION(oLoadThis)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_VALUE(output_reg, activation_context->This);
	
	RETURN_RESULT_CONTINUE();
}

/* *****************************************************************
   Sorting
   ***************************************************************** */

/* iStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(iStoreStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(class_symbol_index);
	CVMValue& class_data = symbol->static_data[field_index];

	class_data.int_value = GET_REGISTER_INT(output_reg);

	RETURN_RESULT_CONTINUE();
}

/* sStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(sStoreStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	class_data.string_value = GET_REGISTER_STRING(output_reg);
	RETURN_RESULT_CONTINUE();
}

/* fStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(fStoreStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	class_data.float_value = GET_REGISTER_FLOAT(output_reg);
	RETURN_RESULT_CONTINUE();
}

/* oStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(oStoreStatic)
{
	int output_reg = DECODE_REGISTER();
	int class_symbol_index = DECODE_SYMBOL();
	int field_index = DECODE_INT();

	CVMValue& class_data = vm->Get_Symbol_Table_Entry(class_symbol_index)->static_data[field_index];
	class_data.object_value = GET_REGISTER_OBJECT(output_reg);
	RETURN_RESULT_CONTINUE();
}

/* iStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(iStoreField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_FIELD_INT(object, field_index, GET_REGISTER_INT(output_reg));

	RETURN_RESULT_CONTINUE();
}

/* sStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(sStoreField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_FIELD_STRING(object, field_index, GET_REGISTER_STRING(output_reg));

	RETURN_RESULT_CONTINUE();
}

/* fStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(fStoreField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_FIELD_FLOAT(object, field_index, GET_REGISTER_FLOAT(output_reg));

	RETURN_RESULT_CONTINUE();
}

/* oStoreField	input_register, object_register, field_index */
INSTRUCTION_FUNCTION(oStoreField)
{
	int output_reg = DECODE_REGISTER();
	int object_reg = DECODE_REGISTER();
	int field_index = DECODE_INT();

	CVMObjectHandle object = GET_REGISTER_OBJECT(object_reg);
	vm->Assert(object.Get() != NULL, "Null access violation.");

	SET_FIELD_OBJECT(object, field_index, GET_REGISTER_OBJECT(output_reg));

	RETURN_RESULT_CONTINUE();
}

/* iStoreLocal	input_register, local_index */
INSTRUCTION_FUNCTION(iStoreLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_LOCAL_INT(local_index, GET_REGISTER_INT(output_reg));
	
	RETURN_RESULT_CONTINUE();
}

/* sStoreLocal	input_register, local_index */
INSTRUCTION_FUNCTION(sStoreLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_LOCAL_STRING(local_index, GET_REGISTER_STRING(output_reg));
	
	RETURN_RESULT_CONTINUE();
}
		
/* fStoreLocal	input_register, local_index */
INSTRUCTION_FUNCTION(fStoreLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_LOCAL_FLOAT(local_index, GET_REGISTER_FLOAT(output_reg));
	
	RETURN_RESULT_CONTINUE();
}

/* oStoreLocal	input_register, local_index */
INSTRUCTION_FUNCTION(oStoreLocal)
{
	int output_reg = DECODE_REGISTER();
	int local_index = DECODE_INT();

	SET_LOCAL_OBJECT(local_index, GET_REGISTER_OBJECT(output_reg));
	
	RETURN_RESULT_CONTINUE();
}					

/* *****************************************************************
   Casting
   ***************************************************************** */

/* ifCast		input_output_register */
INSTRUCTION_FUNCTION(ifCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_FLOAT(output_reg, (float)GET_REGISTER_INT(output_reg));

	RETURN_RESULT_CONTINUE();
}
			
/* isCast		input_output_register */
INSTRUCTION_FUNCTION(isCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_STRING(output_reg, GET_REGISTER_INT(output_reg));

	RETURN_RESULT_CONTINUE();
}
					
/* ibCast		input_output_register */
INSTRUCTION_FUNCTION(ibCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_INT(output_reg, !!GET_REGISTER_INT(output_reg));

	RETURN_RESULT_CONTINUE();
}
			
/* fiCast		input_output_register */
INSTRUCTION_FUNCTION(fiCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_INT(output_reg, (int)GET_REGISTER_FLOAT(output_reg));

	RETURN_RESULT_CONTINUE();
}

/* fsCast		input_output_register */
INSTRUCTION_FUNCTION(fsCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_STRING(output_reg, GET_REGISTER_FLOAT(output_reg));

	RETURN_RESULT_CONTINUE();
}

/* siCast		input_output_register */
INSTRUCTION_FUNCTION(siCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_INT(output_reg, GET_REGISTER_STRING(output_reg).To_Int());

	RETURN_RESULT_CONTINUE();
}
				
/* sbCast		input_output_register */
INSTRUCTION_FUNCTION(sbCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_INT(output_reg, GET_REGISTER_STRING(output_reg).To_Bool());

	RETURN_RESULT_CONTINUE();
}
				
/* siCast		input_output_register */
INSTRUCTION_FUNCTION(sfCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_STRING(output_reg).To_Float());

	RETURN_RESULT_CONTINUE();
}
	
/* obCast		input_output_register */
INSTRUCTION_FUNCTION(obCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_INT(output_reg, GET_REGISTER_OBJECT(output_reg).Get() != NULL);

	RETURN_RESULT_CONTINUE();
}
				
/* ofCast		input_output_register */
INSTRUCTION_FUNCTION(ofCast)
{
	int output_reg = DECODE_REGISTER();
	
	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_OBJECT(output_reg).Get() != NULL ? 1.0f : 0.0f);

	RETURN_RESULT_CONTINUE();
}
					
/* ooCast		input_register, class_symbol_index */
INSTRUCTION_FUNCTION(ooCast)
{
	int object_reg = DECODE_REGISTER();
	int symbol_index = DECODE_SYMBOL();

	CVMObjectHandle obj = GET_REGISTER_OBJECT(object_reg);
	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(symbol_index);
	CVMObjectHandle new_obj = vm->Cast_Object(obj.Get(), symbol);

	// TODO: Blah, just do safecast's.
	//	vm->Assert(new_obj.Get() != NULL);
	
	SET_REGISTER_OBJECT(object_reg, new_obj);
	RETURN_RESULT_CONTINUE();
}
			
/* ooSafeCast	input_register, class_symbol_index		Performs a cast like ooCast except returns null on failure, rather than raising an exception. */
INSTRUCTION_FUNCTION(ooSafeCast)
{
	int object_reg = DECODE_REGISTER();
	int symbol_index = DECODE_SYMBOL();

	CVMObjectHandle obj = GET_REGISTER_OBJECT(object_reg);
	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(symbol_index);
	CVMObjectHandle new_obj = vm->Cast_Object(obj.Get(), symbol);

//	DBG_LOG("Safe Cast %i (%s->%s)", GET_OP_INT(0), obj.Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

//	std::string a_type = obj.Get()->Get_Symbol()->symbol->name;
//	std::string b_type = symbol->symbol->name;

//	if (a_type == "Type" && b_type == "IEnumerable")
//	{
//		DBG_LOG("FGOOOOOOOO!");
//	}

	SET_REGISTER_OBJECT(object_reg, new_obj);
	RETURN_RESULT_CONTINUE();
}

/* *****************************************************************
   Arithmatic
   ***************************************************************** */

/* iAdd		input_output_register, add_register */
INSTRUCTION_FUNCTION(iAdd)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) + GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
	
/* iSub		input_output_register, add_register */
INSTRUCTION_FUNCTION(iSub)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) - GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
		
/* iMul		input_output_register, add_register */
INSTRUCTION_FUNCTION(iMul)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) * GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* iDiv		input_output_register, add_register */
INSTRUCTION_FUNCTION(iDiv)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) / GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
			
/* iAnd		input_output_register, add_register */
INSTRUCTION_FUNCTION(iAnd)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) & GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
					
/* iOr		input_output_register, add_register */
INSTRUCTION_FUNCTION(iOr)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) | GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
				
/* iXOr		input_output_register, add_register */
INSTRUCTION_FUNCTION(iXOr)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) ^ GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* iSHL		input_output_register, add_register */
INSTRUCTION_FUNCTION(iSHL)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) << GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
				
/* iSHR		input_output_register, add_register */
INSTRUCTION_FUNCTION(iSHR)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) >> GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}
				
/* iMod		input_output_register, add_register */
INSTRUCTION_FUNCTION(iMod)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) % GET_REGISTER_INT(input_reg));

	RETURN_RESULT_CONTINUE();
}

/* fAdd		input_output_register, add_register */
INSTRUCTION_FUNCTION(fAdd)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_FLOAT(output_reg) + GET_REGISTER_FLOAT(input_reg));

	RETURN_RESULT_CONTINUE();
}
				
/* fSub		input_output_register, add_register */
INSTRUCTION_FUNCTION(fSub)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_FLOAT(output_reg) - GET_REGISTER_FLOAT(input_reg));

	RETURN_RESULT_CONTINUE();
}
			
/* fMul		input_output_register, add_register */
INSTRUCTION_FUNCTION(fMul)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_FLOAT(output_reg) * GET_REGISTER_FLOAT(input_reg));

	RETURN_RESULT_CONTINUE();
}
			
/* fDiv		input_output_register, add_register */
INSTRUCTION_FUNCTION(fDiv)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_FLOAT(output_reg, GET_REGISTER_FLOAT(output_reg) / GET_REGISTER_FLOAT(input_reg));

	RETURN_RESULT_CONTINUE();
}
			
/* sDiv		input_output_register */
INSTRUCTION_FUNCTION(sAdd)
{
	int output_reg = DECODE_REGISTER();
	int input_reg  = DECODE_REGISTER();

	SET_REGISTER_STRING(output_reg, GET_REGISTER_STRING(output_reg) + GET_REGISTER_STRING(input_reg));

	RETURN_RESULT_CONTINUE();
}
		
/* fPos		input_output_register */
INSTRUCTION_FUNCTION(fPos)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_FLOAT(output_reg, fabs(GET_REGISTER_FLOAT(output_reg)));
	RETURN_RESULT_CONTINUE();
}

/* iPos		input_output_register */
INSTRUCTION_FUNCTION(iPos)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_INT(output_reg, abs(GET_REGISTER_INT(output_reg)));
	RETURN_RESULT_CONTINUE();
}
			
/* fNeg		input_output_register */
INSTRUCTION_FUNCTION(fNeg)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_FLOAT(output_reg, -GET_REGISTER_FLOAT(output_reg));
	RETURN_RESULT_CONTINUE();
}

/* iNeg		input_output_register */
INSTRUCTION_FUNCTION(iNeg)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_INT(output_reg, -GET_REGISTER_INT(output_reg));
	RETURN_RESULT_CONTINUE();
}

/* iNot		input_output_register */
INSTRUCTION_FUNCTION(iNot)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_INT(output_reg, ~GET_REGISTER_INT(output_reg));
	RETURN_RESULT_CONTINUE();
}
			
/* iInc		input_output_register */
INSTRUCTION_FUNCTION(iInc)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) + 1);
	RETURN_RESULT_CONTINUE();
}
		
/* iDec		input_output_register */
INSTRUCTION_FUNCTION(iDec)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_INT(output_reg, GET_REGISTER_INT(output_reg) - 1);
	RETURN_RESULT_CONTINUE();
}
				
/* iLogicalNot	lvalue_output_register */
INSTRUCTION_FUNCTION(iLogicalNot)
{
	int output_reg = DECODE_REGISTER();
	SET_REGISTER_INT(output_reg, !GET_REGISTER_INT(output_reg));
	RETURN_RESULT_CONTINUE();
}	
	
/* *****************************************************************
   Comparisons
   ***************************************************************** */

/* iCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(iCmpEQ)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_INT(lvalue_reg) == GET_REGISTER_INT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
				
/* fCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(fCmpEQ)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, fabs(GET_REGISTER_FLOAT(lvalue_reg) - GET_REGISTER_FLOAT(rvalue_reg)) < FLT_EPSILON);
	RETURN_RESULT_CONTINUE();
}
			
/* sCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(sCmpEQ)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_STRING(lvalue_reg) == GET_REGISTER_STRING(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
			
/* oCmpEQ	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(oCmpEQ)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_OBJECT(lvalue_reg).Get() == GET_REGISTER_OBJECT(rvalue_reg).Get());
	RETURN_RESULT_CONTINUE();
}
				
/* iCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(iCmpNE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_INT(lvalue_reg) != GET_REGISTER_INT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
					
/* fCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(fCmpNE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, fabs(GET_REGISTER_FLOAT(lvalue_reg) - GET_REGISTER_FLOAT(rvalue_reg)) >= FLT_EPSILON);
	RETURN_RESULT_CONTINUE();
}
				
/* sCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(sCmpNE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_STRING(lvalue_reg) != GET_REGISTER_STRING(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
					
/* oCmpNE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(oCmpNE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_OBJECT(lvalue_reg).Get() != GET_REGISTER_OBJECT(rvalue_reg).Get());
	RETURN_RESULT_CONTINUE();
}
	
/* iCmpG	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(iCmpG)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_INT(lvalue_reg) > GET_REGISTER_INT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
				
/* fCmpG	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(fCmpG)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_FLOAT(lvalue_reg) > GET_REGISTER_FLOAT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
			
/* iCmpGE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(iCmpGE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_INT(lvalue_reg) >= GET_REGISTER_INT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
				
/* fCmpGE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(fCmpGE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_FLOAT(lvalue_reg) >= GET_REGISTER_FLOAT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
					
/* iCmpL	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(iCmpL)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_INT(lvalue_reg) < GET_REGISTER_INT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
			
/* fCmpL	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(fCmpL)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_FLOAT(lvalue_reg) < GET_REGISTER_FLOAT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
			
/* iCmpLE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(iCmpLE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_INT(lvalue_reg) <= GET_REGISTER_INT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}
		
/* fCmpLE	lvalue_output_register, rvalue_register */
INSTRUCTION_FUNCTION(fCmpLE)
{
	int lvalue_reg = DECODE_REGISTER();
	int rvalue_reg = DECODE_REGISTER();
	SET_REGISTER_INT(lvalue_reg, GET_REGISTER_FLOAT(lvalue_reg) <= GET_REGISTER_FLOAT(rvalue_reg));
	RETURN_RESULT_CONTINUE();
}		

/* oCmpNull	lvalue_output_register */
INSTRUCTION_FUNCTION(oCmpNull)
{
	int lvalue_reg = DECODE_REGISTER();
	CVMObjectHandle obj = GET_REGISTER_OBJECT(lvalue_reg);
	SET_REGISTER_INT(lvalue_reg, obj.Get() == NULL);
	RETURN_RESULT_CONTINUE();
}

/* *****************************************************************
   Branching
   ***************************************************************** */

/* Jmp			jump_target_table_index */
INSTRUCTION_FUNCTION(Jmp)
{
	ip = ip_base + DECODE_JUMPTARGET();
	RETURN_RESULT_JUMPED();
}
						
/* JmpOnTrue	compare_register, jump_target_table_index */
INSTRUCTION_FUNCTION(JmpOnTrue)
{
	int compare_register = DECODE_REGISTER();
	int dest_instruction = DECODE_JUMPTARGET();
	if (GET_REGISTER_INT(compare_register) != 0)
	{
		ip = ip_base + dest_instruction;
		RETURN_RESULT_JUMPED();
	}
	RETURN_RESULT_CONTINUE();
}
			
/* JmpOnFalse	compare_register, jump_target_table_index */
INSTRUCTION_FUNCTION(JmpOnFalse)
{
	int compare_register = DECODE_REGISTER();
	int dest_instruction = DECODE_JUMPTARGET();
	if (GET_REGISTER_INT(compare_register) == 0)
	{
		ip = ip_base + dest_instruction;
		RETURN_RESULT_JUMPED();
	}
	RETURN_RESULT_CONTINUE();
}
			
/* Ret */
INSTRUCTION_FUNCTION(vRet)
{
	CVMValue retval;
	if (vm->Return_Internal(retval))
	{
		RETURN_RESULT_FINISHED();
	}
	else
	{
		RETURN_RESULT_CONTEXTCHANGED();
	}
}
							
/* Ret result_register */
INSTRUCTION_FUNCTION(iRet)
{
	int input_register = DECODE_REGISTER();

	if (vm->Return_Internal(GET_REGISTER_VALUE(input_register)))
	{
		RETURN_RESULT_FINISHED();
	}
	else
	{
		RETURN_RESULT_CONTEXTCHANGED();
	}
}
							
/* Ret result_register */
INSTRUCTION_FUNCTION(fRet)
{
	int input_register = DECODE_REGISTER();

	if (vm->Return_Internal(GET_REGISTER_VALUE(input_register)))
	{
		RETURN_RESULT_FINISHED();
	}
	else
	{
		RETURN_RESULT_CONTEXTCHANGED();
	}
}
					
/* Ret result_register */
INSTRUCTION_FUNCTION(sRet)
{
	int input_register = DECODE_REGISTER();

	if (vm->Return_Internal(GET_REGISTER_VALUE(input_register)))
	{
		RETURN_RESULT_FINISHED();
	}
	else
	{
		RETURN_RESULT_CONTEXTCHANGED();
	}
}
			
/* Ret result_register */
INSTRUCTION_FUNCTION(oRet)
{
	int input_register = DECODE_REGISTER();

	if (vm->Return_Internal(GET_REGISTER_VALUE(input_register)))
	{
		RETURN_RESULT_FINISHED();
	}
	else
	{
		RETURN_RESULT_CONTEXTCHANGED();
	}
}
	
/* InvokeMethodIgnoreVirtual result_register, object, symbol_index */ // Same as InvokeMethod except it dosen't go through the vftable if method is virtual.
INSTRUCTION_FUNCTION(InvokeNonVirtualMethod)
{
	int result_reg = DECODE_REGISTER();
	int obj_reg = DECODE_REGISTER();
	int func_symbol = DECODE_SYMBOL();

	CVMValue handle = GET_REGISTER_VALUE(obj_reg);
	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(func_symbol);

	vm->Invoke_Internal(symbol, handle, true, true, result_reg);
	RETURN_RESULT_CONTEXTCHANGED();
}

/* InvokeMethod result_register, object, symbol_index */
INSTRUCTION_FUNCTION(InvokeMethod)
{
	int result_reg = DECODE_REGISTER();
	int obj_reg = DECODE_REGISTER();
	int func_symbol = DECODE_SYMBOL();

	CVMValue handle = GET_REGISTER_VALUE(obj_reg);
	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(func_symbol);
	
	vm->Invoke_Internal(symbol, handle, true, false, result_reg);
	RETURN_RESULT_CONTEXTCHANGED();
}

/* InvokeStatic result_register, symbol_index */
INSTRUCTION_FUNCTION(InvokeStatic)
{
	int result_reg = DECODE_REGISTER();
	int func_symbol = DECODE_SYMBOL();

	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(func_symbol);
	CVMValue obj_instance;

	vm->Invoke_Internal(symbol, obj_instance, true, false, result_reg);
	RETURN_RESULT_CONTEXTCHANGED();
}			

/* *****************************************************************
   Objects
   ***************************************************************** */

/* NewObject result_register, class_symbol_index */
INSTRUCTION_FUNCTION(NewObject)
{
	int result_reg = DECODE_REGISTER();
	int obj_symbol = DECODE_SYMBOL();

	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(obj_symbol);
	SET_REGISTER_OBJECT(result_reg, vm->New_Object(symbol, false));
	RETURN_RESULT_CONTINUE();
}
				
/* NewArray  result_register, size_register, class_symbol_index */
INSTRUCTION_FUNCTION(NewArray)
{
	int result_reg = DECODE_REGISTER();
	int size_reg = DECODE_REGISTER();
	int obj_symbol = DECODE_SYMBOL();

	int size = GET_REGISTER_INT(size_reg);

	CVMLinkedSymbol* symbol = vm->Get_Symbol_Table_Entry(obj_symbol);
	SET_REGISTER_OBJECT(result_reg, vm->New_Array(symbol, size));
	RETURN_RESULT_CONTINUE();
}	

/* GetLength  result_register, array_register */
INSTRUCTION_FUNCTION(GetLength)
{
	int result_reg = DECODE_REGISTER();
	int array_reg = DECODE_REGISTER();

	CVMObjectHandle array_obj = GET_REGISTER_OBJECT(array_reg);
	SET_REGISTER_INT(result_reg, array_obj.Get()->Slot_Count());
	RETURN_RESULT_CONTINUE();
}

/* GetIndex  result_register, array_register, index_register */
INSTRUCTION_FUNCTION(GetIndex)
{
	int result_reg = DECODE_REGISTER();
	int array_reg = DECODE_REGISTER();
	int index_reg = DECODE_REGISTER();

	int index = GET_REGISTER_INT(index_reg);
	SET_REGISTER_VALUE(result_reg, GET_REGISTER_OBJECT(array_reg).Get()->Get_Slot(index));
	RETURN_RESULT_CONTINUE();
}


/* *****************************************************************
   States
   ***************************************************************** */

/* PushState state_symbol_index  */
INSTRUCTION_FUNCTION(PushState)
{
	int symbol = DECODE_SYMBOL();
	CVMLinkedSymbol* state = vm->Get_Symbol_Table_Entry(symbol);
	Push_State(activation_context->This.object_value, state);
	RETURN_RESULT_CONTEXTCHANGED();
}

/* ChangeState state_symbol_index  */
INSTRUCTION_FUNCTION(ChangeState)
{
	int symbol = DECODE_SYMBOL();
	CVMLinkedSymbol* state = vm->Get_Symbol_Table_Entry(symbol);
	Change_State(activation_context->This.object_value, state);
	RETURN_RESULT_CONTEXTCHANGED();
}

/* PopState  */
INSTRUCTION_FUNCTION(PopState)
{
	Pop_State(activation_context->This.object_value);
	RETURN_RESULT_CONTEXTCHANGED();
}


#undef INSTRUCTION_FUNCTION
#undef RETURN_RESULT
#undef DECODE_INT
#undef DECODE_FLOAT
#undef DECODE_REGISTER
#undef DECODE_SYMBOL
#undef DECODE_JUMPTARGET
#undef DECODE_STRING
#undef SET_REGISTER_VALUE
#undef SET_REGISTER_INT
#undef SET_REGISTER_FLOAT
#undef SET_REGISTER_STRING
#undef SET_REGISTER_OBJECT
#undef GET_REGISTER_VALUE
#undef GET_REGISTER_INT
#undef GET_REGISTER_FLOAT
#undef GET_REGISTER_STRING
#undef GET_REGISTER_OBJECT
#undef SET_LOCAL_VALUE
#undef SET_LOCAL_INT
#undef SET_LOCAL_FLOAT
#undef SET_LOCAL_STRING
#undef SET_LOCAL_OBJECT
#undef GET_LOCAL_VALUE
#undef GET_LOCAL_INT
#undef GET_LOCAL_FLOAT
#undef GET_LOCAL_STRING
#undef GET_LOCAL_OBJECT
#undef SET_FIELD_VALUE
#undef SET_FIELD_INT
#undef SET_FIELD_FLOAT
#undef SET_FIELD_STRING
#undef SET_FIELD_OBJECT
#undef GET_FIELD_VALUE
#undef GET_FIELD_INT
#undef GET_FIELD_FLOAT
#undef GET_FIELD_STRING
#undef GET_FIELD_OBJECT

