// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Helper/StringHelper.h"

#include <algorithm>

int PrintfScopeProfiler::Depth_Counter = 0;

bool Sort_By_Percent(ProfileNode* a, ProfileNode* b)
{
	if (a->Parent == NULL || b->Parent == NULL)
	{
		return false;
	}

	double percent_a = a->Total_Time / a->Parent->Total_Time;
	double percent_b = b->Total_Time / b->Parent->Total_Time;

	return percent_a > percent_b;
}

ScopeProfiler::ScopeProfiler(const char* name, const char* function, const char* file, int line)
	: Name(name)
	, Function(function)
	, File(file)
	, Line(line)
	, Active(false)
	, Budget(0.0f)
#ifdef USE_VS_CONCURRENCY_MARKERS
	, m_span(NULL)
#endif
{
	// VS2012+ Concurrency Profiler Markers
	int depth = ProfilingManager::m_depth.Get();
#ifdef USE_VS_CONCURRENCY_MARKERS
	if (depth < ProfilingManager::max_series_depth)
	{
		m_span = new span(ProfilingManager::m_series[depth], depth, name);
	}
#endif	
	ProfilingManager::m_depth.Set(depth + 1);

	Active = ProfilingManager::Is_Running();
	if (Active == true)
	{
		Start_Time = Platform::Get()->Get_Ticks(); // Keeps overhead of profiler rolled into the times.
		ProfilingManager::Push_Scope(this);
	}
#ifdef PROFILE_TRACK_BUDGET
	else
	{
		Start_Time = Platform::Get()->Get_Ticks(); // Keeps overhead of profiler rolled into the times.
	}
#endif
}

ScopeProfiler::ScopeProfiler(const char* name, const char* function, const char* file, int line, float budget)
	: Name(name)
	, Function(function)
	, File(file)
	, Line(line)
	, Active(false)
	, Budget(budget)
#ifdef USE_VS_CONCURRENCY_MARKERS
	, m_span(NULL)
#endif
{
	// VS2012+ Concurrency Profiler Markers
	int depth = ProfilingManager::m_depth.Get();
#ifdef USE_VS_CONCURRENCY_MARKERS
	if (depth < ProfilingManager::max_series_depth)
	{
		m_span = new span(ProfilingManager::m_series[depth], depth, name);
	}
#endif	
	ProfilingManager::m_depth.Set(depth + 1);

	Active = ProfilingManager::Is_Running();
	if (Active == true)
	{
		Start_Time = Platform::Get()->Get_Ticks(); // Keeps overhead of profiler rolled into the times.
		ProfilingManager::Push_Scope(this);
	}
#ifdef PROFILE_TRACK_BUDGET
	else
	{
		Start_Time = Platform::Get()->Get_Ticks(); // Keeps overhead of profiler rolled into the times.
	}
#endif
}
ScopeProfiler::~ScopeProfiler()
{
#ifdef USE_VS_CONCURRENCY_MARKERS
	SAFE_DELETE(m_span);
#endif

	if (Active == true)
	{
		End_Time = Platform::Get()->Get_Ticks();
		ProfilingManager::Pop_Scope(this);

		Active = false;
	}
#ifdef PROFILE_TRACK_BUDGET
	else
	{
		End_Time = Platform::Get()->Get_Ticks();
	}
#endif

#ifdef PROFILE_TRACK_BUDGET

	if (Budget > 0.0f)
	{
		double elapsed = End_Time - Start_Time;
		if (elapsed > Budget)
		{
			DBG_LOG("[Scope:%s] Exceeded Budget: Target=%.2f, Elapsed=%.2f", Name, Budget, elapsed);
		}
	}

	/*
	if (strcmp(Name, "Main Loop") != 0)
	{
		float elapsed = End_Time - Start_Time;
		if (elapsed > 33.0f)
		{
			DBG_LOG("[Scope:%s] Exceeded Frame Budget: Target=%.2f, Elapsed=%.2f", Name, 33.0f, elapsed);
		}
	}
	*/

#endif

	int depth = ProfilingManager::m_depth.Get();
	ProfilingManager::m_depth.Set(depth - 1);
}

bool					ProfilingManager::m_running			= false;
ThreadLocalData<int>	ProfilingManager::m_depth;
double					ProfilingManager::m_start_time		= 0.0f;
double					ProfilingManager::m_end_time		= 0.0f;
double					ProfilingManager::m_total_time		= 0.0f;
ProfileNode*			ProfilingManager::m_base_root		= NULL;
ProfileNode*			ProfilingManager::m_root			= ProfilingManager::m_base_root;

