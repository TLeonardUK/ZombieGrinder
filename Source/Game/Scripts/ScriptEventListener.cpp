// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/ScriptEventListener.h"
#include "Game/Scene/Actors/ScriptedActor.h"
#include "Engine/Engine/GameEngine.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Profiling/ProfilingManager.h"

#include <algorithm>
#include <vector>

std::vector<ScriptEventListener*> ScriptEventListener::g_script_event_listeners;

ScriptEventListener::ScriptEventListener(CVMContext* context, CVMObjectHandle object, ScriptedActor* actor)
	: m_context(context)
	, m_object(object)
	, m_last_state_change_counter(-1)
	, m_enabled(true)
	, m_actor(actor)
	, m_manual_tick(false)
{
	static int idCounter = 0;
	m_id = ++idCounter;

	g_script_event_listeners.push_back(this);
	Get_Symbols();
}

ScriptEventListener::~ScriptEventListener()
{
	g_script_event_listeners.erase(std::find(g_script_event_listeners.begin(), g_script_event_listeners.end(), this));
}

void ScriptEventListener::Get_Symbols()
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	
	int state_change_counter = m_object.Get().Get()->Get_State_Change_Counter();
	if (state_change_counter == m_last_state_change_counter)
	{
		return;
	}

	CVMLinkedSymbol* current_state = vm->Get_Current_State(m_object);
	CVMLinkedSymbol* class_symbol = m_object.Get().Get()->Get_Symbol();

	#define SCRIPT_EVENT_0(name)																				m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 0);
	#define SCRIPT_EVENT_1(name, x, native_x)																	m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 1, x);
	#define SCRIPT_EVENT_2(name, x, native_x, y, native_y)														m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 2, x, y);
	#define SCRIPT_EVENT_3(name, x, native_x, y, native_y, z, native_z)											m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 3, x, y, z);
	#define SCRIPT_EVENT_4(name, x, native_x, y, native_y, z, native_z, w, native_w)							m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 4, x, y, z, w);
	#define SCRIPT_EVENT_5(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a)				m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 5, x, y, z, w, a);
	#define SCRIPT_EVENT_6(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b)	m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 6, x, y, z, w, a, b);
	#define SCRIPT_EVENT_7(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c)	m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 7, x, y, z, w, a, b, c);
	#define SCRIPT_EVENT_8(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d)	m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 8, x, y, z, w, a, b, c, d);
	#define SCRIPT_EVENT_9(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e)	m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 9, x, y, z, w, a, b, c, d, e);
	#define SCRIPT_EVENT_10(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e, f, native_f)	m_event_##name = vm->Find_Event(current_state, class_symbol, #name, 10, x, y, z, w, a, b, c, d, e, f);
	#include "Game/Scripts/ScriptEvents.inc"
	#undef SCRIPT_EVENT_0
	#undef SCRIPT_EVENT_1
	#undef SCRIPT_EVENT_2
	#undef SCRIPT_EVENT_3
	#undef SCRIPT_EVENT_4
	#undef SCRIPT_EVENT_5
	#undef SCRIPT_EVENT_6
	#undef SCRIPT_EVENT_7
	#undef SCRIPT_EVENT_8
	#undef SCRIPT_EVENT_9
	#undef SCRIPT_EVENT_10

	m_custom_event_lookup.Clear();

	m_last_state_change_counter = state_change_counter;
}

