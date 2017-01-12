/* *****************************************************************

		CGarbageCollector.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "XScript/VirtualMachine/CGarbageCollector.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Tasks/TaskManager.h"
#include "Engine/Tasks/GroupTask.h"
#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Math/Math.h"

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
CVMGCRoot* CVMGCRoot::m_global_list = NULL;
int CVMGCRoot::m_global_list_size = 0;
#else
CVMGCRoot** CVMGCRoot::m_global_list = NULL;
int CVMGCRoot::m_global_list_size = 0;
int CVMGCRoot::m_global_list_capacity = 0;
#endif

void CVMGCRoot::Set_Handle(const CVMObjectHandle& other)
{
	if (m_handle.Get() != NULL &&
		m_handle_marked_uncollectable)
	{
		m_handle.Get()->Mark_Collectable();
		m_handle_marked_uncollectable = false;
	}

	m_handle = other;

	if (m_handle.Get() != NULL &&
		!m_handle_marked_uncollectable &&
		!m_handle.Get()->m_class->can_reference_objects)
	{
		m_handle.Get()->Mark_Uncollectable();
		m_handle_marked_uncollectable = true;
	}
	
	if (m_handle.Get() == NULL || m_handle_marked_uncollectable)
	{
		if (m_in_list)
		{
			Remove_Link();
		}
	}
	else
	{
		if (!m_in_list)
		{
			Add_Link();
		}
	}
}

CGarbageCollector::CGarbageCollector(int size)
	: m_vm(NULL)
	, m_traversed_objects(0)
#ifndef GC_USE_MALLOC
	, m_heap_small(size / GC_SMALL_ALLOC_POOL_DIVISOR)
	, m_heap_large(size - (size / GC_SMALL_ALLOC_POOL_DIVISOR))
#else
	, m_total_allocated(0)
	, m_lifetime_bytes_allocated(0)
#endif
{
}

CGarbageCollector::~CGarbageCollector()
{
}

void CGarbageCollector::Init(CVirtualMachine* vm)
{
	m_vm = vm;

	for (int i = 0; i < CACHE_LEVELS; i++)
	{
		Cache& cache = m_cache[i];
		cache.entry_count = 0; 
		cache.entry_size = ((i + 1) * CACHE_LEVEL_INTERVAL);
	}

	m_max_cache_size = (CACHE_LEVEL_INTERVAL * CACHE_LEVELS);
}

// Allocates data from the cache if possible, otherwise from heap.
void* CGarbageCollector::Alloc_Cached(int size)
{
	if (size > m_max_cache_size)
	{
#ifndef GC_USE_MALLOC
		return Get_Heap(size).Alloc(size);
#else
		return HeaplessAlloc(size);
#endif
	}

	int level = ((size - 1) / CACHE_LEVEL_INTERVAL);
	Cache& cache = m_cache[level];

	if (cache.entry_count <= 0)
	{
#ifndef GC_USE_MALLOC
		return Get_Heap(size).Alloc(cache.entry_size);
#else
		return HeaplessAlloc(cache.entry_size);
#endif
	}
	else
	{
		return cache.entries[--cache.entry_count];
	}
}

void CGarbageCollector::Free_Cached(void* ptr)
{
#ifndef GC_USE_MALLOC
	int size = Get_Heap(ptr).Get_Block_Size(ptr);
#else
	int size = HeaplessSize(ptr);
#endif

	// To large, isn't from cache.
	if (size > m_max_cache_size)
	{
#ifndef GC_USE_MALLOC
		Get_Heap(ptr).Free(ptr);
#else
		HeaplessFree(ptr);
#endif
	}

	// Otherwise add back to cache.
	else
	{
		int level = ((size - 1) / CACHE_LEVEL_INTERVAL);
		Cache& cache = m_cache[level];
		if (cache.entry_count >= CACHE_ENTRIES)
		{
#ifndef GC_USE_MALLOC
			Get_Heap(ptr).Free(ptr);
#else
			HeaplessFree(ptr);
#endif
			return;
		}

		DBG_ASSERT_STR(cache.entry_size == size, "Attempting to put entry of size %i into cache of size %i.", size, cache.entry_size);

		cache.entries[cache.entry_count++] = ptr;
	}
}

void CGarbageCollector::Reintegrate_Cache()
{	
	// Take all the cached allocations and give them back to the main heap.

	for (int i = 0; i < CACHE_LEVELS; i++)
	{
		Cache& cache = m_cache[i];
		while (cache.entry_count > 0)
		{
			void* result = cache.entries[--cache.entry_count];

#ifndef GC_USE_MALLOC
			Get_Heap(result).Free(result);
#else
			HeaplessFree(result);
#endif
		}
	}
}

struct GCCounter
{
	CVMLinkedSymbol* symbol;
	int count;
	int size;

	static bool Sort_By_Size(const GCCounter& a, const GCCounter& b)
	{
		return (a.size < b.size);
	}
};


void CGarbageCollector::Dump_Object_State()
{
	std::vector<GCCounter> counter_list;

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
	CVMObject* obj = CVMObject::m_global_list;
	while (obj != NULL)
	{
#else
	for (int i = 0; i < CVMObject::m_collectable_list_size; i++)
	{
		CVMObject* obj = CVMObject::m_collectable_list[i];
#endif

		bool bFound = false;

#ifndef GC_USE_MALLOC
		int size = Get_Heap(obj).Get_Block_Size(obj);
		if (obj->m_data != NULL)
		{
			size += Get_Heap(obj->m_data).Get_Block_Size(obj->m_data);
		}
#else
		int size = HeaplessSize(obj);
		if (obj->m_data != NULL)
		{
			size += HeaplessSize(obj->m_data);
		}
#endif

		for (std::vector<GCCounter>::iterator iter = counter_list.begin(); iter != counter_list.end(); iter++)
		{
			GCCounter& counter = *iter;

			if (counter.symbol == obj->Get_Symbol())
			{
				counter.count++;
				counter.size += size;
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			GCCounter c = GCCounter();
			c.count = 1;
			c.size = size;
			c.symbol = obj->Get_Symbol();
			counter_list.push_back(c);
		}

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
		obj = obj->m_next;
#endif
	}

	std::sort(counter_list.begin(), counter_list.end(), &GCCounter::Sort_By_Size);

	DBG_LOG("==========================================================================================================");
	for (std::vector<GCCounter>::iterator iter = counter_list.begin(); iter != counter_list.end(); iter++)
	{
		GCCounter& counter = *iter;
		DBG_LOG("[%s] count=%i size=%i kb", counter.symbol->symbol->name, counter.count, counter.size / 1024);
	}
	DBG_LOG("==========================================================================================================");
}

void* CGarbageCollector::Alloc(int size)
{
	void* ptr = Alloc_Cached(size);

	// First, fail, we attempt a collection to make room.
	// Actually NO! Don't do this; if we do stuff like this we are fucked:
	//	obj = gc.New();
	//  dosomething
	//  root = obj;
	//
	if (ptr == NULL)
	{
#ifndef GC_USE_MALLOC
		DBG_LOG("GC allocation attempt failed, attempting to reintegrate cache.");
		Reintegrate_Cache();

		ptr = Alloc_Cached(size);
		if (ptr == NULL)
		{
#endif
			DBG_LOG("GC allocation attempt failed, attempting to perform full recollect.");
	
			Collect(); /// TODO: Fix this, this is going to cause us pain (RPC's, stuff like above, etc) ...		
			ptr = Alloc_Cached(size);

#ifndef GC_USE_MALLOC
		}
#endif
	}

	if (ptr == NULL)
	{
		Dump_Object_State();
	}
	DBG_ASSERT_STR(ptr != NULL, "Script GC heap ran out of memory (Used:%.1f mb/%.1f mb Alloc:%iB).", Get_Total_Used() / 1024.0f / 1024.0f, Get_Total() / 1024.0f / 1024.0f, size);

/*	int used = Get_Total_Used();
	if (used > 11.0f * 1024.0f * 1024.0f || size > 1024 * 1024)
	{
		DBG_LOG("Usage now up to %.1f MB, allocated %i B.", used / 1024.0f / 1024.0f, size);
	}
*/

	return ptr;
}

