/* *****************************************************************

		CVirtualMachine.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Atomic.h"

#include "XScript/VirtualMachine/Runtime/CRuntime_Array.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Log.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Debug.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Math.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Object.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_OS.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_String.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Vector2.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Vector3.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Vector4.h"
#include "XScript/VirtualMachine/Runtime/CRuntime_Type.h"

#include "XScript/VirtualMachine/CVMBindingHelper.h"

#include "Engine/Profiling/ProfilingManager.h"
#include "Engine/Platform/Platform.h"
#include "Engine/IO/Stream.h"

#include "Generic/Stats/Stats.h"
#include "Generic/Helper/PersistentLogHelper.h"

#include <algorithm>
#include <float.h>
#include <cstdlib>
#include <stdio.h>

DEFINE_FRAME_STATISTIC("Scripts/GC Time", float, g_gc_time_time, true);

#define VM_ONSCREEN_MESSAGE_ID_START 0x10000000

// If set the VM will time and log expensive functions. Functions
// deemed expensive are ones where > EXPENSIVE_FUNCTION_LOG_THRESHOLD ms 
// is spent inside of per EXPENSIVE_FUNCTION_LOG_CHECK_INTERVAL ms clock time.
// WARNING: The profiling required for this is mega fucking expensive, don't accidently
//			leave this on!
//#define LOG_EXPENSIVE_FUNCTIONS
#define EXPENSIVE_FUNCTION_LOG_THRESHOLD		0.1f
#define EXPENSIVE_FUNCTION_LOG_CHECK_INTERVAL	1000

// When defined the VM will dump an execution trace to stdout.
//#define VM_LOG_EXECUTION
//#define VM_LOG_EXECUTION_FUNCTION "RPC_Begin_Firing"
 
// Determines infinite loop.
#define INFINITE_LOOP_INSTRUCTION_COUNT 1000000
 
// If we are in debug/release release our error guard resolves to a setjmp call, otherwise
// we don't need it as we are going to hard-error.
#if MASTER_RELEASE
	#define VM_HARD_CRASH_ON_ERROR
#endif

#ifdef VM_HARD_CRASH_ON_ERROR
	#define VM_ERROR_GUARD(method) method
#else
	#define VM_ERROR_GUARD(method) \
		if (!thread_data.m_error_recovery_jump_set) \
		{ \
			thread_data.m_error_recovery_jump_set = true; \
			if (setjmp(thread_data.m_error_recovery_jump) == 0) \
				method \
			thread_data.m_error_recovery_jump_set = false; \
		} \
		else \
			method
#endif

#ifdef VM_ALLOW_MULTITHREADING
	#define VM_GET_THREAD_LOCAL_DATA(vm, var_name) CVMThreadLocalData& var_name = *(vm->m_thread_local_data.Get());
#else
	#define VM_GET_THREAD_LOCAL_DATA(vm, var_name) CVMThreadLocalData& var_name = vm->m_thread_local_data;
#endif

CVMContextLock::CVMContextLock(const CVMContextLock& other)
{
	VM_GET_THREAD_LOCAL_DATA(m_vm, data);
	data.m_context_ref_count++;
}

CVMContextLock::~CVMContextLock()
{		
	VM_GET_THREAD_LOCAL_DATA(m_vm, data);
	data.m_context_ref_count--;
	if (data.m_context_ref_count <= 0)
	{
		m_vm->Release_Context(m_old_ref_count, m_old_context);
	}
}

CVMContextLock::CVMContextLock(CVirtualMachine* machine, int old_ref_count, CVMContext* old_context)
	: m_vm(machine)
	, m_old_ref_count(old_ref_count)
	, m_old_context(old_context)
{
//	m_vm->m_context_ref_count++;
}

CVirtualMachine::CVirtualMachine()
	: m_instruction_counter(0)
	, m_instruction_delta(0)
	, m_ips_timer(0.0f)
	, m_ips_counter(0)
	, m_gc(gc_heap_size)
	//, m_context_ref_count(0)
	, m_log_function_depth(0)
	, m_expensive_log_elapsed(0.0f)
	, m_multithreaded(true)
	, m_profiling_enabled(false)
	, m_gc_collect_pending_rpcs(false)
	, m_onscreen_message_id(0)
#ifndef VM_ALLOW_MULTITHREADING
	, m_lock_thread(NULL)
	, m_lock_thread_depth(0)
#endif
{
	m_gc.Init(this);

	m_context_mutex = Mutex::Create();
	//m_context_wait_mutex = Mutex::Create();
#ifdef VM_ALLOW_MULTITHREADING
	m_native_global_thread_lock = Mutex::Create();
#endif

	m_binding_helper = new CVMBindingHelper(this);

#ifndef VM_ALLOW_MULTITHREADING
	m_thread_local_data.m_thread = Thread::Get_Current();
#endif
}

CVirtualMachine::~CVirtualMachine()
{
	m_static_context = NULL;

	for (std::vector<CVMContext*>::iterator iter = m_contexts.begin(); iter != m_contexts.end(); iter++)
	{
		CVMContext* ctx = *iter;
		SAFE_DELETE(ctx);
	}
	m_contexts.clear();

	m_symbol_table.clear();
	m_symbol_table_lookup.Clear();

	SAFE_DELETE(m_context_mutex);
}

void CVirtualMachine::Set_Profiling_Enabled(bool bEnabled)
{
	m_profiling_enabled = bEnabled;
}

CGarbageCollector* CVirtualMachine::Get_GC()
{
	return &m_gc;
}

void CVirtualMachine::Assert(bool cond)
{
	Assert(cond, "No Message");
}

void CVirtualMachine::Assert(bool cond, const char* format, ...)
{
	if (!cond)
	{
		VM_GET_THREAD_LOCAL_DATA(this, thread_data);
		CVMContext* active_context = thread_data.m_active_context;

		/*
		if (stricmp(format, "Stack overflow.") == 0)
		{
			DBG_LOG("SHIT!");
			DBG_ASSERT(false);
		}
		*/

#if VM_HARD_CRASH_ON_ERROR
		char buffer[1024];

		va_list args;
		va_start(args, format);
		vsprintf(buffer, format, args); // Buffer overflow, fix plz.
		va_end(args);

		String msg = "";
		msg += "ERROR:\n\t";
		msg += buffer;
		msg += "\n\n";
		msg += "CALLSTACK: \n";

		if (active_context != NULL)
		{
			for (int i = active_context->Activation_Stack.Size() - 1; i >= 0; i--)
			{
				String func = "";
				int line = 0;

				CActivationContext* callee = &active_context->Activation_Stack[i];	
				CVMDebugInfo* info = Resolve_Debug_Info(callee);

				//if (i > 0)
				//{
					//CActivationContext* caller = &m_active_context->Activation_Stack[i - 1];	
					func = Platform::Get()->Extract_Filename(Get_String_Table_Entry(info->file_index));
					line = info->row;
				//}

				msg += StringHelper::Format("\t [%i] %-20s %s:%i\n", 
					i + 1, 
					callee->Method->symbol->name, 
					callee->Method->symbol->method_data->is_native == 1 ? "<native>" : func.c_str(), 
					callee->Method->symbol->method_data->is_native == 1 ? 0 : line);
			}
		}
		else
		{
			msg += "No Active Context\n";
		}

		DBG_ASSERT_STR(false, "Virtual Machine Encountered Fatal Error\n\n%s", msg.c_str());
#else
		char buffer[1024];

		va_list args;
		va_start(args, format);
  		vsprintf(buffer, format, args); // Buffer overflow, fix plz.
		va_end(args);

		String msg = "[SCRIPT ERROR] ";
		msg += buffer;
		msg += "\n";

		std::string onscreen_file = "";
		int onscreen_line = 0;
		std::string onscreen_name = "";

		for (int i = active_context->Activation_Stack.Size() - 1; i >= 0; i--)
		{
			String func = "";
			int line = 0;

			CActivationContext* callee = &active_context->Activation_Stack[i];	
			CVMDebugInfo* info = Resolve_Debug_Info(callee);

			//if (i > 0)
			//{
			//CActivationContext* caller = &m_active_context->Activation_Stack[i - 1];	
			func = Platform::Get()->Extract_Filename(Get_String_Table_Entry(info->file_index));
			line = info->row;
			//}

			if (onscreen_file == "" && callee->Method->symbol->method_data->is_native != 1)
			{
				onscreen_file = callee->Method->symbol->method_data->is_native == 1 ? "<native>" : func.c_str();
				onscreen_line = callee->Method->symbol->method_data->is_native == 1 ? 0 : line;
				onscreen_name = callee->Method->symbol->name;
			}

			msg += StringHelper::Format("\t[%s:%i] %-20s\n", 
				callee->Method->symbol->method_data->is_native == 1 ? "<native>" : func.c_str(), 
				callee->Method->symbol->method_data->is_native == 1 ? 0 : line,
				callee->Method->symbol->name);
		}

		DBG_ONSCREEN_LOG(VM_ONSCREEN_MESSAGE_ID_START + ((m_onscreen_message_id++) % 10), Color::Red, 10.0f, "%s:%i:%s: %s", onscreen_file.c_str(), onscreen_line, onscreen_name.c_str(), buffer);

		DBG_LOG("%s", msg.c_str());

		// Right now we're going to try and clean up back to a state where we can hopefully keep executing.
		// Plan is:
		//		Keep poping call frames until we encounter native code frame or until we have no more left.
		//		Use longjmp to get back to get back to last error recovery state.
		//
		// This *should* put us back in the best position we can realistically get.
		thread_data.m_error_recovering = true;

		int counter = 0;
		while (active_context->Activation_Stack.Size() > 0)
		{
			CActivationContext& context = active_context->Activation_Stack.Peek();
			if (context.Method->symbol->method_data->is_native == 1 && counter != 0) // Ignore first native method as its the one we are recovering from!
			{
				break;
			}

			CVMValue retval;
			Return_Internal(retval, false);
			counter++;
		}

		thread_data.m_error_recovering = false;

		// plz live.
		//if (thread_data != NULL)
		//{
			DBG_ASSERT(thread_data.m_error_recovery_jump_set);
			longjmp(thread_data.m_error_recovery_jump, 1);
		//}
		DBG_ASSERT(false);
#endif
	}
}

CVMLinkedSymbol* CVirtualMachine::Get_Active_Function()
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	return thread_data.m_active_context->Activation_Stack.Peek().Method;
}

