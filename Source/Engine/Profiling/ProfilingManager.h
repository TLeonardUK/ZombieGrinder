// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PROFILING_MANAGER__
#define _ENGINE_PROFILING_MANAGER__

#include "Engine/Platform/Platform.h"
#include "Generic/Types/CachedMemoryPool.h"
#include "Generic/Types/StackAllocator.h"
#include "Generic/Threads/ThreadLocalData.h"
#include "Generic/Threads/Thread.h"
#include "Generic/Types/HashTable.h"

#ifdef USE_BROFILER_MARKERS
#define USE_PROFILER 1
#include <Brofiler.h>
#endif

#ifdef USE_VS_CONCURRENCY_MARKERS
#include "cvmarkersobj.h"
#include "cvmarkers.h"
using namespace Concurrency::diagnostic;
#endif

// Use this defines to profile different bits of code.
//
//	void MyFunc()
//	{
//		PROFILE_FUNCTION();
//	}
//

#ifdef ENABLE_PROFILING

#ifdef USE_BROFILER_MARKERS
#define PROFILE_FUNCTION()						BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Red)
#define PROFILE_SCOPE(name)						Profiler::EventDescription* _s_##__LINE__##_description = Profiler::EventDescription::Create(ProfilingManager::Get_Static_String(name), __FILE__, __LINE__, (unsigned long)Profiler::Color::Red); Profiler::Category _s_##__LINE__##_category(*_s_##__LINE__##_description);
#define PROFILE_SCOPE_BUDGETED(name, budget)	Profiler::EventDescription* _s_##__LINE__##_description = Profiler::EventDescription::Create(ProfilingManager::Get_Static_String(name), __FILE__, __LINE__, (unsigned long)Profiler::Color::Red); Profiler::Category _s_##__LINE__##_category(*_s_##__LINE__##_description);
#define PROFILE_SCOPE_FORMATTED(format, ...)	Profiler::EventDescription* _s_##__LINE__##_description = Profiler::EventDescription::Create(ProfilingManager::Get_Static_String(StringHelper::Format(format, ##__VA_ARGS__).c_str()), __FILE__, __LINE__, (unsigned long)Profiler::Color::Red); Profiler::Category _s_##__LINE__##_category(*_s_##__LINE__##_description);
#define PROFILE_THREAD(name)					BROFILER_THREAD(ProfilingManager::Get_Static_String(name))
#else
#define PROFILE_THREAD(name)
#define PROFILE_FUNCTION()						ScopeProfiler _s_##__LINE__##_profiler(__FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define PROFILE_SCOPE(name)						ScopeProfiler _s_##__LINE__##_profiler(name, __FUNCTION__, __FILE__, __LINE__)
#define PROFILE_SCOPE_BUDGETED(name, budget)	ScopeProfiler _s_##__LINE__##_profiler(name, __FUNCTION__, __FILE__, __LINE__)
#define PROFILE_SCOPE_FORMATTED(format, ...)	std::string _s_##__LINE__##_profile_scope = StringHelper::Format(format, ##__VA_ARGS__); ScopeProfiler _s_##__LINE__##_profiler(_s_##__LINE__##_profile_scope.c_str(), __FUNCTION__, __FILE__, __LINE__)
//#define PROFILE_SCOPE_BUDGETED(name, budget)	ScopeProfiler _s_##__LINE__##_profiler(name, __FUNCTION__, __FILE__, __LINE__, budget)
#endif

#else
#define PROFILE_THREAD(name)
#define PROFILE_FUNCTION()			
#define PROFILE_SCOPE(name)			
#define PROFILE_SCOPE_BUDGETED(name, budget)
#define PROFILE_SCOPE_FORMATTED(format, ...)
#endif


//#define PRINTF_PROFILE_SCOPE(name)				PrintfScopeProfiler _s_##__LINE__##_profiler(name)
#define PRINTF_PROFILE_SCOPE(name)

#ifndef MASTER_BUILD
#define PROFILE_TRACK_BUDGET
#endif

struct PrintfScopeProfiler
{
public:
	const char* Name;
	double		Start_Time;
	double		End_Time;
	int			Depth;