void CGarbageCollector::Free(void* ptr)
{	
	Free_Cached(ptr);
}

#ifndef GC_USE_MALLOC

MemoryHeap& CGarbageCollector::Get_Heap(void* ptr)
{
	if (m_heap_small.Contains(ptr))
	{
		return m_heap_small;
	}
	else
	{
		return m_heap_large;
	}
}

MemoryHeap& CGarbageCollector::Get_Heap(int size)
{
	if (size <= GC_SMALL_ALLOC_THRESHOLD)
	{
		return m_heap_small;
	}
	else
	{
		return m_heap_large;
	}
}

int CGarbageCollector::Get_Total()
{
	return m_heap_small.Get_Total() + m_heap_large.Get_Total();
}

int CGarbageCollector::Get_Total_Used()
{
	return m_heap_small.Get_Total_Used() + m_heap_large.Get_Total_Used();
}

int CGarbageCollector::Get_Lifetime_Bytes_Allocated()
{
	return m_heap_small.Get_Lifetime_Bytes_Allocated() + m_heap_large.Get_Lifetime_Bytes_Allocated();
}

#else

void* CGarbageCollector::HeaplessAlloc(int size)
{
	char* ptr = (char*)malloc(size + sizeof(int));
	*reinterpret_cast<int*>(ptr) = size;
	m_lifetime_bytes_allocated += size;
	m_total_allocated += size;
	return ptr + sizeof(int);
}