CVMLinkedSymbol* CVirtualMachine::Get_Stack_Frame(int offset)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	return thread_data.m_active_context->Activation_Stack[thread_data.m_active_context->Activation_Stack.Size() - offset].Method;
}

void CVirtualMachine::Sleep(float seconds)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);
	DBG_ASSERT_STR(thread_data.m_active_context != m_static_context, "Static context should not be used for latent execution. Latent execution should only be done on objects with their own contexts.");

	thread_data.m_active_context->Sleeping     = true;
	thread_data.m_active_context->Wakeup_Time  = (seconds * 1000.0f);
}

bool CVirtualMachine::On_Stack(CVMLinkedSymbol* func_symbol)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	for (int i = 0; i < thread_data.m_active_context->Activation_Stack.Size(); i++)
	{
		if (thread_data.m_active_context->Activation_Stack[i].Method == func_symbol)
		{
			return true;
		}
	}
	return false;
}

CActivationContext* CVirtualMachine::Get_Activation_Context(int offset)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	return &thread_data.m_active_context->Activation_Stack.Peek(offset);
}

void CVirtualMachine::Bind_Method_Internal(const char* class_name, const char* name, CVMNativeTrampoline* trampoline)
{
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& other = *iter;
		if (other.symbol->type == SymbolType::ClassMethod &&
			other.symbol->method_data->is_native &&
			stricmp(name, other.symbol->native_name) == 0)
		{			
			CVMLinkedSymbol* linked = Get_Symbol_Table_Entry(other.symbol->class_scope_index);
			if (stricmp(class_name, linked->symbol->native_name) == 0)
			{
				other.native_trampoline = trampoline;
				other.is_native_bound = true;
			}
		}
	}
}	

int CVirtualMachine::Add_String_To_Table(const char* str)
{
	int index = 0;
	for (std::vector<const char*>::iterator iter = m_string_table.begin(); iter != m_string_table.end(); iter++, index++)
	{
		const char* other = *iter;
		if (strcmp(str, other) == 0)
		{
			return index;
		}
	}

	m_string_table.push_back(str);
	return index;
}

bool CVirtualMachine::Is_Symbol_Same(CVMBinarySymbol* sym1, CVMBinarySymbol* sym2)
{
	return sym1->unique_id == sym2->unique_id;
}

int CVirtualMachine::Add_Symbol_To_Table(CVMBinarySymbol* symbol)
{
/*
	int index = 0;
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++, index++)
	{
		CVMLinkedSymbol& other = *iter;
		if (other.symbol->unique_id == symbol->unique_id)
		{
			return index;			
		}
	}
*/

	int index = 0;
	if (m_symbol_table_lookup.Get(symbol->unique_id, index))
	{
		//CVMLinkedSymbol* sym = Get_Symbol_Table_Entry(index);
		//DBG_ASSERT(sym->symbol->unique_id == symbol->unique_id);
		//DBG_LOG("FOUND DUPLICATE[%i]: %s", index, symbol->name);
		return index;
	}

	CVMLinkedSymbol sym;
	sym.have_instructions_been_patched	= false;
	sym.have_symbols_been_patched		= false;
	sym.symbol							= symbol;
	sym.index							= m_symbol_table.size();
	sym.is_native_bound					= false;
	sym.native_trampoline				= NULL;
	sym.name_hash						= StringHelper::Hash(sym.symbol->name);
	sym.can_reference_objects			= false;

	sym.event_name						= symbol->name;	
	if (sym.event_name.Starts_With("Event_"))
	{
		sym.event_name = sym.event_name.Sub_String(6);
	}
	sym.event_name						= "On_" + sym.event_name;
	sym.event_name_hash					= StringHelper::Hash(sym.event_name.C_Str());

	m_symbol_table.push_back(sym);
	m_symbol_table_lookup.Set(symbol->unique_id, sym.index);

	return sym.index;
}

void CVirtualMachine::Patch_Index_Array(int* src_array, int src_count, std::vector<int> remap)
{
	for (int i = 0; i < src_count; i++)
	{
		if (src_array[i] >= 0)
			src_array[i] = remap.at(src_array[i]);
	}
}

void CVirtualMachine::Patch_Data_Type(CVMDataType* dt, std::vector<int> symbol_remaps)
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

void CVirtualMachine::Patch_Symbols(std::vector<int> symbol_remaps, std::vector<int> string_remaps, CVMBinaryData* data)
{
	// Keep in sync with CVMBinary::Patch_Symbols

	int index = 0;
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++, index++)
	{
		CVMLinkedSymbol& sym = *iter;
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

void CVirtualMachine::Patch_Instructions(std::vector<int> symbol_remaps, std::vector<int> string_remaps, CVMBinaryData* data)
{
	// Keep in sync with CVMBinary::Patch_Instructions

	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& sym = *iter;
		if (sym.have_instructions_been_patched == false &&
			sym.symbol->type == SymbolType::ClassMethod)
		{
			char* buffer = sym.symbol->method_data->bytecode;
			char* end_buffer = sym.symbol->method_data->bytecode + sym.symbol->method_data->bytecode_size;
			while (buffer < end_buffer)
			{
				char* instruction_start = buffer;

				// Decode next instruction.
				CInstruction instruction;
				instruction.Decode(buffer);				

				// Patch operands.
				CInstructionMetaInfo info = InstructionMetaInfo[(int)instruction.OpCode];
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

			/*
			for (int i = 0; i < sym.symbol->method_data->instruction_count; i++)
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
			}*/

			for (int i = 0; i < sym.symbol->method_data->debug_count; i++)
			{
				CVMDebugInfo* instruction = sym.symbol->method_data->debug + i;
				instruction->file_index = string_remaps.at(instruction->file_index);
			}
		}
	
		sym.have_instructions_been_patched = true;
	}
}
/*
CVMLinkedSymbol* CVirtualMachine::Get_Symbol_Table_Entry(int index)
{
	DBG_ASSERT(index >= 0 && index < (int)m_symbol_table.size());
	return &m_symbol_table[index];
}

const char* CVirtualMachine::Get_String_Table_Entry(int index)
{
	DBG_ASSERT(index >= 0 && index < (int)m_string_table.size());
	return m_string_table[index];
}
*/
CVMLinkedSymbol* CVirtualMachine::Find_Symbol(int unique_id)
{
	int index = 0;
	if (m_symbol_table_lookup.Get(unique_id, index))
	{
		return Get_Symbol_Table_Entry(index);
	}
	return NULL;
	/*
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& sym = *iter;
		if (sym.symbol->unique_id == unique_id)
		{
			return &sym;
		}
	}*/
}

CVMLinkedSymbol* CVirtualMachine::Find_Class(const char* class_name)
{
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& sym = *iter;
		if (sym.symbol->type == SymbolType::Class &&
			stricmp(sym.symbol->name, class_name) == 0)
		{
			return &sym;
		}
	}
	return NULL;
}

std::vector<CVMLinkedSymbol*> CVirtualMachine::Find_Derived_Classes(CVMLinkedSymbol* base_class)
{
	std::vector<CVMLinkedSymbol*> result;

	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& sym = *iter;
		if (sym.symbol->type == SymbolType::Class &&
			sym.symbol->class_data->super_class_index == base_class->index)
		{
			// Get sub-symbols.
			std::vector<CVMLinkedSymbol*> sub_results = Find_Derived_Classes(&sym);
			for (std::vector<CVMLinkedSymbol*>::iterator iter = sub_results.begin(); iter != sub_results.end(); iter++)
			{
				result.push_back(*iter);
			}

			result.push_back(&sym);
		}
	}

	return result;
}

bool CVirtualMachine::Is_Class_Derived_From(CVMLinkedSymbol* child, CVMLinkedSymbol* ancestor)
{
	int target = ancestor->index;
	for (unsigned int i = 0; i < child->inherited_symbol_indexes.size(); i++)
	{
		if (child->inherited_symbol_indexes[i] == target)
		{
			return true;
		}
	}
	return false;
}

CVMLinkedSymbol* CVirtualMachine::Find_Function(CVMLinkedSymbol* class_symbol, const char* function_name, int parameter_count, ...)
{
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& sym = *iter;
		if (sym.symbol->type == SymbolType::ClassMethod &&
			sym.symbol->class_scope_index == class_symbol->index &&
			sym.symbol->method_data->parameter_count == parameter_count &&
			stricmp(sym.symbol->name, function_name) == 0)
		{
			va_list va;
			va_start(va, parameter_count);

			bool params_correct = true;
			for (int i = 0; i < parameter_count; i++)
			{
				CVMLinkedSymbol* param = Get_Symbol_Table_Entry(sym.symbol->method_data->parameter_indexes[i]);
				const char* expected_param_name = va_arg(va, const char*);
				if (stricmp(param->symbol->variable_data->data_type->name, expected_param_name) != 0)
				{
					params_correct = false;
					break;
				}
			}

			va_end(va);

			if (params_correct == true)
				return &sym;
		}
	}
	return NULL;
}

CVMLinkedSymbol* CVirtualMachine::Find_Variable(CVMLinkedSymbol* class_symbol, const char* variable_name)
{
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& sym = *iter;
		if (sym.symbol->type == SymbolType::ClassField &&
			sym.symbol->class_scope_index == class_symbol->index &&
			stricmp(sym.symbol->name, variable_name) == 0)
		{
			return &sym;
		}
	}
	return NULL;
}