bool																				ProfilingManager::m_frame_capture_pending		= false;
bool																				ProfilingManager::m_frame_capture_active		= false;
int																					ProfilingManager::m_frame_capture_max_depth		= 8;
int																					ProfilingManager::m_frame_capture_allocator_index = 0;
ThreadLocalData<int>																ProfilingManager::m_frame_capture_depth;
ThreadLocalData<ProfileFrameCaptureNode*>											ProfilingManager::m_frame_capture_root;
ThreadSafeStackAllocator<ProfileFrameCaptureNode, ProfilingManager::frame_capture_max_size>	ProfilingManager::m_frame_capture_allocator[2];

HashTable<char*, int> ProfilingManager::m_static_strings;
Mutex* ProfilingManager::m_static_strings_mutex = Mutex::Create();

#ifdef USE_VS_CONCURRENCY_MARKERS
marker_series ProfilingManager::m_series[max_series_depth] = 
{
	"Depth A",
	"Depth B",
	"Depth C",
	"Depth D",
	"Depth E",
	"Depth F",
	"Depth G",
	"Depth H",
	"Depth I",
	"Depth J",
	"Depth K",
	"Depth L",
	"Depth M",
	"Depth N",
	"Depth O",
	"Depth P"
};
#endif

void ProfilingManager::Dispose_Tree(ProfileNode* tree)
{
	for (std::vector<ProfileNode*>::iterator iter = tree->Children.begin(); iter != tree->Children.end(); iter++)
	{
		ProfileNode* node = *iter;
		Dispose_Tree(node);
	}
	
	SAFE_DELETE(tree);
}

void ProfilingManager::Push_Scope(ScopeProfiler* scope)
{
	if (m_frame_capture_active)
	{
		int depth = m_frame_capture_depth.Get();
		m_frame_capture_depth.Set(depth + 1);
		if (depth <= m_frame_capture_max_depth)
		{
			ProfileFrameCaptureNode* node = m_frame_capture_allocator[m_frame_capture_allocator_index].Alloc();
			node->Complete = false;

			ProfileFrameCaptureNode* root = m_frame_capture_root.Get();
			m_frame_capture_root.Set(node);

			node->Active_Thread = Thread::Get_Current();
			node->Start_Time = scope->Start_Time;
			node->Parent = root;
			node->Depth = depth;
			strncpy(node->Name, scope->Name, 64);
		}
	}
	else
	{
		DBG_ASSERT(m_running);

		ProfileNode* node = new ProfileNode();
		node->Parent = m_root;
		node->Depth  = m_root->Depth + 1; 
		m_root->Children.push_back(node);

		m_root = node;
	}
}

void ProfilingManager::Pop_Scope(ScopeProfiler* scope)
{
	if (m_frame_capture_active)
	{
		int depth = m_frame_capture_depth.Get();
		m_frame_capture_depth.Set(depth - 1);
		if (depth - 1 <= m_frame_capture_max_depth)
		{
			ProfileFrameCaptureNode* root = m_frame_capture_root.Get();
			root->End_Time = scope->End_Time;
			root->Complete = true;
			m_frame_capture_root.Set(root->Parent);
		}
	}
	else
	{
		//DBG_ASSERT(m_running);
		DBG_ASSERT(m_root->Parent != NULL);

		m_root->State = *scope;
		m_root->State.Active = false;

		m_root = m_root->Parent;
	}
}

void ProfilingManager::Init()
{
	m_depth.Set(0);

#ifdef ENABLE_PROFILING
	bool bEnabled = true;
#else
	bool bEnabled = false;
#endif

#ifdef USE_BROFILER_MARKERS
	bool bBrofilerMarkers = true;
#else
	bool bBrofilerMarkers = false;
#endif

#ifdef USE_VS_CONCURRENCY_MARKERS
	bool bVsConcurrencyMarkers = true;
#else
	bool bVsConcurrencyMarkers = false;
#endif

	DBG_LOG("Profiler: bEnabled=%i bVsConcurrencyMarkers=%i bBrofilerMarkers=%i", bEnabled, bBrofilerMarkers, bVsConcurrencyMarkers);
}

void ProfilingManager::Start()
{
	DBG_ASSERT(m_running == false);

	m_running = true;
	m_start_time = Platform::Get()->Get_Ticks();

	if (m_base_root != NULL)
	{
		Dispose_Tree(m_base_root);
	}

	m_base_root = new ProfileNode();
	m_base_root->State.Start_Time = m_start_time;
	m_root = m_base_root;

	DBG_LOG("[Profiling] Starting profiling at time %.2f.", m_start_time);
}

void ProfilingManager::Stop()
{
	DBG_ASSERT(m_running == true);

	m_running = false;
	m_end_time = Platform::Get()->Get_Ticks();
	m_base_root->State.End_Time = m_end_time;

	DBG_LOG("[Profiling] Stopping profiling at time %.2f.", m_end_time);
}

bool ProfilingManager::Is_Running()
{
	return m_running || m_frame_capture_active;
}