bool ScriptEventListener::Should_Tick()
{
	if (!m_enabled)
	{
		return false;
	}

	if (m_actor != NULL && !m_actor->Get_Enabled())
	{
		return false;
	}

	if (m_event_On_Tick == NULL)
	{
		if (m_last_state_change_counter == -1)
		{
			Get_Symbols();

			if (m_event_On_Tick == NULL)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

void ScriptEventListener::Fire_Custom(int function_name_hash, CVMObjectHandle param_obj)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMLinkedSymbol* current_state = vm->Get_Current_State(m_object);

	if (!m_enabled)
	{
		return;
	}

	if (m_actor != NULL && !m_actor->Get_Enabled())
	{
		return;
	}

//	if (current_state == NULL)
//	{
//		return;
//	}

	CVMLinkedSymbol* sym = NULL;
	if (!m_custom_event_lookup.Get(function_name_hash, sym))
	{
		sym = vm->Find_Event(current_state, m_object.Get().Get()->Get_Symbol(), function_name_hash, 1, param_obj.Get()->Get_Symbol()->symbol->name);
		m_custom_event_lookup.Set(function_name_hash, sym);
	}

	if (sym != NULL)
	{
		CVMContextLock lock = vm->Set_Context(m_context);

		// Don't fire event if event function is already in the stack,
		// probably because function is latently executing.
		if (vm->On_Stack(sym))
			return;

		{
			PROFILE_SCOPE("Script Event");

			CVMValue param_1 = param_obj;
			CVMValue instance = m_object.Get();
			vm->Push_Parameter(param_1);
			vm->Invoke(sym, instance, false, false, false);
		}
	}
}

void ScriptEventListener::Fire_Global_Custom(int function_name_hash, CVMObjectHandle param_obj)
{
	std::vector<ScriptEventListener*> list = g_script_event_listeners; 
 	for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) 
	{
		(*iter)->Fire_Custom(function_name_hash, param_obj);
	}
}

void ScriptEventListener_Fire0(ScriptEventListener* listener, CVMLinkedSymbol* symbol)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		CVMValue instance = listener->m_object.Get();
		vm->Invoke(symbol, instance, false, false, false);
	}
}

template <typename P1>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{		
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		CVMValue param_1_value = param_1;
		CVMValue instance_value = listener->m_object.Get();

		vm->Push_Parameter(param_1_value);
		vm->Invoke(symbol, instance_value, false, false, false);
	}
}

template <typename P1, typename P2>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		CVMValue param_1_value = param_1;
		CVMValue param_2_value = param_2;
		CVMValue instance_value = listener->m_object.Get();

		vm->Push_Parameter(param_1_value);
		vm->Push_Parameter(param_2_value);
		vm->Invoke(symbol, instance_value, false, false, false);
	}
}

template <typename P1, typename P2, typename P3>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

template <typename P1, typename P2, typename P3, typename P4>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

template <typename P1, typename P2, typename P3, typename P4, typename P5>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4, P5 param_5)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Push_Parameter(param_5);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4, P5 param_5, P6 param_6)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Push_Parameter(param_5);
		vm->Push_Parameter(param_6);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4, P5 param_5, P6 param_6, P7 param_7)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Push_Parameter(param_5);
		vm->Push_Parameter(param_6);
		vm->Push_Parameter(param_7);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4, P5 param_5, P6 param_6, P7 param_7, P8 param_8)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Push_Parameter(param_5);
		vm->Push_Parameter(param_6);
		vm->Push_Parameter(param_7);
		vm->Push_Parameter(param_8);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4, P5 param_5, P6 param_6, P7 param_7, P8 param_8, P9 param_9)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Push_Parameter(param_5);
		vm->Push_Parameter(param_6);
		vm->Push_Parameter(param_7);
		vm->Push_Parameter(param_8);
		vm->Push_Parameter(param_9);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}


template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
void ScriptEventListener_FireN(ScriptEventListener* listener, CVMLinkedSymbol* symbol, P1 param_1, P2 param_2, P3 param_3, P4 param_4, P5 param_5, P6 param_6, P7 param_7, P8 param_8, P9 param_9, P10 param_10)
{
	if (symbol == NULL)
		return;

	if (!listener->m_enabled)
	{
		return;
	}

	if (listener->m_actor != NULL && !listener->m_actor->Get_Enabled())
	{
		return;
	}

	CVirtualMachine* vm = GameEngine::Get()->Get_VM();
	CVMContextLock lock = vm->Set_Context(listener->m_context);

	// Don't fire event if event function is already in the stack,
	// probably because function is latently executing.
	if (vm->On_Stack(symbol))
		return;

	{
		PROFILE_SCOPE_FORMATTED("Script - %s.%s", listener->m_object.Get().Get()->Get_Symbol()->symbol->name, symbol->symbol->name);

		vm->Push_Parameter(param_1);
		vm->Push_Parameter(param_2);
		vm->Push_Parameter(param_3);
		vm->Push_Parameter(param_4);
		vm->Push_Parameter(param_5);
		vm->Push_Parameter(param_6);
		vm->Push_Parameter(param_7);
		vm->Push_Parameter(param_8);
		vm->Push_Parameter(param_9);
		vm->Push_Parameter(param_10);
		vm->Invoke(symbol, listener->m_object.Get(), false, false, false);
	}
}