void CVirtualMachine::Link(CVMBinary* binary)
{
	double ticks = Platform::Get()->Get_Ticks();

	CVMBinaryData* data = binary->GetBinaryData();

	int symbol_table_start = m_symbol_table.size();
	int string_table_start = m_string_table.size();

	std::vector<int> string_index_remaps;
	std::vector<int> symbol_index_remaps;

	string_index_remaps.reserve(data->string_table_count);
	symbol_index_remaps.reserve(data->symbol_table_count);

	m_symbol_table.reserve(m_symbol_table.size() + data->symbol_table_count);
	m_string_table.reserve(m_string_table.size() + data->string_table_count);
	
	// Add binary's strings to master string table.
	for (int i = 0; i < data->string_table_count; i++)
	{
		const char* str = data->string_table[i];
		string_index_remaps.push_back(Add_String_To_Table(str));
	}

	// Add binary's symbols to master symbol table.
	for (int i = 0; i < data->symbol_table_count; i++)
	{
		CVMBinarySymbol* sym = data->symbols[i];
		symbol_index_remaps.push_back(Add_Symbol_To_Table(sym));
	}

	int symbol_table_new = m_symbol_table.size() - symbol_table_start;
	int string_table_new = m_string_table.size() - string_table_start;

	// Go through symbol instructions and update references
	// to symbol/string indexes.
	Patch_Symbols(symbol_index_remaps, string_index_remaps, data);
	Patch_Instructions(symbol_index_remaps, string_index_remaps, data);

	/*
	DBG_LOG("============================ SYMBOL TABLE ============================");
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		CVMLinkedSymbol& sym = m_symbol_table[i];
		DBG_LOG("[%i] %s", i, sym.symbol->name);
	}
	DBG_LOG("============================ REPLICATED SYMBOLS ============================");
	for (unsigned int i = 0; i < m_symbol_table.size(); i++)
	{
		CVMLinkedSymbol& sym = m_symbol_table[i];
		if (sym.symbol->type == SymbolType::Class)
		{
			if (sym.symbol->class_data->is_replicated)
			{
				DBG_LOG("[Class=%s]", sym.symbol->name);

				CVMReplicationInfo* rep_info = sym.symbol->class_data->replication_info;
				for (int j = 0; j < rep_info->variable_count; j++)
				{
					CVMLinkedSymbol* linked_sym = Get_Symbol_Table_Entry(rep_info->variables[j].var_symbol_index >= 0 ? rep_info->variables[j].var_symbol_index : rep_info->variables[j].prop_symbol_index);
					DBG_LOG("Symbol[%i]=%s", rep_info->variables[j].var_symbol_index >= 0 ? rep_info->variables[j].var_symbol_index : rep_info->variables[j].prop_symbol_index, linked_sym->symbol->name);
				}
			}
		}
	}*/

	DBG_LOG("Linking VM binary (%s) in %.2f ms (%i new symbols, %i new strings)", data->name, Platform::Get()->Get_Ticks() - ticks, symbol_table_new, string_table_new);
}

void CVirtualMachine::Register_Runtime()
{
	CRuntime_Array::Bind(this);
	CRuntime_Log::Bind(this);
	CRuntime_Debug::Bind(this);
	CRuntime_Math::Bind(this);
	CRuntime_Object::Bind(this);
	CRuntime_OS::Bind(this);
	CRuntime_String::Bind(this);
	CRuntime_Vector2::Bind(this);
	CRuntime_Vector3::Bind(this);
	CRuntime_Vector4::Bind(this);
	CRuntime_Type::Bind(this);
}

void CVirtualMachine::Construct()
{
	DBG_LOG("Constructing initial virtual machine data ..");

	// Create static data.
	int static_data_size = 0;
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& symbol = *iter;
		if (symbol.symbol->type == SymbolType::Class)
		{
			symbol.static_data_length = symbol.symbol->class_data->static_table_size;
			if (symbol.static_data_length > 0)
			{
				symbol.static_data = Get_GC()->Alloc_Unmanaged<CVMValue>(symbol.static_data_length);
				static_data_size += symbol.static_data_length;
			}

			// Work out if we can reference other objects, speeds up GC a bit by allowing us to skip traversal of these objects.
			CVMLinkedSymbol* baseSymbol = &symbol;
			symbol.can_reference_objects = false;
			symbol.specific_slots_reference_only = true;

			// Shitty hack as Array<x> are not marked as actual arrays.
			if (strcmp(baseSymbol->symbol->native_name, "Array") == 0)
			{
				symbol.specific_slots_reference_only = false;

				//symbol.can_reference_objects = true;
				//m_object_referable_symbols.push_back(baseSymbol);

				CVMLinkedSymbol* elementSymbol = Get_Symbol_Table_Entry(baseSymbol->symbol->class_data->data_type->element_type->class_index);
				if (strcmp(elementSymbol->symbol->native_name, "float") != 0 &&
					strcmp(elementSymbol->symbol->native_name, "int") != 0 &&
					strcmp(elementSymbol->symbol->native_name, "string") != 0 &&
					strcmp(elementSymbol->symbol->native_name, "void") != 0)
				{
					symbol.can_reference_objects = true;
					m_object_referable_symbols.push_back(baseSymbol);
				}
			}
			else
			{
				for (int i = 0; i < baseSymbol->symbol->class_data->field_count; i++)
				{
					CVMLinkedSymbol* fieldSymbol = Get_Symbol_Table_Entry(baseSymbol->symbol->class_data->fields_indexes[i]);
					//DBG_LOG("\tField=%s Type=%i", fieldSymbol->symbol->name, fieldSymbol->symbol->field_data->data_type->type);
					if (fieldSymbol->symbol->field_data->data_type->type == CVMBaseDataType::Array ||
						fieldSymbol->symbol->field_data->data_type->type == CVMBaseDataType::Object)
					{
						symbol.can_reference_objects = true;
						m_object_referable_symbols.push_back(baseSymbol);
						symbol.specific_slots_reference_indexes.push_back(i);
					}
				}
			}

			/*DBG_LOG("symbol=%s, can_ref=%i, array=%i", 
				symbol.symbol->name, 
				symbol.can_reference_objects,
				symbol.symbol->class_data->data_type->type == CVMBaseDataType::Array);*/
		}

#ifdef LOG_EXPENSIVE_FUNCTIONS
		symbol.inclusive_time = 0.0f;
#endif
	}
	DBG_LOG("Created %i fields of static data.", static_data_size);

	// Create inheritance lookup tables for faster casting.
	DBG_LOG("Constructing inheritance lookup table ..");
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol* base_symbol = &(*iter);
		CVMLinkedSymbol* child = base_symbol;
		
		if (base_symbol->symbol->type == SymbolType::Class)
		{
			while (true)
			{
				base_symbol->inherited_symbol_indexes.push_back(child->index);

				for (int i = 0; i < child->symbol->class_data->interface_count; i++)
				{
					CVMLinkedSymbol* in = Get_Symbol_Table_Entry(child->symbol->class_data->interfaces[i].interface_symbol_index);
					base_symbol->inherited_symbol_indexes.push_back(in->index);
				}

				if (child->symbol->class_data->super_class_index <= 0)
				{
					break;
				}
				else
				{
					child = Get_Symbol_Table_Entry(child->symbol->class_data->super_class_index);
				}
			}
		}
	}

	// Create typeof() instances.
	DBG_LOG("Constructing typeof instances ...");
	CVMLinkedSymbol* type_class = Find_Class("Type");
	DBG_ASSERT(type_class != NULL);
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& symbol = *iter;
		if (symbol.symbol->type == SymbolType::Class)
		{
			symbol.type_instance = New_Object(type_class, false, &symbol);
		}
	}

	// Grab some system classes.
	Vec2_Class = Find_Class("Vec2");
	Vec3_Class = Find_Class("Vec3");
	Vec4_Class = Find_Class("Vec4");
	Int_Class = Find_Class("int");
	IntArray_Class = Find_Class("array<int>");
	Float_Class = Find_Class("float");
	FloatArray_Class = Find_Class("array<float>");
	String_Class = Find_Class("string");
	StringArray_Class = Find_Class("array<string>");

	// Bind runtime libraries to native symbols.
	Register_Runtime();

	// Throw errors for any unresolved native symbols.
	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& other = *iter;
		if (other.symbol->type == SymbolType::ClassMethod &&
			other.symbol->method_data->is_native &&
			other.is_native_bound == false)
		{
			CVMLinkedSymbol* class_scope = Get_Symbol_Table_Entry(other.symbol->class_scope_index);
			DBG_LOG("[WARNING] %s::%s has not been bound to native interface.", class_scope->symbol->name, other.symbol->name);
		}
	}

	// Run class constructors.
	m_static_context = New_Context(NULL);
	CVMContextLock lock = Set_Context(m_static_context);

	for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
	{
		CVMLinkedSymbol& symbol = *iter;
		if (symbol.symbol->type == SymbolType::ClassMethod &&
			symbol.symbol->method_data->is_constructor != 0&&
			symbol.symbol->method_data->is_static != 0)
		{
			CVMValue instance;
			Invoke_Internal(&symbol, instance, false, false, -1);
		}
	}
}

CVMContext* CVirtualMachine::Get_Active_Context()
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	return thread_data.m_active_context;
}

CVMContext* CVirtualMachine::Get_Static_Context()
{
	return m_static_context;
}

CVMContext* CVirtualMachine::New_Context(void* metadata)
{
	MutexLock lock(m_context_mutex);

	CVMContext* ctx = new CVMContext(this, stack_size, activation_stack_size, metadata);
	m_contexts.push_back(ctx);
	return ctx;
}

void CVirtualMachine::Free_Context(CVMContext* context)
{	
	MutexLock lock(m_context_mutex);

	for (std::vector<CVMContext*>::iterator iter = m_contexts.begin(); iter != m_contexts.end(); iter++)
	{
		CVMContext* ctx = *iter;
		if (ctx == context)
		{
			m_contexts.erase(iter);
			SAFE_DELETE(ctx);
			return;
		}
	}
	DBG_ASSERT(false);
}

int CVirtualMachine::Get_Context_Count()
{
	return m_contexts.size();
}

CVMDebugInfo* CVirtualMachine::Resolve_Debug_Info(CActivationContext* context)
{
	static CVMDebugInfo s_blank;
	s_blank.file_index = 0;
	s_blank.column = 0;
	s_blank.row = 0;
	s_blank.bytecode_offset = 0;

	CVMDebugInfo* best = NULL;

	int ip_offset = ((char*)context->IP - (char*)context->Method->symbol->method_data->bytecode);

	for (int i = 0; i < context->Method->symbol->method_data->debug_count; i++)
	{
		CVMDebugInfo* info = context->Method->symbol->method_data->debug + i;
		if (info->bytecode_offset <= ip_offset)
		{
			best = info;
		}
		else
		{
			break;
		}
	}

	return best == NULL ? &s_blank : best;
}

