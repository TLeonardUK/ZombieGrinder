// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scripts/EngineVirtualMachine.h"
#include "Engine/Scripts/ScriptHandle.h"
#include "Engine/Scripts/Script.h"

#include "Engine/Scripts/Runtime/CRuntime_Canvas.h"
#include "Engine/Scripts/Runtime/CRuntime_Locale.h"
#include "Engine/Scripts/Runtime/CRuntime_Display.h"
#include "Engine/Scripts/Runtime/CRuntime_Audio.h"
#include "Engine/Scripts/Runtime/CRuntime_Audio_Channel.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Stats/Stats.h"

DEFINE_RANGE_STATISTIC		("Memory/GC Usage (MB)",					float,					g_stat_gc_usage,							true);
DEFINE_AVERAGE_STATISTIC	("Memory/GC Alloc Per Frame (KB)",			float,			60,		g_stat_gc_alloc_per_frame,					true);
DEFINE_GLOBAL_STATISTIC		("Scripts/Context Count",					int,					g_stat_script_context_count,				true);
DEFINE_AVERAGE_STATISTIC	("Scripts/Instructions Per Second (K)",		float,			60,		g_stat_script_instructions_per_second,		true);
DEFINE_AVERAGE_STATISTIC	("Scripts/Instructions Per Frame (K)",		float,			60,		g_stat_script_instructions_per_frame,		true);

void EngineVirtualMachine::Register_Runtime()
{
	// Register base types.
	CVirtualMachine::Register_Runtime();

	// Register engine types.
	CRuntime_Canvas::Bind(this);
	CRuntime_Locale::Bind(this);
	CRuntime_Display::Bind(this);
	CRuntime_Audio::Bind(this);
	CRuntime_Audio_Channel::Bind(this);
}

EngineVirtualMachine::EngineVirtualMachine()
	: CVirtualMachine()
	, m_locked(false)
	, m_register_progress(0.0f)
{
	m_link_mutex = Mutex::Create();
}

EngineVirtualMachine::~EngineVirtualMachine()
{
	SAFE_DELETE(m_link_mutex);
}

void EngineVirtualMachine::Lock()
{
	DBG_ASSERT(!m_locked);
	m_locked = true;
}

void EngineVirtualMachine::Unlock()
{
	DBG_ASSERT(m_locked);
	m_locked = false;
}

/*
void EngineVirtualMachine::Link_Script(CVMBinary* binary)
{
	MutexLock lock(m_link_mutex);
	Link(binary);
}
*/

struct SortedScript
{
	Script* File;
	std::string Name;

	static bool SortPredicate(SortedScript a, SortedScript b)
	{
		if (a.Name == "__base_linked.xs")
		{
			return true;
		}
		else
		{
			return a.Name < b.Name;
		}
	}

};

void EngineVirtualMachine::Register_Scripts()
{
	double start = Platform::Get()->Get_Ticks();
	DBG_LOG("Registering scripts ...");

	// Link all scripts.
	ThreadSafeHashTable<ScriptHandle*, unsigned int>& files = ResourceFactory::Get()->Get_Scripts();
	int index = 0;

	std::vector<SortedScript> scripts;
	for (ThreadSafeHashTable<ScriptHandle*, unsigned int>::Iterator iter = files.Begin(); iter != files.End(); iter++)
	{
		ScriptHandle* handle = *iter;
		Script* file = handle->Get();

		SortedScript script;
		script.File = file;
		script.Name = StringHelper::Lowercase(Platform::Get()->Extract_Filename(file->Get_Name()).c_str());
		scripts.push_back(script);
	}

	// Need to ensure load order is the same on all clients.
	std::sort(scripts.begin(), scripts.end(), SortedScript::SortPredicate);

	for (std::vector<SortedScript>::iterator iter = scripts.begin(); iter != scripts.end(); iter++, index++)
	{
		SortedScript& script = *iter;
		Link(script.File->Get_Binary());	
		m_register_progress = (float)index / (float)files.Size();
	}

	// Finalize construction.
	Construct();

	m_register_progress = 1.0f;
	
	DBG_LOG("Registered in %f ms", Platform::Get()->Get_Ticks() - start);
}

void EngineVirtualMachine::Execute_All_Contexts(float time)
{
	if (!m_locked)
	{
		CVirtualMachine::Execute_All_Contexts(time);
	}

	Update_Statistics();
}

void EngineVirtualMachine::Update_Statistics()
{
	CGarbageCollector* gc = Get_GC();

	static int m_last_global_allocation = 0;
	int global_allocs = gc->Get_Lifetime_Bytes_Allocated();

	g_stat_gc_usage.Set((float)gc->Get_Total_Used() / 1024.0f / 1024.0f, (float)gc->Get_Total() / 1024.0f / 1024.0f);
	g_stat_gc_alloc_per_frame.Set((float)(global_allocs - m_last_global_allocation) / 1024.0f);
	g_stat_script_context_count.Set(Get_Context_Count());
	g_stat_script_instructions_per_second.Set((float)Get_Instructions_Per_Second() / 1000.0f);
	g_stat_script_instructions_per_frame.Set((float)Get_Instructions_Delta() / 1000.0f);

	Reset_Instructions_Delta();
	Reset_Instructions_Per_Second();

	m_last_global_allocation = global_allocs;
}