void CGarbageCollector::HeaplessFree(void* ptr)
{
	m_total_allocated -= HeaplessSize(ptr);
	free((char*)ptr - sizeof(int));
}

int CGarbageCollector::HeaplessSize(void* ptr)
{
	return *reinterpret_cast<int*>((char*)ptr - sizeof(int));
}

int CGarbageCollector::Get_Total()
{
	return m_total_allocated;
}

int CGarbageCollector::Get_Total_Used()
{
	return m_total_allocated;
}

int CGarbageCollector::Get_Lifetime_Bytes_Allocated()
{
	return m_lifetime_bytes_allocated;
}
#endif

void CGarbageCollector::Traverse(CVMObject* object)
{
	// Already traversed? Skip, don't want any cyclic loops.
	if (object->m_gc_mark)
	{
		return;
	}

	m_traverse_count++;
	m_traversed_objects++;

	object->m_gc_mark = true;

	CVMLinkedSymbol* objClass = object->m_class;

	if (objClass->can_reference_objects)
	{
		/*
		if (objClass->specific_slots_reference_only)
		{
			for (unsigned int i = 0; i < objClass->specific_slots_reference_indexes.size(); i++)
			{
				int idx = objClass->specific_slots_reference_indexes[i];

				CVMObject* sub = object->m_data[idx].object_value.Get();
				if (sub != NULL && !sub->m_gc_mark)
				{
					Traverse(sub);
				}
			}
		}
		else
		{*/
			for (int i = 0; i < object->m_data_size; i++)
			{
				CVMObject* sub = object->m_data[i].object_value.Get();
				if (sub != NULL && !sub->m_gc_mark)
				{
					Traverse(sub);
				}
			}
		//}
	}
	else
	{
		m_skipped_traversals++;
	}
}

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
void CVMGCRoot::Validate_Chain()
{
	CVMGCRoot* root = CVMGCRoot::m_global_list;
	while (root != NULL)
	{
		DBG_ASSERT(reinterpret_cast<u32>(root->m_next) != 0xFEEEFEEE);
		root = root->m_next;
	}
}

bool CVMGCRoot::In_Chain(CVMGCRoot* val)
{
	CVMGCRoot* root = CVMGCRoot::m_global_list;
	while (root != NULL)
	{
		if (root == val)
		{
			return true;
		}
		root = root->m_next;
	}

	return false;
}
#endif