void CVirtualMachine::Release_Context(int old_ref_count, CVMContext* old_context)
{
	VM_GET_THREAD_LOCAL_DATA(this, data);
	DBG_ASSERT_STR(data.m_active_context != NULL, "Attempt to release context without active context (multithreaded=%i).", m_multithreaded);

#ifdef VM_ALLOW_MULTITHREADING

	// Free locked thread.
	data.m_active_context->Lock_Depth--;
	if (data.m_active_context->Lock_Depth <= 0)
	{
		DBG_ASSERT(data.m_active_context->Lock_Thread != NULL);
		data.m_active_context->Lock_Thread = NULL;
		data.m_active_context->Lock_Set_By_Release = true;
		data.m_active_context->Locked = false;
	}

	//DBG_LOG("Thread 0x%08x released context 0x%08x (new context 0x%08x)", Thread::Get_Current(), data.m_active_context, old_context);

	// Switch to old context.
	data.m_active_context = old_context;
	data.m_context_ref_count = old_ref_count;

	// Unlock VM if no more contexts.
	if (data.m_active_context == NULL)
	{
		m_lock_thread = NULL;
	}
#else

	// Clear stacks so things can be GC'd correctly.
	data.m_active_context->Stack.Clear_To_Watermark();
	data.m_active_context->Activation_Stack.Clear_To_Watermark();

	data.m_active_context = old_context;
	data.m_context_ref_count = old_ref_count;

	//DBG_LOG("[m_lock_thread_depth:0x%08x] Dec %i", Thread::Get_Current(), m_lock_thread_depth);
	m_lock_thread_depth--;
	if (m_lock_thread_depth <= 0)
	{
		//DBG_LOG("Thread '%s' unlocked context 0x%08x", Thread::Get_Current()->Get_Name().c_str(), old_context);

		m_lock_thread = NULL;
		m_lock_thread_depth = 0;
	}
	/*
	DBG_LOG("[UNLOCK] thread='%s' ref_count=%i context=0x%08x m_lock_thread_depth=%i m_multithreaded=%i)", 
		Thread::Get_Current()->Get_Name().c_str(), 
		data.m_context_ref_count, 
		data.m_active_context, 
		m_lock_thread_depth,
		m_multithreaded);
		*/
#endif
}

CVMContextLock CVirtualMachine::Set_Context(CVMContext* context)
{
	// Requirements:
	//		- Contexts can only be active on one thread at a time,
	//		  thread must wait until context is free before setting it.
	//			- Note this could cause deadlocks if multiple contexts are waiting on each other (sending events?)
	//		- GC needs to lock.
	//		- Native function calls need to be locked.

	// Setup thread local data if we haven't already.
	VM_GET_THREAD_LOCAL_DATA(this, data);
	Thread* current_thread = Thread::Get_Current(); 

#ifdef VM_ALLOW_MULTITHREADING
	if (data == NULL)
	{
		data = new CVMThreadLocalData();
		data->m_thread = current_thread;
		data->m_active_context = NULL;
		data->m_context_ref_count = 0;
		data->m_error_recovery_jump_set = false;

		m_thread_local_data.Set(data);
	}

	// If context is locked by another thread. We need to acquire the lock.
	bool owned_by_thread = (context->Locked == 1 && context->Lock_Thread == current_thread);
	if (!owned_by_thread)
	{
		while (true)
		{
			if (Atomic::CompareExchange32(&context->Locked, 0, 1) == 0)
			{
				DBG_ASSERT(context->Lock_Thread == NULL);
				context->Lock_Set_By_Release = false;
				context->Lock_Depth = 0;
				context->Lock_Thread = current_thread;
				break;
			}
		}
	}

	context->Lock_Depth++;

	// If this is nested lock 
	int old_ref_count = data.m_context_ref_count;
	CVMContext* old_context = data.m_active_context;

	data.m_active_context = context;
	data.m_context_ref_count = 0;

	//DBG_LOG("Thread 0x%08x locked context 0x%08x", Thread::Get_Current(), data.m_active_context);
	//	DBG_ASSERT(data.m_active_context->Lock_Thread == current_thread);//, "Lock thread 0x%08x is not the same as current thread 0x%08x.", data->m_active_context->Lock_Thread, current_thread);

	return CVMContextLock(this, old_ref_count, old_context);


#else
	
	// If context is locked by another thread. We need to acquire the lock.
	bool owned_by_thread = (m_lock_thread == current_thread);
	if (!owned_by_thread)
	{
		if (!m_multithreaded)
		{
			DBG_ASSERT_STR(m_lock_thread == NULL, "Lock owned by thread '%s' in non-multithreaded (%i) vm.", m_lock_thread->Get_Name().c_str(), m_multithreaded);
			m_lock_thread = current_thread;
		}
		else
		{
			while (true)
			{
				int* lock_thread_ptr = (int*)((void*)&m_lock_thread);
				int  current_thread_int = (int)current_thread;
				if (Atomic::CompareExchange32(lock_thread_ptr, 0, current_thread_int) == 0)
				{
					break;
				}
			}
		}
	}

	//DBG_LOG("[m_lock_thread_depth:0x%08x] Inc %i", Thread::Get_Current(), m_lock_thread_depth);
	m_lock_thread_depth++;

	// If this is nested lock 
	int old_ref_count = data.m_context_ref_count;
	CVMContext* old_context = data.m_active_context;
	
	data.m_active_context = context;
	data.m_context_ref_count = 0;

/*	DBG_LOG("[LOCK] thread='%s' ref_count=%i context=0x%08x m_lock_thread_depth=%i m_multithreaded=%i)", 
		Thread::Get_Current()->Get_Name().c_str(), 
		data.m_context_ref_count, 
		data.m_active_context, 
		m_lock_thread_depth,
		m_multithreaded);
*/

	return CVMContextLock(this, old_ref_count, old_context);

#endif
}

CVMObjectHandle CVirtualMachine::New_Object(CVMLinkedSymbol* class_type, bool call_constructor, void* meta_data)
{
	CVMObject* obj = NULL;
	
	{
	#ifdef VM_PROFILE_BLOCKS
		PROFILE_SCOPE("Object Alloc");
	#endif
		obj = new(m_gc.Alloc(sizeof(CVMObject))) CVMObject(this);
		obj->Set_Meta_Data(meta_data);
		obj->Init_Object(class_type);
		//DBG_LOG("@ New Object: type=%s", class_type->symbol->name);
	}
	
	CVMObjectHandle handle = CVMObjectHandle(obj);

	if (call_constructor == true)
	{
		if (class_type->symbol->class_data->default_constructor_index > 0)
		{
			CVMLinkedSymbol* constructor = Get_Symbol_Table_Entry(class_type->symbol->class_data->default_constructor_index);
			CVMValue instance = handle;
			Invoke_Internal(constructor, instance, false, false, -1);
		}
	}

	return handle;
}

void CVirtualMachine::Set_Default_State(CVMObjectHandle handle)
{
	CVMObject* obj = handle.Get();
	CVMLinkedSymbol* class_type = obj->Get_Symbol();

	CVMLinkedSymbol* default_state = Get_Default_State(handle);
	if (default_state != NULL)
	{
		Push_State(handle, default_state);
		return;
	}
}

CVMObjectHandle CVirtualMachine::New_Array(CVMLinkedSymbol* class_type, int size)
{
	CVMObject* obj = NULL; 
	
	{
#ifdef VM_PROFILE_BLOCKS
		PROFILE_SCOPE("Array Alloc");
#endif

		obj = new(m_gc.Alloc(sizeof(CVMObject))) CVMObject(this);
		obj->Init_Array(class_type, size);
	}

//	DBG_LOG("@ New Array: size=%i type=%s", size, class_type->symbol->name);
	return CVMObjectHandle(obj);
}

CVMObjectHandle CVirtualMachine::Cast_Object(CVMObjectHandle handle, CVMLinkedSymbol* to_class)
{
	CVMObject* obj = handle.Get();

	// NULL object :S
	if (obj == NULL)
	{
		return NULL;
	}

#ifndef MASTER_BUILD
	// DEBUG DEBUG: Lets us get a stack trace.
	if (obj == (CVMObject*)0xFEEEFEEE)
	{
		Assert(false, "Attempt to cast disposed object.");
	}
#endif

	CVMLinkedSymbol* from_class = obj->Get_Symbol();

	// Cast to interface
	if (to_class->symbol->class_data->is_interface)
	{
		// Check object derives from interface.
		for (int i = 0; i < from_class->symbol->class_data->interface_count; i++)
		{
			CVMInterface& inter = from_class->symbol->class_data->interfaces[i];
			if (inter.interface_symbol_index == to_class->index)
			{
				return CVMObjectHandle(obj, inter.vf_table_offset);
			}
		}
	}

	// Up-cast. object->aclass, object->array, etc
	else
	{
		// Traverse parent classes until we find the to_class.
		CVMLinkedSymbol* sym = from_class;
		while (sym != NULL && sym->symbol->type != SymbolType::Root)
		{
			// Is target type.
			if (sym->symbol->unique_id == to_class->symbol->unique_id)
			{
				return CVMObjectHandle(obj);
			}

			sym = Get_Symbol_Table_Entry(sym->symbol->class_data->super_class_index);
		}
	}

	return NULL;
}

CVMLinkedSymbol* CVirtualMachine::Get_Current_State(CVMObjectHandle obj)
{
	return obj.Get()->Current_State();
}

CVMLinkedSymbol* CVirtualMachine::Get_Default_State(CVMObjectHandle obj)
{
	CVMLinkedSymbol* class_type = obj.Get()->Get_Symbol();

	// Head up the inheritance tree till we find a default state.
	while (class_type != NULL && class_type->symbol->type != SymbolType::Root)
	{
		if (class_type->symbol->class_data->default_state_index >= 0)
		{
			return Get_Symbol_Table_Entry(class_type->symbol->class_data->default_state_index);
		}

		class_type = Get_Symbol_Table_Entry(class_type->symbol->class_data->super_class_index);
	}

	return NULL;
}

void  CVirtualMachine::Change_State(CVMObjectHandle obj, CVMLinkedSymbol* new_state)
{
	if (obj.Get()->Current_State() != NULL)
	{
		Pop_State(obj);
	}
	Push_State(obj, new_state);
}

void CVirtualMachine::Push_State(CVMObjectHandle obj, CVMLinkedSymbol* new_state)
{
	obj.Get()->Push_State(new_state);
	
	// Call OnEnter event.
	CVMLinkedSymbol* sym = Find_Event(new_state, obj.Get()->Get_Symbol(), "On_Enter", 0);
	if (sym != NULL)
	{
		CVMValue instance = obj;
		Invoke(sym, instance, false, false);
	}
}

void CVirtualMachine::Pop_State(CVMObjectHandle obj)
{
	CVMLinkedSymbol* current_state = obj.Get()->Current_State();
	DBG_ASSERT(current_state != NULL);

	// Call OnExit event.
	CVMLinkedSymbol* sym = Find_Event(current_state, obj.Get()->Get_Symbol(), "On_Exit", 0);
	if (sym != NULL)
	{
		CVMValue instance = obj;
		Invoke(sym, instance, false, false);
	}

	obj.Get()->Pop_State();
}

