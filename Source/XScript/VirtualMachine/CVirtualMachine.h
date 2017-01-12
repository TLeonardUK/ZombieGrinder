/* *****************************************************************

		CVirtualMachine.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CVIRTUALMACHINE_H_
#define _CVIRTUALMACHINE_H_

#include "Generic/Types/String.h"
#include <vector>

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"
#include "XScript/VirtualMachine/CVMObject.h"

#include "Generic/Types/HashTable.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector4.h"

#include "Generic/Threads/ThreadLocalData.h"

#include <csetjmp>

class CVMBinary;
struct CVMBinaryData;
struct CVMBinarySymbol;
class CActivationContext;
class CVMValue;
class CVMObject;
class CVMContext;
class Stream;
class Thread;
struct CVMDebugInfo;
class CVMBindingHelper;
class CVMNativeTrampoline;

// =================================================================
//	When you call Set_Context one of these handles will be returned
//	when they go out of scope they reset the context and allow
//	others to change the context. Basically a glorified mutex.
// =================================================================
struct CVMContextLock
{
public:
	CVMContextLock(const CVMContextLock& other);
	~CVMContextLock();

private:
	CVMContextLock(CVirtualMachine* machine, int old_ref_count, CVMContext* old_context);

	friend class CVirtualMachine;
	CVirtualMachine* m_vm;
	int m_old_ref_count;
	CVMContext* m_old_context;
};

// =================================================================
//	A symbol that has been linked to the virtual machine.
// =================================================================
struct CVMLinkedSymbol
{
	CVMBinarySymbol*		symbol;

	CVMValue*				static_data;
	int						static_data_length;

	bool					have_instructions_been_patched;
	bool					have_symbols_been_patched;

	int						index;
	CVMGCRoot				type_instance;

	bool					is_native_bound;
	CVMNativeTrampoline*	native_trampoline;

	bool					can_reference_objects;
	bool					specific_slots_reference_only;
	std::vector<int>		specific_slots_reference_indexes;

	int						name_hash;

	String					event_name;
	int						event_name_hash;

	std::vector<int>		inherited_symbol_indexes;

	double					inclusive_time;
};

// =================================================================
//	A pending RPC event.
// =================================================================
struct CVMRPCEvent
{
	CVMLinkedSymbol* symbol;

	std::vector<CVMValue> parameters;

	int target;
	int except;

	CVMValue instance;
};

struct CVMRPCTarget
{
	enum Type
	{
		All 			= -1,
		Server 			= -2, 
		Local		 	= -3,
		None			= -4
	};
};

struct CVMExpensiveFunctionLog
{
	double		inclusive_time;
	std::string name;

	static bool Sort_Predicate(CVMExpensiveFunctionLog& a, CVMExpensiveFunctionLog& b)
	{
		return a.inclusive_time > b.inclusive_time;
	}
};

// =================================================================
//	The amazing magic virtual machine :3. What actually takes out
//  instructions and makes them do useful things!
// =================================================================
struct CVMThreadLocalData
{
	CVMContext*		m_active_context;

	Thread*			m_thread;

	bool			m_error_recovery_jump_set;
	jmp_buf 		m_error_recovery_jump;

	CVMValue		m_native_return_value;

	int				m_context_ref_count;

	bool			m_error_recovering;

	CVMThreadLocalData()
		: m_active_context(NULL)
		, m_thread(NULL)
		, m_error_recovery_jump_set(false)
		, m_context_ref_count(0)
		, m_error_recovering(false)
	{
	}
};

class CVirtualMachine
{
private:
	std::vector<CVMLinkedSymbol>						m_symbol_table;
	std::vector<CVMLinkedSymbol*>						m_object_referable_symbols;
	HashTable<int, int>									m_symbol_table_lookup;
	std::vector<const char*>							m_string_table;

	CGarbageCollector									m_gc;
	std::vector<CVMContext*>							m_contexts;

	u64													m_instruction_counter;
	u64													m_instruction_delta;
	double												m_ips_timer;
	u64													m_ips_counter;

	int													m_onscreen_message_id;

#ifdef VM_ALLOW_MULTITHREADING
	Mutex*												m_native_global_thread_lock;
#endif

	std::vector<CVMRPCEvent>							m_pending_rpcs;

	CVMContext*											m_static_context;

public:
	CVMLinkedSymbol*									Vec2_Class;
	CVMLinkedSymbol*									Vec3_Class;
	CVMLinkedSymbol*									Vec4_Class;
	CVMLinkedSymbol*									Int_Class;
	CVMLinkedSymbol*									IntArray_Class;
	CVMLinkedSymbol*									Float_Class;
	CVMLinkedSymbol*									FloatArray_Class;
	CVMLinkedSymbol*									String_Class;
	CVMLinkedSymbol*									StringArray_Class;

private:

	// Used for expensive log checking, should ifdef this really.
	double												m_expensive_log_elapsed;

	bool												m_multithreaded;
	Thread*												m_lock_thread;
	int													m_lock_thread_depth;

	friend struct CVMContextLock;
	friend class CVMBindingHelper;

	CVMBindingHelper*									m_binding_helper;

	Mutex*												m_context_mutex;	
//	Mutex*												m_context_wait_mutex;
//	int													m_context_ref_count;
//	Thread*												m_context_lock_thread;

	int													m_log_function_depth;

#ifdef VM_ALLOW_MULTITHREADING
	ThreadLocalData<CVMThreadLocalData*>				m_thread_local_data;
#else
	CVMThreadLocalData									m_thread_local_data;
#endif

	bool												m_profiling_enabled;

	bool												m_gc_collect_pending_rpcs;
	
	// Thread local.

	enum
	{
		gc_heap_size			= 24 * 1024 * 1024,

		activation_stack_size	= 64,
		stack_size				= 5 * activation_stack_size 
	};

	// Linking methods.
	bool Is_Symbol_Same(CVMBinarySymbol* sym1, CVMBinarySymbol* sym2);
	int Add_String_To_Table(const char* str);
	int Add_Symbol_To_Table(CVMBinarySymbol* symbol);
	void Patch_Index_Array(int* src_array, int src_count, std::vector<int> remap);
	void Patch_Symbols(std::vector<int> string_remaps, std::vector<int> symbol_remaps, CVMBinaryData* data);
	void Patch_Instructions(std::vector<int> string_remaps, std::vector<int> symbol_remaps, CVMBinaryData* data);
	void Patch_Data_Type(CVMDataType* dt, std::vector<int> symbol_remaps);

	// Error recovery.
	bool Set_Error_Recovery(); // Returns true if we should execute code we are guarding.
	void Clear_Error_Recovery();

protected:
	friend class CGarbageCollector;

	// Overridden by derived classes, register runtime components
	virtual void Register_Runtime();

public:

	// General execution methods.
	INLINE CVMLinkedSymbol* Get_Symbol_Table_Entry(int index)
	{
		DBG_ASSERT(index >= 0 && index < (int)m_symbol_table.size());
		return &m_symbol_table[index];
	}
	INLINE const char* Get_String_Table_Entry(int index)
	{
		DBG_ASSERT(index >= 0 && index < (int)m_string_table.size());
		return m_string_table[index];
	}
	
	void Dump_Content_State();

	INLINE void Set_MultiThreaded(bool value)
	{
		DBG_ASSERT(m_lock_thread == NULL); // Don't change while something is locked!
		//DBG_LOG("CHANGING MULTITHREAD MDOE TO %i", value);
		m_multithreaded = value;
	}

#ifdef VM_ALLOW_MULTITHREADING
	INLINE Mutex* Get_Global_Mutex()
	{
		return m_native_global_thread_lock;
	}
#endif

	// Internal invokation method.
	// These should be inlined but setjmp prevents us on nix.
	void Invoke_Internal(CVMLinkedSymbol* symbol, CVMValue instance, bool async = false, bool ignore_vftable = false, int return_register = -1, bool ignore_rpc = false);
	bool Return_Internal(CVMValue& result, bool set_native_result = true);

	// Internal binding stuffs.
	void Bind_Method_Internal(const char* class_name, const char* name, CVMNativeTrampoline* trampoline);
	
	// Releasesa a context mutex.
	void Release_Context(int old_ref_count, CVMContext* old_context);

public:

	CVirtualMachine();
	~CVirtualMachine();

	// General statistics.
	float Get_Instructions_Per_Second(); // Instructions per second.
	int   Get_Instructions_Delta();
	void  Reset_Instructions_Per_Second();
	void  Reset_Instructions_Delta();

	// Garbage collector, all script data has to be allocated via this!
	CGarbageCollector* Get_GC();

	// Links all the classes in a binary file into the VM's environment.
	// Deals with de-duping classes shared between binaries and all
	// initial loading and static-execution.
	void Link(CVMBinary* binary);

	// Called after linking all classes. Sets up static allocations / runs constructors.
	void Construct();

	// Error handling.
	void Assert(bool cond);
	void Assert(bool cond, const char* format, ...);

	// Context manipulation.
	CVMContext* New_Context(void* metadata);
	void Free_Context(CVMContext* context);
	CVMContextLock Set_Context(CVMContext* context);
	CVMContext* Get_Active_Context();
	CVMContext* Get_Static_Context();
	CActivationContext* Get_Activation_Context(int offset = 0);
	int Get_Context_Count();

	CVMDebugInfo* Resolve_Debug_Info(CActivationContext* context);

	void Set_Profiling_Enabled(bool bEnabled);

	// Some helper functions.
	CVMObjectHandle Create_Vec2(float x, float y);
	CVMObjectHandle Create_Vec2(Vector2 x);
	CVMObjectHandle Create_Vec3(float x, float y, float z);
	CVMObjectHandle Create_Vec3(Vector3 x);
	CVMObjectHandle Create_Vec4(float x, float y, float z, float w);
	CVMObjectHandle Create_Vec4(Vector4 x);

	// Latent execution.
	void Sleep(float seconds);
	bool On_Stack(CVMLinkedSymbol* func_symbol);

	// Host binding.
	CVMLinkedSymbol* Get_Active_Function();
	CVMLinkedSymbol* Get_Stack_Frame(int offset = 1);

	// Symbol searching.
	CVMLinkedSymbol* Find_Symbol(int unique_id);
	CVMLinkedSymbol* Find_Class(const char* class_name);
	std::vector<CVMLinkedSymbol*> Find_Derived_Classes(CVMLinkedSymbol* base_class);
	CVMLinkedSymbol* Find_Function(CVMLinkedSymbol* class_symbol, const char* function_name, int parameter_count, ...); 
	CVMLinkedSymbol* Find_Variable(CVMLinkedSymbol* class_symbol, const char* variable_name); 

	bool Is_Class_Derived_From(CVMLinkedSymbol* child, CVMLinkedSymbol* ancestor);

	// Event handling.
	void			 Set_Default_State	(CVMObjectHandle obj);
	CVMLinkedSymbol* Get_Current_State	(CVMObjectHandle obj);
	CVMLinkedSymbol* Get_Default_State	(CVMObjectHandle obj);
	void			 Change_State		(CVMObjectHandle obj, CVMLinkedSymbol* new_state);
	void			 Push_State			(CVMObjectHandle obj, CVMLinkedSymbol* new_state);
	void			 Pop_State			(CVMObjectHandle obj);
	CVMLinkedSymbol* Find_State			(CVMObjectHandle obj, const char* state_name); 
	CVMLinkedSymbol* Find_Event			(CVMLinkedSymbol* state_symbol, CVMLinkedSymbol* class_symbol, const char* event_name, int parameter_count, ...); 
	CVMLinkedSymbol* Find_Event			(CVMLinkedSymbol* state_symbol, CVMLinkedSymbol* class_symbol, int event_name_hash, int parameter_count, ...); 

	// Object creation.
	CVMObjectHandle New_Object	(CVMLinkedSymbol* class_type, bool call_constructor = true, void* meta_data = NULL);
	CVMObjectHandle New_Array	(CVMLinkedSymbol* class_type, int size);
	CVMObjectHandle Cast_Object	(CVMObjectHandle obj, CVMLinkedSymbol* class_type);
	
	// RPC Stuff.
	void Create_RPC_Event(CVMLinkedSymbol* symbol, CVMValue& instance);
	bool Get_RPC_Event(CVMRPCEvent& evt);

	// External interaction.
	void	 Invoke(CVMLinkedSymbol* symbol, CVMValue instance, bool async = false, bool ignore_vftable = false, bool ignore_rpc = false);

	void	 Get_Field(CVMLinkedSymbol* field_symbol, CVMObjectHandle instance, CVMValue& output);
	void     Set_Field(CVMLinkedSymbol* field_symbol, CVMObjectHandle instance, CVMValue& value);
	
	void	 Get_Static(CVMLinkedSymbol* class_symbol, CVMLinkedSymbol* field_symbol, CVMValue& output);
	void     Set_Static(CVMLinkedSymbol* class_symbol,CVMLinkedSymbol* field_symbol, CVMValue& value);

	void     Get_Return_Value(CVMValue& output);
	void     Set_Return_Value(CVMValue& value);

	void	 Get_This(CVMValue& output);

	void	 Get_Parameter(int index, CVMValue* output);
	void	 Get_Parameter(int index, int* output);
	void	 Get_Parameter(int index, float* output);
	void	 Get_Parameter(int index, CVMString* output);
	void	 Get_Parameter(int index, CVMObjectHandle* output);

	void	 Push_Parameter(CVMValue& value);
	void	 Push_Parameter(int& output);
	void	 Push_Parameter(float& output);
	void	 Push_Parameter(CVMString& output);
	void	 Push_Parameter(CVMObjectHandle& output);

	// Special functions used to get/set fields of objects using a dot notation based "path string". eg:
	//	Get_Value(base_obj, "Bounding_Box.X");

	void	 Get_Value(CVMObjectHandle instance, String path, CVMValue& output);
	void	 Set_Value(CVMObjectHandle instance, String path, CVMValue& value);

	// Binding of native methods.
	CVMBindingHelper* Get_Bindings()
	{
		return m_binding_helper;
	}

	// Execution.
	virtual void Execute_All_Contexts(float frame_time);
	void Execute();

	void Check_Pending_Collect();

};

#endif