//#define GC_TIMED_LOGS
//#define GC_DUMP_ROOT_OBJECT_TYPES
//#define GC_DUMP_GARBAGE_OBJECT_TYPES

struct GCObjectType
{
	CVMLinkedSymbol* symbol;
	int count;
	bool can_ref;
};

class GCTraversalTask : public Task
{
public:
	CGarbageCollector* collector;

	int start_index;
	int end_index;

	void Run()
	{
		PROFILE_SCOPE("GC Traversal Task");
		collector->Traverse_Roots_In_Range(start_index, end_index);
	}
};

void CGarbageCollector::Traverse_Roots_In_Range(int start_index, int end_index)
{
	// Ideally as this is being called in parallel we should do an atomic set/get on 
	// the gc mark. But that adds a fairly significant overhead, and at best only
	// prevents a small amount of unneccessary duplicated traversal.

	for (int i = start_index; i <= end_index; i++)
	{
		CVMGCRoot* root = CVMGCRoot::m_global_list[i];
		CVMObject* handle = root->GetRef().Get();
		if (handle != NULL && !handle->m_gc_mark)
		{
			Traverse(handle);
		}
	}
}

void CGarbageCollector::Collect()
{
	m_traversed_objects = 0;

#ifdef GC_TIMED_LOGS
	double full_start_time = Platform::Get()->Get_Ticks();
	double start_time = Platform::Get()->Get_Ticks();
#endif

	{
		PROFILE_SCOPE("Root Traversal");

		// Traverse all roots.
		m_traverse_count = 0;
		m_skipped_traversals = 0;

		TaskManager* taskManager = TaskManager::Get();

		GCTraversalTask tasks[8];		
		GroupTask groupTask("GC Root Traversal");
		
		int traversalTaskCount = Min(8, taskManager->Get_Worker_Count() + 1);

		TaskID groupTaskId = taskManager->Add_Task(&groupTask);

		int totalRoots = CVMGCRoot::m_global_list_size;
		int rootsPerTask = totalRoots / traversalTaskCount;
		int start_index = 0;

		for (int i = 0; i < traversalTaskCount; i++)
		{
			tasks[i].collector = this;
			if (i != traversalTaskCount - 1)
			{
				tasks[i].start_index = start_index;
				tasks[i].end_index = start_index + rootsPerTask;
			}
			else
			{
				tasks[i].start_index = start_index;
				tasks[i].end_index = totalRoots - 1;
			}
			start_index = tasks[i].end_index + 1;

			TaskID taskId = taskManager->Add_Task(&tasks[i], groupTaskId);
			taskManager->Queue_Task(taskId);
		}


		taskManager->Queue_Task(groupTaskId);
		taskManager->Wait_For(groupTaskId);

/*
#ifdef GC_DUMP_ROOT_OBJECT_TYPES
		std::vector<GCObjectType> root_object_types;
#endif

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
		CVMGCRoot* root = CVMGCRoot::m_global_list;
		while (root != NULL)
#else
		for (int i = 0; i < CVMGCRoot::m_global_list_size; i++)
#endif
		{
			CVMGCRoot* root = CVMGCRoot::m_global_list[i];
			CVMObject* handle = root->GetRef().Get();
			if (handle != NULL && !handle->m_gc_mark)
			{
				Traverse(handle);
			}

#ifdef GC_DUMP_ROOT_OBJECT_TYPES
			bool found_object_type = false;
			for (unsigned int i = 0; i < root_object_types.size(); i++)
			{
				GCObjectType& type = root_object_types[i];
				if ((type.symbol == NULL && handle == NULL) ||
					(handle != NULL && type.symbol == handle->Get_Symbol()))
				{
					type.count++;
					found_object_type = true;
					break;
				}
			}

			if (!found_object_type)
			{
				GCObjectType type;
				type.symbol = handle == NULL ? NULL : handle->Get_Symbol();
				type.count = 1;
				type.can_ref = handle == NULL ? false : handle->m_class->can_reference_objects;
				root_object_types.push_back(type);
			}
#endif

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
			root = root->m_next;
#endif
		}

#ifdef GC_DUMP_ROOT_OBJECT_TYPES
		DBG_LOG("=== %i Roots ===", CVMGCRoot::m_global_list_size);
		for (unsigned int i = 0; i < root_object_types.size(); i++)
		{
			GCObjectType& type = root_object_types[i];
			DBG_LOG("[%i] Symbol=%s Count=%i CanRef=%i", i, type.symbol == NULL ? "NULL" : type.symbol->symbol->name, type.count, type.can_ref);
		}
#endif
*/

		//DBG_LOG("root_count=%i traverse_count=%i skipped=%i", CVMGCRoot::m_global_list_size, m_traverse_count, m_skipped_traversals);
	}

#ifdef GC_TIMED_LOGS
	double root_traversal_time = Platform::Get()->Get_Ticks() - start_time;
	start_time = Platform::Get()->Get_Ticks();
#endif

	{
		PROFILE_SCOPE("Static Traversal");

		// Traverse all static variables, as they are implicit gc roots.
		for (std::vector<CVMLinkedSymbol*>::iterator iter = m_vm->m_object_referable_symbols.begin(); iter != m_vm->m_object_referable_symbols.end(); iter++)
		{
			CVMLinkedSymbol& symbol = *(*iter);
			if (symbol.static_data_length > 0 && symbol.can_reference_objects && symbol.symbol->type == SymbolType::Class)
			{
				for (int i = 0; i < symbol.static_data_length; i++)
				{
					CVMObject* obj = symbol.static_data[i].object_value.Get();
					if (obj != NULL)
					{
						Traverse(obj);
					}
				}
			}
		}
	}

#ifdef GC_TIMED_LOGS
	double static_traversal_time = Platform::Get()->Get_Ticks() - start_time;
	start_time = Platform::Get()->Get_Ticks();
#endif

	{
		PROFILE_SCOPE("Callstack Traversal");

		// Traverse all call-stacks of active contexts, more implicit roots.
		for (std::vector<CVMContext*>::iterator iter = m_vm->m_contexts.begin(); iter != m_vm->m_contexts.end(); iter++)
		{
			CVMContext* context = *iter;

			// "this" value for all activation indexes.
			for (int i = 0; i < context->Activation_Stack.Size(); i++)
			{
				CActivationContext& ctx = context->Activation_Stack[i];
				CVMObject* obj = ctx.This.object_value.Get();
				if (obj != NULL)
				{
					Traverse(obj);
				}
			}

			// actual stack for all of context.
			for (int i = 0; i < context->Stack.Size(); i++)
			{
				CVMObject* obj = context->Stack[i].object_value.Get();
				if (obj != NULL)
				{
					Traverse(obj);
				}
			}
		}
	}

#ifdef GC_TIMED_LOGS
	double context_traversal_time = Platform::Get()->Get_Ticks() - start_time;
	start_time = Platform::Get()->Get_Ticks();
#endif

	// Collect anything not marked.
	int total_non_garbage = 0;
	int total_garbage = 0;

	{
		PROFILE_SCOPE("Collection");

#ifdef GC_DUMP_GARBAGE_OBJECT_TYPES
		std::vector<GCObjectType> collection_object_types;
#endif

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
		CVMObject* obj = CVMObject::m_global_list;
		while (obj != NULL)
		{
#else
		for (int i = 0; i < CVMObject::m_collectable_list_size; i++)
		{
			CVMObject* obj = CVMObject::m_collectable_list[i];
#endif
			if (!obj->m_gc_mark)
			{
#ifdef GC_DUMP_GARBAGE_OBJECT_TYPES
				bool found_object_type = false;
				for (unsigned int i = 0; i < collection_object_types.size(); i++)
				{
					GCObjectType& type = collection_object_types[i];
					if ((type.symbol == NULL && obj == NULL) ||
						(obj != NULL && type.symbol == obj->Get_Symbol()))
					{
						type.count++;
						found_object_type = true;
						break;
					}
				}

				if (!found_object_type)
				{
					GCObjectType type;
					type.symbol = obj == NULL ? NULL : obj->Get_Symbol();
					type.count = 1;
					type.can_ref = obj == NULL ? false : obj->m_class->can_reference_objects;
					collection_object_types.push_back(type);
				}
#endif

				DBG_ASSERT(obj->m_uncollectable <= 0);

				obj->~CVMObject();
				Free(obj);
				
				total_garbage++;

#ifdef GC_USE_ARRAY_GLOBAL_LISTS
				// This index will have been replaced by our removal, so shunt back.
				i--;
#endif
			}
			else
			{
				obj->m_gc_mark = false;
				total_non_garbage++;
			}


#ifndef GC_USE_ARRAY_GLOBAL_LISTS
			obj = next;
#endif
		}

#ifdef GC_DUMP_GARBAGE_OBJECT_TYPES
		DBG_LOG("=== %i Garbage ===", total_garbage);
		for (unsigned int i = 0; i < collection_object_types.size(); i++)
		{
			GCObjectType& type = collection_object_types[i];
			DBG_LOG("[%i] Symbol=%s Count=%i CanRef=%i", i, type.symbol == NULL ? "NULL" : type.symbol->symbol->name, type.count, type.can_ref);
		}
#endif
	}

#ifdef GC_TIMED_LOGS
	double collect_time = Platform::Get()->Get_Ticks() - start_time;
	start_time = Platform::Get()->Get_Ticks();

	double total_time = Platform::Get()->Get_Ticks() - full_start_time;

	static double avg_total_time = 0.0f;
	static int avg_counter = 0;
	avg_total_time += total_time;
	avg_counter++;

	double avg = avg_total_time / avg_counter;

	DBG_LOG("[Garbage Collection] total=%.2f (avg=%.2f) root=%.2f static=%.2f context=%.2f collect=%.2f traversed=%i total_garbage=%i total_non_garbage=%i", 
		total_time,
		avg,
		root_traversal_time, 
		static_traversal_time,
		context_traversal_time,
		collect_time,
		m_traversed_objects,
		total_garbage,
		total_non_garbage);
#endif

			/*
	/*
	float total_time = Platform::Get()->Get_Ticks() - full_start_time;
	DBG_LOG("[Garbage Collection] total=%.2f traversed=%i removed=%i/%i", 
		total_time,
		m_traversed_objects, 
		total_garbage, 
		total_non_garbage);
	*/

	/*
	static int counter = 0;
	if ((counter++ % 1000) == 0)
	{
		std::vector<GCCounter> counter_list;

		obj = CVMObject::m_global_list;
		while (obj != NULL)
		{
			bool bFound = false;

			int size = m_heap.Get_Block_Size(obj);
			if (obj->m_data != NULL)
			{
				size += m_heap.Get_Block_Size(obj->m_data);
			}

			for (std::vector<GCCounter>::iterator iter = counter_list.begin(); iter != counter_list.end(); iter++)
			{
				GCCounter& counter = *iter;

				if (counter.symbol == obj->Get_Symbol())
				{
					counter.count++;
					counter.size += size;
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				GCCounter c = GCCounter();
				c.count = 1;
				c.size += size;
				c.symbol = obj->Get_Symbol();
				counter_list.push_back(c);
			}

			obj = obj->m_next;
		}

		std::sort(counter_list.begin(), counter_list.end(), &GCCounter::Sort_By_Size);

		DBG_LOG("==========================================================================================================");
		for (std::vector<GCCounter>::iterator iter = counter_list.begin(); iter != counter_list.end(); iter++)
		{
			GCCounter& counter = *iter;
			DBG_LOG("[%s] count=%i size=%i kb", counter.symbol->symbol->name, counter.count, counter.size / 1024);
		}
		DBG_LOG("==========================================================================================================");
	}*/
}