CVMLinkedSymbol* CVirtualMachine::Find_State(CVMObjectHandle obj, const char* state_name)
{
	CVMLinkedSymbol* sym = obj.Get()->Get_Symbol();
	for (int i = 0; i < sym->symbol->class_data->state_count; i++)
	{
		CVMLinkedSymbol* state = Get_Symbol_Table_Entry(sym->symbol->class_data->state_indexes[i]);
		if (stricmp(state->symbol->name, state_name) == 0)
		{
			return state;
		}
	}
	return NULL;
}

CVMObjectHandle CVirtualMachine::Create_Vec2(float x, float y)
{
	CVMObjectHandle obj = New_Object(Vec2_Class, false, NULL);
	obj.Get()->Resize(2);
	obj.Get()->Get_Slot(0).float_value = x;
	obj.Get()->Get_Slot(1).float_value = y;

	return obj;
}

CVMObjectHandle CVirtualMachine::Create_Vec2(Vector2 x)
{
	CVMObjectHandle obj = New_Object(Vec2_Class, false, NULL);
	obj.Get()->Resize(2);
	obj.Get()->Get_Slot(0).float_value = x.X;
	obj.Get()->Get_Slot(1).float_value = x.Y;
	return obj;
}

CVMObjectHandle CVirtualMachine::Create_Vec3(float x, float y, float z)
{
	CVMObjectHandle obj = New_Object(Vec3_Class, false, NULL);
	obj.Get()->Resize(3);
	obj.Get()->Get_Slot(0).float_value = x;
	obj.Get()->Get_Slot(1).float_value = y;
	obj.Get()->Get_Slot(2).float_value = z;
	return obj;
}

CVMObjectHandle CVirtualMachine::Create_Vec3(Vector3 x)
{
	CVMObjectHandle obj = New_Object(Vec3_Class, false, NULL);
	obj.Get()->Resize(3);
	obj.Get()->Get_Slot(0).float_value = x.X;
	obj.Get()->Get_Slot(1).float_value = x.Y;
	obj.Get()->Get_Slot(2).float_value = x.Z;
	return obj;
}

CVMObjectHandle CVirtualMachine::Create_Vec4(float x, float y, float z, float w)
{
	CVMObjectHandle obj = New_Object(Vec4_Class, false, NULL);
	obj.Get()->Resize(4);
	obj.Get()->Get_Slot(0).float_value = x;
	obj.Get()->Get_Slot(1).float_value = y;
	obj.Get()->Get_Slot(2).float_value = z;
	obj.Get()->Get_Slot(3).float_value = w;
	return obj;
}

CVMObjectHandle CVirtualMachine::Create_Vec4(Vector4 x)
{
	CVMObjectHandle obj = New_Object(Vec4_Class, false, NULL);
	obj.Get()->Resize(4);
	obj.Get()->Get_Slot(0).float_value = x.X;
	obj.Get()->Get_Slot(1).float_value = x.Y;
	obj.Get()->Get_Slot(2).float_value = x.Z;
	obj.Get()->Get_Slot(3).float_value = x.W;
	return obj;
}

CVMLinkedSymbol* CVirtualMachine::Find_Event(CVMLinkedSymbol* state_symbol, CVMLinkedSymbol* class_symbol, const char* event_name, int parameter_count, ...)
{
	int event_name_hash = StringHelper::Hash(event_name);

	if (state_symbol != NULL)
	{
		for (int i = 0; i < state_symbol->symbol->state_data->event_count; i++)
		{
			CVMLinkedSymbol* sym = Get_Symbol_Table_Entry(state_symbol->symbol->state_data->event_indexes[i]);

			if (sym->symbol->method_data->parameter_count == parameter_count &&
				sym->name_hash == event_name_hash)
			{
				va_list va;
				va_start(va, parameter_count);

				bool params_correct = true;
				for (int i = 0; i < parameter_count; i++)
				{
					CVMLinkedSymbol* param = Get_Symbol_Table_Entry(sym->symbol->method_data->parameter_indexes[i]);
					const char* expected_param_name = va_arg(va, const char*);
					if (stricmp(param->symbol->variable_data->data_type->name, expected_param_name) != 0)
					{
						params_correct = false;
						break;
					}
				}

				va_end(va);

				if (params_correct == true)
					return sym;
			}
		}
	}

	// Go through global events.
	for (int i = 0; i < class_symbol->symbol->children_count; i++)
	{
		CVMLinkedSymbol* sym = Get_Symbol_Table_Entry(class_symbol->symbol->children_indexes[i]);

		if (sym->symbol->method_data->parameter_count == parameter_count &&
			sym->name_hash == event_name_hash)
		{
			va_list va;
			va_start(va, parameter_count);

			bool params_correct = true;
			for (int i = 0; i < parameter_count; i++)
			{
				CVMLinkedSymbol* param = Get_Symbol_Table_Entry(sym->symbol->method_data->parameter_indexes[i]);
				const char* expected_param_name = va_arg(va, const char*);
				if (stricmp(param->symbol->variable_data->data_type->name, expected_param_name) != 0)
				{
					params_correct = false;
					break;
				}
			}

			va_end(va);

			if (params_correct == true)
				return sym;
		}
	}

	return NULL;
}

CVMLinkedSymbol* CVirtualMachine::Find_Event(CVMLinkedSymbol* state_symbol, CVMLinkedSymbol* class_symbol, int event_name_hash, int parameter_count, ...)
{
	if (state_symbol != NULL)
	{
		for (int i = 0; i < state_symbol->symbol->state_data->event_count; i++)
		{
			CVMLinkedSymbol* sym = Get_Symbol_Table_Entry(state_symbol->symbol->state_data->event_indexes[i]);

			if (sym->symbol->method_data->parameter_count == parameter_count &&
				sym->name_hash == event_name_hash)
			{
				va_list va;
				va_start(va, parameter_count);

				bool params_correct = true;
				for (int i = 0; i < parameter_count; i++)
				{
					CVMLinkedSymbol* param = Get_Symbol_Table_Entry(sym->symbol->method_data->parameter_indexes[i]);
					const char* expected_param_name = va_arg(va, const char*);
					if (stricmp(param->symbol->variable_data->data_type->name, expected_param_name) != 0)
					{
						params_correct = false;
						break;
					}
				}

				va_end(va);

				if (params_correct == true)
					return sym;
			}
		}
	}

	// Go through global events.
	for (int i = 0; i < class_symbol->symbol->children_count; i++)
	{
		CVMLinkedSymbol* sym = Get_Symbol_Table_Entry(class_symbol->symbol->children_indexes[i]);

		if (sym->symbol->method_data->parameter_count == parameter_count &&
			sym->name_hash == event_name_hash)
		{
			va_list va;
			va_start(va, parameter_count);

			bool params_correct = true;
			for (int i = 0; i < parameter_count; i++)
			{
				CVMLinkedSymbol* param = Get_Symbol_Table_Entry(sym->symbol->method_data->parameter_indexes[i]);
				const char* expected_param_name = va_arg(va, const char*);
				if (stricmp(param->symbol->variable_data->data_type->name, expected_param_name) != 0)
				{
					params_correct = false;
					break;
				}
			}

			va_end(va);

			if (params_correct == true)
				return sym;
		}
	}

	return NULL;
}

void CVirtualMachine::Create_RPC_Event(CVMLinkedSymbol* symbol, CVMValue& instance)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	bool is_static = symbol->symbol->method_data->is_static != 0;

	CVMRPCEvent evt;

	for (int i = 0; i < symbol->symbol->method_data->parameter_count; i++)
	{
		CVMValue param = thread_data.m_active_context->Stack.Pop_Fast();
		evt.parameters.insert(evt.parameters.begin(), param);
	}
	
	evt.symbol = symbol;

	evt.except = thread_data.m_active_context->Stack.Pop_Fast().int_value;
	evt.target = thread_data.m_active_context->Stack.Pop_Fast().int_value;

	evt.instance = instance;

	{
		MutexLock lock(m_context_mutex);
		m_pending_rpcs.push_back(evt);
	}
}

bool CVirtualMachine::Get_RPC_Event(CVMRPCEvent& evt)
{
	if (m_pending_rpcs.size() <= 0)
	{
		return false;
	}

	evt = m_pending_rpcs.at(0);
	m_pending_rpcs.erase(m_pending_rpcs.begin());

	return true;
}

void CVirtualMachine::Dump_Content_State()
{			
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	DBG_LOG("\t{ Context State 0x%08x }", thread_data.m_active_context);
	for (int i = 0; i < thread_data.m_active_context->Activation_Stack.Size(); i++)
	{
		CActivationContext* ac = &thread_data.m_active_context->Activation_Stack[i];	
		DBG_LOG("\t [%i] 0x%08x %s (%i)", i, ac->Method->symbol, ac->Method->symbol->name, ac->Method->symbol->unique_id);
	}
}