void ProfilingManager::New_Frame()
{
	m_frame_capture_allocator_index = 1 - m_frame_capture_allocator_index;
	m_frame_capture_allocator[m_frame_capture_allocator_index].Free();
	m_frame_capture_active = m_frame_capture_pending;
	m_frame_capture_pending = false;

#ifdef USE_BROFILER_MARKERS
	Profiler::NextFrame();
#endif
}

void ProfilingManager::Capture_Frame(int max_depth)
{
	m_frame_capture_pending = true;
	m_frame_capture_max_depth = max_depth;
}

int ProfilingManager::Get_Frame_Capture_Nodes(ProfileFrameCaptureNode*& nodes)
{
	int read_index = 1 - m_frame_capture_allocator_index;
	nodes = m_frame_capture_allocator[read_index].Get_Base();
	return m_frame_capture_allocator[read_index].Get_Count();
}

void ProfilingManager::Dump_Output()
{
	m_total_time = m_end_time - m_start_time; 
	
	DBG_LOG("");
	DBG_LOG("=========== PROFILING STATS =============");
	DBG_LOG("Start Time : %.2f ms", m_start_time);
	DBG_LOG("Finish Time: %.2f ms", m_end_time);
	DBG_LOG("Total Time : %.2f ms", m_total_time);
	DBG_LOG("");
	DBG_LOG("%-45s %-15s %-15s %-15s %-15s %-15s", "SCOPE", "CALLS", "LOCAL-%", "GLOBAL-%", "INCLUSIVE", "EXCLUSIVE");
	DBG_LOG("");

	ProfileNode* compact_root = new ProfileNode();
	compact_root->Parent = NULL;
	compact_root->Depth  = 0;
	
	Compact_Tree(m_base_root, compact_root);

	Dump_Output_Tree(compact_root);

	Dispose_Tree(compact_root);
}

void ProfilingManager::Dump_Output_Tree(ProfileNode* root)
{	
	// We don't want to print out the root node, as it dosen't
	// contain any valid data.
	if (root->Parent != NULL)
	{
		double local_percent = root->Total_Time / root->Parent->Total_Time;
		double global_percent = root->Total_Time / m_total_time;
		std::string name = root->State.Name;

		// TODO: Pretty fucking sure there is a better way to do this
		// the obvious std::string constructor seems to bork. So quick hack to get this running.
		std::string tabs = "";
		for (int i = 1; i < root->Depth; i++)
		{
			tabs += "  ";
		}

		DBG_LOG("%s %-45s %-15i %-15.1f %-15.1f %-15.2f %-15.2f", tabs.c_str(), name.c_str(), root->Calls,  (local_percent * 100.0f), (global_percent * 100.0f), root->Total_Time, root->Total_Time - root->Child_Total_Time );
	}
	
	if (root->Children.size() > 1 && root->Parent != NULL)
	{
//		DBG_LOG("");
	}

	for (std::vector<ProfileNode*>::iterator iter = root->Children.begin(); iter != root->Children.end(); iter++)
	{
		ProfileNode* child = *iter;
		Dump_Output_Tree(child);
	}

	if (root->Children.size() > 1)
	{
//		DBG_LOG("");
	}
}

void ProfilingManager::Compact_Tree(ProfileNode* root, ProfileNode* parent)
{
	double elapsed = (root->State.End_Time - root->State.Start_Time);

	// Increment stats.
	parent->Calls++;
	parent->Total_Time += elapsed;

	if (parent->Parent != NULL)
	{
		parent->Parent->Child_Total_Time += elapsed;
	}

	// Go through all child nodes and remove duplicates.
	for (std::vector<ProfileNode*>::iterator iter = root->Children.begin(); iter != root->Children.end(); iter++)
	{
		ProfileNode* child = *iter;
		bool exists = false;

		for (std::vector<ProfileNode*>::iterator iter2 = parent->Children.begin(); iter2 != parent->Children.end(); iter2++)
		{
			ProfileNode* existing_child = *iter2;
			if (child->Equal_To(existing_child))
			{
				Compact_Tree(child, existing_child);
				exists = true;
			}
		}

		if (exists == false)
		{
			ProfileNode* new_child = new ProfileNode();
			new_child->Parent = parent;
			new_child->State  = child->State;
			new_child->Depth  = child->Depth;

			parent->Children.push_back(new_child);

			Compact_Tree(child, new_child);
		}
	}

	std::sort(parent->Children.begin(), parent->Children.end(), Sort_By_Percent);
}

const char* ProfilingManager::Get_Static_String(const char* val)
{
	MutexLock lock(m_static_strings_mutex);

	int hash = StringHelper::Hash(val);
	char* result = NULL;

	if (m_static_strings.Get(hash, result))
	{
		return result;
	}
	else
	{
		int len = strlen(val);
		char* res = new char[len + 1];
		memcpy(res, val, len + 1);
		m_static_strings.Set(hash, res);
	}

	return result;
}