#define SCRIPT_EVENT_0(name)														\
	void ScriptEventListener::name()																				{ ScriptEventListener_Fire0(this, m_event_##name); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name()																			{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(); } }

#define SCRIPT_EVENT_1(name, x, native_x)											\
	void ScriptEventListener::name(native_x p1)																		{ ScriptEventListener_FireN<native_x>(this, m_event_##name, p1); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1)																{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1); } }

#define SCRIPT_EVENT_2(name, x, native_x, y, native_y)								\
	void ScriptEventListener::name(native_x p1, native_y p2)														{ ScriptEventListener_FireN<native_x,native_y>(this, m_event_##name, p1, p2); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2)													{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2); } }
		
#define SCRIPT_EVENT_3(name, x, native_x, y, native_y, z, native_z)					\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3)											{ ScriptEventListener_FireN<native_x,native_y,native_z>(this, m_event_##name, p1, p2, p3); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3)									{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3); } }

#define SCRIPT_EVENT_4(name, x, native_x, y, native_y, z, native_z, w, native_w)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4)								{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w>(this, m_event_##name, p1, p2, p3, p4); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4)						{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4); } }

#define SCRIPT_EVENT_5(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5)							{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w,native_a>(this, m_event_##name, p1, p2, p3, p4, p5); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5)					{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4, p5); } }

#define SCRIPT_EVENT_6(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6)			{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w,native_a,native_b>(this, m_event_##name, p1, p2, p3, p4, p5, p6); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6)		{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4, p5, p6); } }

#define SCRIPT_EVENT_7(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7)			{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w,native_a,native_b,native_c>(this, m_event_##name, p1, p2, p3, p4, p5, p6, p7); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7)	{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4, p5, p6, p7); } }

#define SCRIPT_EVENT_8(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7, native_d p8)			{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w,native_a,native_b,native_c,native_d>(this, m_event_##name, p1, p2, p3, p4, p5, p6, p7, p8); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7, native_d p8)		{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4, p5, p6, p7, p8); } }

#define SCRIPT_EVENT_9(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7, native_d p8, native_e p9)			{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w,native_a,native_b,native_c,native_d,native_e>(this, m_event_##name, p1, p2, p3, p4, p5, p6, p7, p8, p9); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7, native_d p8, native_e p9)		{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4, p5, p6, p7, p8, p9); } }

#define SCRIPT_EVENT_10(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e, f, native_f)	\
	void ScriptEventListener::name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7, native_d p8, native_e p9, native_f p10)			{ ScriptEventListener_FireN<native_x,native_y,native_z,native_w,native_a,native_b,native_c,native_d,native_e>(this, m_event_##name, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); Get_Symbols(); }	\
	void ScriptEventListener::Fire_##name(native_x p1, native_y p2, native_z p3, native_w p4, native_a p5, native_b p6, native_c p7, native_d p8, native_e p9, native_f p10)		{ std::vector<ScriptEventListener*> list = g_script_event_listeners; for (std::vector<ScriptEventListener*>::iterator iter = list.begin(); iter != list.end(); iter++) { (*iter)->name(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10); } }

#include "Game/Scripts/ScriptEvents.inc"

#undef SCRIPT_EVENT_0
#undef SCRIPT_EVENT_1
#undef SCRIPT_EVENT_2
#undef SCRIPT_EVENT_3
#undef SCRIPT_EVENT_4
#undef SCRIPT_EVENT_5
#undef SCRIPT_EVENT_6
#undef SCRIPT_EVENT_7
#undef SCRIPT_EVENT_8
#undef SCRIPT_EVENT_9
#undef SCRIPT_EVENT_10