void CVirtualMachine::Invoke_Internal(CVMLinkedSymbol* symbol, CVMValue instance, bool async, bool ignore_vftable, int return_register, bool ignore_rpc)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);

	DBG_ASSERT_STR(thread_data.m_active_context != NULL, "Attempt to invoke symbol '%s' without active context (multithreaded=%i).", 
		symbol == NULL ? "<unknown>" : symbol->symbol->name, 
		m_multithreaded);

	DBG_ASSERT(symbol->symbol->type == SymbolType::ClassMethod);

	bool is_static = symbol->symbol->method_data->is_static != 0;
	bool is_native = symbol->symbol->method_data->is_native != 0;
	bool is_virtual = symbol->symbol->method_data->is_virtual != 0;

	{
		int vf_offset = 0;

		CVMObject* object_instance = NULL;

		if (is_native == false || is_virtual == true)
		{
			object_instance = instance.object_value.Get();
			Assert(is_static == true || instance.object_value.Get() != NULL, "Attempt to invoke method '%s' of null object.", symbol->symbol->name);
		}

		if (symbol->symbol->method_data->is_rpc != 0 && ignore_rpc == false)
		{ 
			Create_RPC_Event(symbol, instance);
			return;
		}

		// Lookup VFTable entry.
		if (is_static == false && is_virtual == true && ignore_vftable == false)
		{
			Assert(symbol->symbol->class_scope_index != 0, "Invalid VFTable lookup class.");

			CVMLinkedSymbol* class_symbol = Get_Symbol_Table_Entry(symbol->symbol->class_scope_index);
			CVMLinkedSymbol* base_symbol  = symbol;

			int method_base_vf_index = symbol->symbol->method_data->vf_table_index;
			int object_vf_index_offset = instance.object_value.Get_VF_Table_Offset();

			int vf_table_index = method_base_vf_index + object_vf_index_offset;
			int func_index = object_instance->Get_Symbol()->symbol->class_data->virtual_table_indexes[vf_table_index];		

			symbol = Get_Symbol_Table_Entry(func_index);		

			Assert(symbol->index != 0, "Invalid VFTable lookup class.");

			is_native = symbol->symbol->method_data->is_native != 0;

			// Remove interface offset after the initial call, sub calls will be calling absolute vf table addresses.
			instance.object_value = CVMObjectHandle(instance.object_value.Get(), 0); 
		}

		Assert(thread_data.m_active_context->Activation_Stack.Available() > 1, "Stack overflow.");

		CActivationContext& context = thread_data.m_active_context->Activation_Stack.Push_And_Return();
		context.Method				= symbol;
		context.IP					= (u8*)symbol->symbol->method_data->bytecode;
		context.Local_Offset		= thread_data.m_active_context->Stack.Size() - symbol->symbol->method_data->parameter_count;
		context.Register_Offset		= context.Local_Offset + symbol->symbol->method_data->local_count;// + symbol->symbol->method_data->parameter_count;
		context.Result_Register		= return_register;
		context.This				= instance;
		context.Finish_On_Return	= !async;

	#ifdef VM_PROFILE_BLOCKS
		context.Profile_Block		= new ScopeProfiler(symbol->symbol->name, __FUNCTION__, __FILE__, __LINE__);
	#endif

	#ifdef VM_LOG_EXECUTION
	//	CVMLinkedSymbol* c = Get_Symbol_Table_Entry(symbol->symbol->class_scope_index);
	//	DBG_LOG("CALLING: %s::%s", c->symbol->name, context.Method->symbol->name);
	#endif

		// Log function?
#ifdef VM_LOG_EXECUTION
#ifdef VM_LOG_EXECUTION_FUNCTION 
		if (stricmp(symbol->symbol->name, VM_LOG_EXECUTION_FUNCTION) == 0)
		{
			m_log_function_depth++;
			DBG_LOG("ENTERED %s", VM_LOG_EXECUTION_FUNCTION);
		}
#endif
#endif

		// Push stack space for locals.
		int stack_space = (symbol->symbol->method_data->local_count - symbol->symbol->method_data->parameter_count) + symbol->symbol->method_data->register_count;

		/*
		if (m_log_function_depth > 0)
		{
			DBG_LOG("IVK[%s], Stack=%i Pushing=%i", symbol->symbol->name, thread_data.m_active_context->Stack.Size(), stack_space);
			if (std::string(symbol->symbol->name) == "RPC_Update_Weapon_State")
			{
				static int counter = 0;
				if (counter++ > 20)
				{
					DBG_LOG("wut");
				}
			}
		}*/


		if (is_native == false)
		{
			Assert(thread_data.m_active_context->Stack.Available() >= stack_space, "Stack overflow.");
			thread_data.m_active_context->Stack.Push_Block(stack_space);
		}

		// Invoke native trampoline.
		if (is_native == true)
		{
			if (!context.Method->is_native_bound)
			{	
				CVMLinkedSymbol* class_symbol = Get_Symbol_Table_Entry(symbol->symbol->class_scope_index);
				Assert(false, "Native method %s::%s is not bound.", class_symbol->symbol->name, context.Method->symbol->name);
			}

	#ifdef MASTER_BUILD
			try
			{
	#endif

				bool bProfile = false;
				if (m_profiling_enabled == true)
				{
					bProfile = true;
				}
	#ifdef LOG_EXPENSIVE_FUNCTIONS
				bProfile = true;
	#endif

				double begin = 0.0f;
				if (bProfile)
				{
					begin = Platform::Get()->Get_Ticks();
				}

	#ifdef VM_PROFILE_BLOCKS
				PROFILE_SCOPE(symbol->symbol->name);
	#endif

				context.Method->native_trampoline->Invoke(this);

				if (bProfile)
				{
					double elapsed = Platform::Get()->Get_Ticks() - begin;
					if (elapsed > 0.1f)
						printf("%s took %.1f\n", symbol->symbol->name, elapsed);
#ifdef LOG_EXPENSIVE_FUNCTIONS
					context.Method->inclusive_time += elapsed;
#endif
				}

	#ifdef MASTER_BUILD
			}
			catch (...)
			{
				CVMLinkedSymbol* class_symbol = Get_Symbol_Table_Entry(symbol->symbol->class_scope_index);
				Assert(false, "Native exception caused during native call to '%s::%s'.", class_symbol->symbol->name, context.Method->symbol->name);
			}
	#endif

			context.Result_Register = -1;

			CVMValue retval;
			Return_Internal(retval, false);
		}

	/*
		DBG_LOG("============= Invoking %s (Sleeping=%i) ============", symbol->symbol->name, m_active_context->Sleeping);
		for (int i = 0; i < m_active_context->Activation_Stack.Size(); i++)
		{
			DBG_LOG("[%i] %s::%s", i, Get_Symbol_Table_Entry(m_active_context->Activation_Stack[i].Method->symbol->class_scope_index)->symbol->name, m_active_context->Activation_Stack[i].Method->symbol->name);
		}
	*/
	}

	// If not async, then run until the function ends.
	if (async == false && is_native == false)
	{
#ifdef LOG_EXPENSIVE_FUNCTIONS
		double begin = Platform::Get()->Get_Ticks();
#endif

		bool wasSleeping = thread_data.m_active_context->Sleeping;
		thread_data.m_active_context->Sleeping = false;

		VM_ERROR_GUARD({
			Execute();
		})
		
		thread_data.m_active_context->Sleeping = thread_data.m_active_context->Sleeping || wasSleeping;

#ifdef LOG_EXPENSIVE_FUNCTIONS
		double elapsed = Platform::Get()->Get_Ticks() - begin;
		symbol->inclusive_time += elapsed;
#endif
	}
}

bool CVirtualMachine::Return_Internal(CVMValue& result, bool set_native_result)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* activation_context = &thread_data.m_active_context->Activation_Stack.Peek();	

	bool finish_on_return = activation_context->Finish_On_Return;

	//Set_Return_Value(result);

	// Set the result register of the previous activation context.
	if (activation_context->Result_Register >= 0)
	{
		CActivationContext& old_activation_context = thread_data.m_active_context->Activation_Stack[thread_data.m_active_context->Activation_Stack.Size() - 2];

		thread_data.m_active_context->Stack[old_activation_context.Register_Offset + activation_context->Result_Register] = result;		
	}

#ifdef VM_PROFILE_BLOCKS
	SAFE_DELETE(activation_context->Profile_Block);
#endif

	if (set_native_result == true)
		thread_data.m_native_return_value = result;

	// Pop off all locals/parameters.
	int count = activation_context->Method->symbol->method_data->local_count + activation_context->Method->symbol->method_data->register_count;

	if (m_log_function_depth > 0 )
	{
		DBG_LOG("RET[]%s, Stack=%i Popping=%i", activation_context->Method->symbol->name, thread_data.m_active_context->Stack.Size(), count);
	}


#ifdef VM_LOG_EXECUTION
#ifdef VM_LOG_EXECUTION_FUNCTION 
	if (stricmp(activation_context->Method->symbol->name, VM_LOG_EXECUTION_FUNCTION) == 0)
	{
	//	m_log_function_depth--;
		DBG_LOG("LEFT %s", VM_LOG_EXECUTION_FUNCTION);
	}
#endif
#endif

	// If we are error-recovering, we need to pop off as much as it required to get back to original stack size, as there
	// many be temporary values currently on the stack, so just poping our frame will not be enough.
	if (thread_data.m_error_recovering)
	{
		count = thread_data.m_active_context->Stack.Size() - activation_context->Local_Offset;
		DBG_ASSERT_STR(count >= 0, "Stack appears corrupted, unable to return during error recovery.");
	}

	Assert(thread_data.m_active_context->Stack.Size() >= count, "Stack underflow.");
	thread_data.m_active_context->Stack.Pop_Block(count);

	// Cannot do assert as assert will try to call this :S
	DBG_ASSERT_STR(thread_data.m_active_context->Stack.Size() == activation_context->Local_Offset, "Stack appears corrupted, expected return size of %i but got %i.", activation_context->Local_Offset, thread_data.m_active_context->Stack.Size());

	//DBG_LOG("RET %s, Stack=%i. Poping %i", activation_context->Method->symbol->name, m_active_context->Stack.Size(), count);

	// Pop off activation context.
	Assert(thread_data.m_active_context->Activation_Stack.Size() >= 1, "Stack underflow.");
	thread_data.m_active_context->Activation_Stack.Pop_Fast();

	return finish_on_return;
}

void CVirtualMachine::Invoke(CVMLinkedSymbol* symbol, CVMValue instance, bool async, bool ignore_vftable, bool ignore_rpc)
{
	Invoke_Internal(symbol, instance, async, ignore_vftable, -1, ignore_rpc);
}

void CVirtualMachine::Get_Field(CVMLinkedSymbol* field_symbol, CVMObjectHandle instance, CVMValue& output)
{
	DBG_ASSERT(field_symbol->symbol->type == SymbolType::ClassField);
	output = instance.Get()->Get_Slot(field_symbol->symbol->field_data->offset);
}

void CVirtualMachine::Set_Field(CVMLinkedSymbol* field_symbol, CVMObjectHandle instance, CVMValue& value)
{
	DBG_ASSERT(field_symbol->symbol->type == SymbolType::ClassField);
	instance.Get()->Get_Slot(field_symbol->symbol->field_data->offset) = value;
}

void CVirtualMachine::Get_Static(CVMLinkedSymbol* class_symbol, CVMLinkedSymbol* field_symbol, CVMValue& output)
{
	DBG_ASSERT(field_symbol->symbol->type == SymbolType::ClassField);
	output = class_symbol->static_data[field_symbol->symbol->field_data->offset];
}

