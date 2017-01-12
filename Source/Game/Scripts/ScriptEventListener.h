// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCRIPT_EVENT_LISTENER_
#define _GAME_SCRIPT_EVENT_LISTENER_

#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Generic/Types/HashTable.h"

struct CVMLinkedSymbol;
class ScriptedActor;

class ScriptEventListener
{
	MEMORY_ALLOCATOR(ScriptEventListener, "Engine");

private:
public:
	#define SCRIPT_EVENT_0(name)																											CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_1(name, x, native_x)																								CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_2(name, x, native_x, y, native_y)																					CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_3(name, x, native_x, y, native_y, z, native_z)																		CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_4(name, x, native_x, y, native_y, z, native_z, w, native_w)														CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_5(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a)											CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_6(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b)								CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_7(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c)					CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_8(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d)	CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_9(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e)	CVMLinkedSymbol* m_event_##name; 
	#define SCRIPT_EVENT_10(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e, f, native_f)	CVMLinkedSymbol* m_event_##name; 
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

public:
	// Dick, these should be private. But we need some templates in the cpp to access them ;_;
	CVMContext* m_context;
	CVMGCRoot m_object;
	ScriptedActor* m_actor;
	int m_last_state_change_counter;
	int m_id;

	bool m_enabled;
	bool m_manual_tick;

	HashTable<CVMLinkedSymbol*, int> m_custom_event_lookup;

	static std::vector<ScriptEventListener*> g_script_event_listeners;

	void Get_Symbols();

public:
	ScriptEventListener(CVMContext* context, CVMObjectHandle object, ScriptedActor* actor = NULL);
	~ScriptEventListener();

	CVMObject* Get_Object()
	{
		return m_object.Get().Get();
	}

	void Set_Enabled(bool value)
	{
		m_enabled = value;
	}

	void Set_Manual_Tick(bool value)
	{
		m_manual_tick = false;
	}

	bool Get_Manual_Tick()
	{
		return m_manual_tick;
	}

	ScriptedActor* Get_Actor()
	{
		return m_actor;
	}

	bool Should_Tick();

	void Fire_Custom(int function_name_hash, CVMObjectHandle param_obj);
	static void Fire_Global_Custom(int function_name_hash, CVMObjectHandle param_obj);

	#define SCRIPT_EVENT_0(name)																										static void Fire_##name();																																					void name();
	#define SCRIPT_EVENT_1(name, x, native_x)																							static void Fire_##name(native_x param_1);																																	void name(native_x param_1);
	#define SCRIPT_EVENT_2(name, x, native_x, y, native_y)																				static void Fire_##name(native_x param_1, native_y param_2);																												void name(native_x param_1, native_y param_2);
	#define SCRIPT_EVENT_3(name, x, native_x, y, native_y, z, native_z)																	static void Fire_##name(native_x param_1, native_y param_2, native_z param_3);																								void name(native_x param_1, native_y param_2, native_z param_3);
	#define SCRIPT_EVENT_4(name, x, native_x, y, native_y, z, native_z, w, native_w)													static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4);																			void name(native_x param_1, native_y param_2, native_z param_3, native_w param_4);
	#define SCRIPT_EVENT_5(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a)										static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5);															void name(native_x param_1, native_y param_2, native_z param_3, native w param_4, native_a param_5);
	#define SCRIPT_EVENT_6(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b)							static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6);										void name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6);
	#define SCRIPT_EVENT_7(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c)				static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7);						void name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7);
	#define SCRIPT_EVENT_8(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d)static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7, native_d param_8);	void name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7, native_d param_8);
	#define SCRIPT_EVENT_9(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e) static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7, native_d param_8, native_e param_9);	void name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7, native_d param_8, native_e param_9);
	#define SCRIPT_EVENT_10(name, x, native_x, y, native_y, z, native_z, w, native_w, a, native_a, b, native_b, c, native_c, d, native_d, e, native_e, f, native_f)	static void Fire_##name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7, native_d param_8, native_e param_9, native_f param_10);	void name(native_x param_1, native_y param_2, native_z param_3, native_w param_4, native_a param_5, native_b param_6, native_c param_7, native_d param_8, native_e param_9, native_f param_10);
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

};

#endif