	static int Depth_Counter;

public:
	PrintfScopeProfiler(const char* name)
		: Name(name)		
	{
		Start_Time = Platform::Get()->Get_Ticks();
		Depth = Depth_Counter;
		Depth_Counter++;

		std::string padding;
		for (int i = 0; i < Depth; i++)
			padding += " ";
		DBG_LOG("%s[START %s]", padding.c_str(), Name);
	}

	~PrintfScopeProfiler()
	{
		End_Time = Platform::Get()->Get_Ticks();
		Depth_Counter--;

		std::string padding;
		for (int i = 0; i < Depth; i++)
			padding += " ";

		DBG_LOG("%s[END %s] Took %.1f", padding.c_str(), Name, End_Time - Start_Time);
	}

};

struct ScopeProfiler
{
public:
	const char* Name;
	const char* Function;
	const char* File;
	int			Line;

	float		Budget;

	bool		Active;

	double		Start_Time;
	double		End_Time;

#ifdef USE_VS_CONCURRENCY_MARKERS
	span*		m_span;
#endif

public:
	ScopeProfiler() 
		: Start_Time(0)
		, End_Time(0)
		, Active(false)
		, Budget(0.0f)
	{
	}

	ScopeProfiler(const char* name, const char* function, const char* file, int line);
	ScopeProfiler(const char* name, const char* function, const char* file, int line, float budget);
	~ScopeProfiler();

};

class ProfileNode
{
public:
	ProfileNode*				Parent;
	ScopeProfiler				State;
	std::vector<ProfileNode*>	Children;
	int							Depth;

	int							Calls;
	double						Total_Time;
	double						Child_Total_Time;

	ProfileNode()
		: Parent(NULL)
		, Depth(0)
		, Calls(0)
		, Total_Time(0)
		, Child_Total_Time(0)
	{
	}

	bool Equal_To(ProfileNode* other)
	{
		return  Depth == other->Depth && 
				State.File == other->State.File && //stricmp(State.File, other->State.File) == 0 &&		// We assume constant strings for file/name, so pointer comparison should be valid.
				State.Name == other->State.Name && //stricmp(State.Name, other->State.Name) == 0 && 
				State.Line == other->State.Line;
	}
};

struct ProfileFrameCaptureNode
{
	char Name[64];

	double Start_Time;
	double End_Time;

	Thread* Active_Thread;

	bool Complete;
	int Depth;

	ProfileFrameCaptureNode* Parent;
};

class ProfilingManager
{
private:
	enum
	{
		frame_capture_max_size = 10000,  // Maximum number of capture nodes that can be allocated.
		max_frame_capture_child_nodes = 256
	};


private:
	static bool m_running;

	static double m_start_time;
	static double m_end_time;
	static double m_total_time;
	
	static ProfileNode* m_base_root;
	static ProfileNode* m_root;

	static ThreadLocalData<int> m_depth;

	static bool m_frame_capture_pending;
	static bool m_frame_capture_active;
	static int m_frame_capture_max_depth;
	static int m_frame_capture_allocator_index;
	static ThreadLocalData<int> m_frame_capture_depth;
	static ThreadLocalData<ProfileFrameCaptureNode*> m_frame_capture_root;
	static ThreadSafeStackAllocator<ProfileFrameCaptureNode, frame_capture_max_size> m_frame_capture_allocator[2];

	static HashTable<char*, int> m_static_strings;
	static Mutex* m_static_strings_mutex;

protected:
	friend struct ScopeProfiler;
	
#ifdef USE_VS_CONCURRENCY_MARKERS
	enum
	{
		max_series_depth = 16,
	};

	static marker_series m_series[max_series_depth];
#endif

	static void Push_Scope(ScopeProfiler* scope);
	static void Pop_Scope(ScopeProfiler* scope);

	static void Dispose_Tree(ProfileNode* tree);
	static void Compact_Tree(ProfileNode* tree, ProfileNode* parent);

	static void Dump_Output_Tree(ProfileNode* root);

	static int Scope_Depth();

public:
	static void Init();
	static void Start();
	static void Stop();
	static bool Is_Running();
	static void Dump_Output();

	static void New_Frame();

	// Captures a frame of profile blocks, available by calling Get_Frame_Capture;
	static void Capture_Frame(int max_depth);
	static int Get_Frame_Capture_Nodes(ProfileFrameCaptureNode*& nodes);

	static const char* Get_Static_String(const char* val);

};

#endif