void CVirtualMachine::Set_Static(CVMLinkedSymbol* class_symbol, CVMLinkedSymbol* field_symbol, CVMValue& value)
{
	DBG_ASSERT(field_symbol->symbol->type == SymbolType::ClassField);
	class_symbol->static_data[field_symbol->symbol->field_data->offset] = value;
}

void CVirtualMachine::Get_Value(CVMObjectHandle instance, String path, CVMValue& output)
{
	DBG_ASSERT(path.size() > 0);

	// Split field and path.
	String field = "";	
	int dot = path.find('.');
	if (dot != String::npos)
	{
		field = path.substr(0, dot);
		path = path.substr(dot + 1);
	}
	else
	{
		field = path;
		path = "";
	}

	// Array access.
	int arr_start = field.find('[');
	int arr_index = 0;
	bool is_array = false;
	if (arr_start != String::npos)
	{
		int arr_end = field.find(']');
		arr_index = atoi(field.substr(arr_start + 1, arr_end - arr_start).c_str());
		is_array = true;
		field = field.substr(0, arr_start);
	}

	// Grab symbol for field.
	CVMObject* obj = instance.Get();
	CVMLinkedSymbol* obj_sym = obj->Get_Symbol();
	CVMLinkedSymbol* field_sym = NULL;

	for (int i = 0; i < obj_sym->symbol->children_count; i++)
	{
		CVMLinkedSymbol* child = Get_Symbol_Table_Entry(obj_sym->symbol->children_indexes[i]);
		if (strcmp(child->symbol->name, field.c_str()) == 0)
		{
			field_sym = child;
			break;
		}
	}

	DBG_ASSERT(field_sym != NULL);

	// Get the value.
	CVMValue field_value;
	if (field_sym->symbol->type == SymbolType::ClassField)
	{
		if (field_sym->symbol->field_data->is_static)
		{
			Get_Static(obj_sym, field_sym, field_value);
		}
		else
		{
			Get_Field(field_sym, instance, field_value);
		}
	}
	else if (field_sym->symbol->type == SymbolType::ClassProperty)
	{
		if (field_sym->symbol->property_data->get_property_symbol > 0)
		{
			CVMLinkedSymbol* get_method = Get_Symbol_Table_Entry(field_sym->symbol->property_data->get_property_symbol);

			CVMValue obj_instance = instance;
			Invoke(get_method, obj_instance, false, false);
			Get_Return_Value(field_value);
		}
	}
	else
	{
		DBG_ASSERT(false);
	}

	if (is_array)
	{
		CVMObject* array_obj = field_value.object_value.Get();
		if (array_obj != NULL)
		{
			field_value = array_obj->Get_Slot(arr_index);
		}
	}

	// If path is empty get this fields value.
	if (path == "")
	{
		output = field_value;
	}

	// Otherwise get next part.
	else
	{
		Get_Value(field_value.object_value, path, output);
	}
}

void CVirtualMachine::Set_Value(CVMObjectHandle instance, String path, CVMValue& value)
{
	DBG_ASSERT(path.size() > 0);

	// Split field and path.
	String field = "";	
	int dot = path.find('.');
	if (dot != String::npos)
	{
		field = path.substr(0, dot);
		path = path.substr(dot + 1);
	}
	else
	{
		field = path;
		path = "";
	}

	// Array access.
	int arr_start = field.find('[');
	int arr_index = 0;
	bool is_array = false;
	if (arr_start != String::npos)
	{
		int arr_end = field.find(']');
		arr_index = atoi(field.substr(arr_start + 1, arr_end - arr_start).c_str());
		is_array = true;
		field = field.substr(0, arr_start);
	}

	// Grab symbol for field.
	CVMObject* obj = instance.Get();
	CVMLinkedSymbol* obj_sym = obj->Get_Symbol();
	CVMLinkedSymbol* field_sym = NULL;

	for (int i = 0; i < obj_sym->symbol->children_count; i++)
	{ 
		CVMLinkedSymbol* child = Get_Symbol_Table_Entry(obj_sym->symbol->children_indexes[i]);
		if (strcmp(child->symbol->name, field.c_str()) == 0)
		{
			field_sym = child;
			break;
		}
	}

	DBG_ASSERT(field_sym != NULL);


	// If path is empty get this fields value.
	if (path == "")
	{		
		// Get the value.
		if (field_sym->symbol->type == SymbolType::ClassField)
		{
			if (is_array)
			{
				CVMValue array_value;
				Get_Field(field_sym, instance, array_value);

				CVMObject* array_obj = array_value.object_value.Get();
				if (array_obj != NULL)
				{
					array_obj->Get_Slot(arr_index) = value;
				}
			}
			else
			{
				if (field_sym->symbol->field_data->is_static)
				{
					Set_Static(obj_sym, field_sym, value);
				}
				else
				{
					Set_Field(field_sym, instance, value);
				}
			}
		}
		else if (field_sym->symbol->type == SymbolType::ClassProperty)
		{
			if (is_array)
			{
				if (field_sym->symbol->property_data->get_property_symbol > 0)
				{
					CVMLinkedSymbol* get_method = Get_Symbol_Table_Entry(field_sym->symbol->property_data->get_property_symbol);
					if (get_method != NULL)
					{
						CVMValue obj_instance = instance;
						Invoke(get_method, obj_instance, false, false);

						CVMValue retval;
						Get_Return_Value(retval);

						CVMObject* array_obj = retval.object_value.Get();
						if (array_obj != NULL)
						{
							array_obj->Get_Slot(arr_index) = value;
						}
					}
				}
			}
			else
			{
				if (field_sym->symbol->property_data->set_property_symbol > 0)
				{
					CVMLinkedSymbol* set_method = Get_Symbol_Table_Entry(field_sym->symbol->property_data->set_property_symbol);
					Push_Parameter(value);

					CVMValue obj_instance = instance;
					Invoke(set_method, obj_instance, false, false);
				}
			}
		}
		else
		{
			DBG_ASSERT(false);
		}
	}

	// Otherwise get next part.
	else
	{
		// Get the value.
		CVMValue field_value;
		if (field_sym->symbol->type == SymbolType::ClassField)
		{
			if (field_sym->symbol->field_data->is_static)
			{
				Get_Static(obj_sym, field_sym, field_value);
			}
			else
			{
				Get_Field(field_sym, instance, field_value);
			}
		}
		else if (field_sym->symbol->type == SymbolType::ClassProperty)
		{
			if (field_sym->symbol->property_data->get_property_symbol > 0)
			{
				CVMLinkedSymbol* get_method = Get_Symbol_Table_Entry(field_sym->symbol->property_data->get_property_symbol);

				CVMValue obj_instance = instance;
				Invoke(get_method, obj_instance, false, false);

				Get_Return_Value(field_value);
			}
		}
		else
		{
			DBG_ASSERT(false);
		}

		if (is_array)
		{
			CVMObject* array_obj = field_value.object_value.Get();
			if (array_obj != NULL)
			{
				field_value = array_obj->Get_Slot(arr_index);
			}
		}

		Set_Value(field_value.object_value, path, value);
		
		// Set the property to the new value. Silly pass by value ;-;.
		if (!is_array)
		{
			if (field_sym->symbol->type == SymbolType::ClassProperty)/* && 
				field_sym->symbol->property_data->data_type->type != CVMBaseDataType::Array && 
				field_sym->symbol->property_data->data_type->type != CVMBaseDataType::Void)
			{
				bool bContinue = true;

				if (field_sym->symbol->property_data->data_type->type == CVMBaseDataType::Object)
				{
					// TODO: Super shitty plz fix. We need to change vmbinary to include information about types passed by refe
					CVMLinkedSymbol* sym = Get_Symbol_Table_Entry(field_sym->symbol->property_data->data_type->class_index);
					if (sym->symbol->class_data->
				}

				if (bContinue)
				{*/
					if (field_sym->symbol->property_data->set_property_symbol > 0)
					{
						CVMLinkedSymbol* set_method = Get_Symbol_Table_Entry(field_sym->symbol->property_data->set_property_symbol);
						Push_Parameter(field_value);

						CVMValue obj_instance = instance;
						Invoke(set_method, obj_instance, false, false);
					}
				/*}
			}*/
		}
	}
}

void CVirtualMachine::Get_This(CVMValue& output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* activation_context = &thread_data.m_active_context->Activation_Stack.Peek();	
	output = activation_context->This;
}

void CVirtualMachine::Get_Return_Value(CVMValue& output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	output = thread_data.m_native_return_value;
}

void CVirtualMachine::Set_Return_Value(CVMValue& value)
{	
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* activation_context = &thread_data.m_active_context->Activation_Stack.Peek();	

	// Set the result register of the previous activation context.
	if (activation_context->Result_Register >= 0)
	{
		CActivationContext& old_activation_context = thread_data.m_active_context->Activation_Stack[thread_data.m_active_context->Activation_Stack.Size() - 2];
		thread_data.m_active_context->Stack[old_activation_context.Register_Offset + activation_context->Result_Register] = value;
	}
	
	thread_data.m_native_return_value = value;
}

void CVirtualMachine::Get_Parameter(int index, CVMValue* output)
{	
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* ctx = &thread_data.m_active_context->Activation_Stack.Peek();
	*output = thread_data.m_active_context->Stack[ctx->Local_Offset + index];
}

void CVirtualMachine::Get_Parameter(int index, int* output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* ctx = &thread_data.m_active_context->Activation_Stack.Peek();
	*output = thread_data.m_active_context->Stack[ctx->Local_Offset + index].int_value;
}

void CVirtualMachine::Get_Parameter(int index, float* output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* ctx = &thread_data.m_active_context->Activation_Stack.Peek();
	*output = thread_data.m_active_context->Stack[ctx->Local_Offset + index].float_value;
}

void CVirtualMachine::Get_Parameter(int index, CVMString* output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* ctx = &thread_data.m_active_context->Activation_Stack.Peek();
	*output = thread_data.m_active_context->Stack[ctx->Local_Offset + index].string_value;
}

void CVirtualMachine::Get_Parameter(int index, CVMObjectHandle* output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	CActivationContext* ctx = &thread_data.m_active_context->Activation_Stack.Peek();
	*output = thread_data.m_active_context->Stack[ctx->Local_Offset + index].object_value;
}
void CVirtualMachine::Push_Parameter(CVMValue& value)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	if (thread_data.m_active_context->Stack.Available() < 1)
	{
		Assert(false, "Stack overflow.");
	}
	thread_data.m_active_context->Stack.Push(value);
}

void CVirtualMachine::Push_Parameter(int& output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	if (thread_data.m_active_context->Stack.Available() < 1)
	{
		Assert(false, "Stack overflow.");
	}

	CVMValue val(output);
	thread_data.m_active_context->Stack.Push(val);
}

void CVirtualMachine::Push_Parameter(float& output)
{	
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	if (thread_data.m_active_context->Stack.Available() < 1)
	{
		Assert(false, "Stack overflow.");
	}

	CVMValue val(output);
	thread_data.m_active_context->Stack.Push(val);
}

void CVirtualMachine::Push_Parameter(CVMString& output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	if (thread_data.m_active_context->Stack.Available() < 1)
	{
		Assert(false, "Stack overflow.");
	}

	CVMValue val(output);
	thread_data.m_active_context->Stack.Push(val);
}

void CVirtualMachine::Push_Parameter(CVMObjectHandle& output)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	if (thread_data.m_active_context->Stack.Available() < 1)
	{
		Assert(false, "Stack overflow.");
	}

	CVMValue val(output);
	thread_data.m_active_context->Stack.Push(val);
}

void CVirtualMachine::Execute_All_Contexts(float frame_time)
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);

	std::vector<CVMContext*> original_contexts = m_contexts;

	for (std::vector<CVMContext*>::iterator iter = original_contexts.begin(); iter != original_contexts.end(); iter++)
	{
		CVMContext* ctx = *iter;

		// Wake up context?
		if (ctx->Sleeping == true)
		{
			ctx->Wakeup_Time -= frame_time;

			if (ctx->Wakeup_Time <= 0)
			{
				ctx->Sleeping = false;
			}
			else
			{
				continue;
			} 
		}

		// Context actually has anything to execute?
		if (ctx->Activation_Stack.Size() <= 0)
		{
			continue;
		}

		// Finally execute!
		{
			CVMContextLock lock = Set_Context(ctx);

			VM_ERROR_GUARD({
				Execute();
			})
		}
	}

	// Run a GC collect.
	{
		PROFILE_SCOPE("Garbage Collection");
		STATISTICS_TIMED_SCOPE(g_gc_time_time);
		
		// Do not collect garbage if we have pending rpc's, as they are not gc-rooted atm.
		// Instead we defer until the pending rpcs are collected.
		if (m_pending_rpcs.size() <= 0)
		{
			m_gc.Collect();
			m_gc_collect_pending_rpcs = false;
		}
		else
		{
			m_gc_collect_pending_rpcs = true;
		}
	}

#ifdef LOG_EXPENSIVE_FUNCTIONS
	//if (Platform::Get()->Get_Ticks() - m_expensive_log_elapsed >= EXPENSIVE_FUNCTION_LOG_CHECK_INTERVAL)
	//{
		std::vector<CVMExpensiveFunctionLog> logs;

		for (std::vector<CVMLinkedSymbol>::iterator iter = m_symbol_table.begin(); iter != m_symbol_table.end(); iter++)
		{
			CVMLinkedSymbol* sym = &(*iter);
			if (sym->symbol->type == SymbolType::ClassMethod)
			{
				if (sym->inclusive_time >= EXPENSIVE_FUNCTION_LOG_THRESHOLD)
				{
					CVMExpensiveFunctionLog log;
					log.inclusive_time = sym->inclusive_time;
					log.name = std::string(Get_Symbol_Table_Entry(sym->symbol->class_scope_index)->symbol->name) + "::" + std::string(sym->symbol->name);
					logs.push_back(log);
				}

				sym->inclusive_time = 0.0f;
			}
		}		

		if (logs.size() > 0)
		{
			std::sort(logs.begin(), logs.end(), &CVMExpensiveFunctionLog::Sort_Predicate);

			DBG_LOG("==== Expensive Functions (Over %i ms) ====", EXPENSIVE_FUNCTION_LOG_CHECK_INTERVAL);
			for (std::vector<CVMExpensiveFunctionLog>::iterator iter = logs.begin(); iter != logs.end(); iter++)
			{
				CVMExpensiveFunctionLog& log = *iter;
				DBG_LOG("[%.2f ms] %s", log.inclusive_time, log.name.c_str());
			}

		}

		m_expensive_log_elapsed = Platform::Get()->Get_Ticks();
	//}
#endif

#if defined(GC_DUMP_STATS) || defined(GC_TRACK_ALLOCS)
	m_gc.Print_Stats();
#endif
}

void CVirtualMachine::Check_Pending_Collect()
{	
	if (m_gc_collect_pending_rpcs && m_pending_rpcs.size() <= 0)
	{
		m_gc.Collect();
		m_gc_collect_pending_rpcs = false;
	}
}

// =================================================================
//	Execution/dispatch code.
// =================================================================

float CVirtualMachine::Get_Instructions_Per_Second()
{
	float elapsed_time = (float)(Platform::Get()->Get_Ticks() - m_ips_timer);
	u64 elapsed_instructions = m_instruction_counter - m_ips_counter;
	return elapsed_instructions / (elapsed_time / 1000.0f);
}

int CVirtualMachine::Get_Instructions_Delta()
{
	return (int)(m_instruction_counter - m_instruction_delta);
}

void CVirtualMachine::Reset_Instructions_Per_Second()
{
	m_ips_timer = Platform::Get()->Get_Ticks();
	m_ips_counter = m_instruction_counter;
}

void CVirtualMachine::Reset_Instructions_Delta()
{
	m_instruction_delta = m_instruction_counter;
}

void CVirtualMachine::Execute()
{
	VM_GET_THREAD_LOCAL_DATA(this, thread_data);
	DBG_ASSERT(thread_data.m_active_context != NULL);

	u64 start_instruction_counter = m_instruction_counter;

	// TODO:
	//	Increase spatial locality of data by compressing instruction stream, get rid of the variant structs. Don't you want to be friends with the cache? 

	if (thread_data.m_active_context != NULL)
	{
		while (true)
		{
#ifdef VM_ALLOW_MULTITHREADING
			VM_GET_THREAD_LOCAL_DATA(this, thread_data);
			if (thread_data.m_active_context->Activation_Stack.Size() <= 0)
			{
				break;
			}
#endif

			CVMContext* context = thread_data.m_active_context;
			CActivationContext* activation_context = &context->Activation_Stack.Peek();
			//CVMInstruction* instructions = activation_context->Method->symbol->method_data->instructions;
			CVirtualMachine* vm = this;

			u8*& ip = activation_context->IP;
			u8*  ip_base = (u8*)activation_context->Method->symbol->method_data->bytecode;

			int context_register_offset = activation_context->Register_Offset;
			int context_local_offset = activation_context->Local_Offset;

			while (true)
			{

#ifdef VM_LOG_EXECUTION
#ifdef VM_LOG_EXECUTION_FUNCTION
				if (m_log_function_depth > 0)
#endif
				{
kjhgrvjhtreyjtre

					CInstruction decoded;
					char* decode_ptr = (char*)ip;
					decoded.Decode(decode_ptr);
					CInstructionMetaInfo* info = &InstructionMetaInfo[decoded.OpCode];

					std::string args = "";
					for (int i = 0; i < info->Operand_Count; i++)
					{
						std::string arg = "";
						switch (info->Operand_Types[i])
						{
						case OperandType::Float:
							arg = StringHelper::Format("%f", decoded.Operands[i].float_value);
							break;
						case OperandType::Int:
							arg = StringHelper::Format("%i", decoded.Operands[i].int_value);
							break;
						case OperandType::JumpTarget:
							arg = StringHelper::Format("[%i]", decoded.Operands[i].int_value);
							break;
						case OperandType::Register:
							arg = StringHelper::Format("r%i", decoded.Operands[i].int_value);
							break;
						case OperandType::String:
							arg = StringHelper::Format("\"%s\"", Get_String_Table_Entry(decoded.Operands[i].int_value));
							break;
						case OperandType::Symbol:
							arg = StringHelper::Format("'%s'", Get_Symbol_Table_Entry(decoded.Operands[i].int_value)->symbol->name);
							break;
						}

						args = StringHelper::Format("%s %-10s", args.c_str(), arg.c_str());
					}
					DBG_LOG("[%s Stack=%i] [@ %i]: %-20s %s", 
						activation_context->Method->symbol->name, 
						thread_data.m_active_context->Stack.Size(), 
						ip - ip_base, 
						info->Name, 
						args.c_str());	
				}
#endif

				m_instruction_counter++;

				// Infinite loop?
				u64 elapsed = (m_instruction_counter - start_instruction_counter);
				if (elapsed > INFINITE_LOOP_INSTRUCTION_COUNT)
				{
					std::string msg = "[Possible Infinite Loop Detected]\n";

					for (int i = context->Activation_Stack.Size() - 1; i >= 0; i--)
					{
						String func = "";
						int line = 0;

						CActivationContext* callee = &context->Activation_Stack[i];	
						CVMDebugInfo* info = Resolve_Debug_Info(callee);

						func = Platform::Get()->Extract_Filename(Get_String_Table_Entry(info->file_index));
						line = info->row;
						
						msg += StringHelper::Format("\t[%s:%i] %-20s\n", 
							callee->Method->symbol->method_data->is_native == 1 ? "<native>" : func.c_str(), 
							callee->Method->symbol->method_data->is_native == 1 ? 0 : line,
							callee->Method->symbol->name);
					}

					DBG_LOG("%s", msg.c_str());
					start_instruction_counter = m_instruction_counter;
				}

				// Read opcode.
				u8 op_code = CInstruction::Decode_OpCode(ip);

				switch (op_code)
				{
#define INSTRUCTION_FUNCTION(opcode) case InstructionOpCode::opcode:					
#define RETURN_RESULT_CONTEXTCHANGED()											\
					{															\
						if (thread_data.m_active_context->Sleeping == true)		\
							return;												\
						goto restart_context_loop;								\
					}
#define RETURN_RESULT_FINISHED() return;
#define RETURN_RESULT_CONTINUE() break;
#define RETURN_RESULT_JUMPED()   break;

					#include "XScript/VirtualMachine/InstructionFunctions.inc"

#undef RETURN_RESULT_JUMPED
#undef RETURN_RESULT_CONTINUE
#undef RETURN_RESULT_FINISHED
#undef RETURN_RESULT_CONTEXTCHANGED
#undef INSTRUCTION_FUNCTION	

				default:
					{
						vm->Assert(false, "Invalid Instruction 0x%02x", op_code);
						break;
					}
				}
			}

restart_context_loop:	
			;
		}
	}
}
