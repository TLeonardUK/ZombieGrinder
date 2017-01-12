/* *****************************************************************

		MemoryHeap.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "Generic/Types/MemoryHeap.h"
#include "Generic/Threads/MutexLock.h"

#include <cstring>

MemoryHeap::MemoryHeap()
//	: m_start_size(0)					// Blllllllllllah, we have to get rid of these
//	, m_total_allocations(0)			// so the global heaps don't get messed up during static iniitalization. 
//	, m_memory(NULL)
//	, m_mutex(NULL)
//	: m_group_index(0)
//	, m_allocation_index(0)
{
#ifdef MEMORY_HEAP_LOG_ENABLED_DEFAULT
	Enable_Callstack_Logging();
#endif
}

MemoryHeap::MemoryHeap(int size)
	: m_start_size(size)
	, m_total_allocations(0)
	//, m_mutex(NULL)
	, m_group_index(0)
	, m_allocation_index(0)
	, m_total_allocated_bytes(0)
	, m_lifetime_allocs(0)
#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	#ifdef USE_HEAP_VALIDATION_DEFAULT_ON
	, m_validate_heap(true)
	#else
	, m_validate_heap(false)	
	#endif
#endif
#ifdef MEMORY_HEAP_LOG_CALLSTACKS
	, m_logging_enabled(false)
#endif
{
#ifdef MEMORY_HEAP_LOG_ENABLED_DEFAULT
	Enable_Callstack_Logging();
#endif

	m_memory = (char*)Get_Allocator()->Alloc(size);
	m_owns_memory = true;
	Setup();
}

#ifdef MEMORY_HEAP_LOG_CALLSTACKS
void MemoryHeap::Enable_Callstack_Logging()
{
	m_logging_enabled = true;
}

void MemoryHeap::Disable_Callstack_Logging()
{
	m_logging_enabled = false;
}

void MemoryHeap::Dump_Callstack_Logging()
{
	//MutexLock lock(m_mutex);
	
	MemoryHeapBlock* block = m_first_block->next;
	while (block != m_first_block)
	{
		if (block->used == true && block->callstack_size != 0 && block->size > 1024)
		{
			DBG_LOG("##### Block %.2f KB #####", block->size / 1024.0f);
			Log_Block_Callstack(block);
		}		
		block = block->next;
	}
}
#endif

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
void MemoryHeap::Enable_Heap_Validation(bool enabled)
{
	m_validate_heap = true;
}
#endif

void MemoryHeap::Push_Group()
{
	DBG_ASSERT(m_group_index < max_groups);
	m_groups[m_group_index++] = m_allocation_index;
	if (m_group_index == 1)
	{
		m_allocation_index = 0;
	}
}

void MemoryHeap::Pop_Group()
{
	DBG_ASSERT(m_group_index > 0);
	unsigned int index = m_groups[--m_group_index];
	if (m_group_index == 0)
	{
		m_allocation_index = 0;
	}

	//MutexLock lock(m_mutex);
	
	MemoryHeapBlock* block = m_first_block->next;
	while (block != m_first_block)
	{
		if (block->used == true && block->alloc_index > index && block->grouped == true)
		{
			Free_Block(block);
			block = m_first_block->next;			
		}	
		else
		{
			block = block->next;
		}
	}
}

void MemoryHeap::Setup_With_Memory(char* mem, int size)
{
	m_start_size = size;
	m_memory = mem;
	m_owns_memory = false;
	Setup();
}

//void MemoryHeap::Become_Reentrant()
//{
//	DBG_ASSERT(m_mutex == NULL);
//	m_mutex = Mutex::Create();
//}

void MemoryHeap::Setup()
{
	m_first_block = (MemoryHeapBlock*)m_memory;
	m_first_block->data = (m_memory + sizeof(MemoryHeapBlock));
	m_first_block->size = m_start_size - sizeof(MemoryHeapBlock);
	m_first_block->used = false;
	m_first_block->prev_free = &m_free_chain;
	m_first_block->next_free = &m_free_chain;
	m_first_block->next = &m_free_chain;
	m_first_block->prev = &m_free_chain;

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	Set_Memory(m_first_block->guard_start, MEMORY_HEAP_GUARD_BYTE_COUNT, MEMORY_HEAP_GUARD_BYTE_MAGIC);
	Set_Memory(m_first_block->guard_end, MEMORY_HEAP_GUARD_BYTE_COUNT, MEMORY_HEAP_GUARD_BYTE_MAGIC);
#endif

	m_free_chain.next = m_first_block;
	m_free_chain.prev = m_first_block;
	m_free_chain.next_free = m_first_block;
	m_free_chain.prev_free = m_first_block;
	m_free_chain.used = true;	// Neither this or size should have any effect
	m_free_chain.size = 0;		// but setting the to this will at least make things assert
								// if we ever try to allocate the chain head.
}

MemoryHeap::~MemoryHeap()
{
	if (m_owns_memory)
	{
		Get_Allocator()->Free(m_memory);
	}
}

int MemoryHeap::Get_Block_Size(void* block)
{
	MemoryHeapBlock* b = (MemoryHeapBlock*)(((char*)block) - sizeof(MemoryHeapBlock));
	return b->alloc_size;
}

int MemoryHeap::Get_Total_Used()
{
	return m_total_allocated_bytes;
}

int MemoryHeap::Get_Lifetime_Bytes_Allocated()
{
	return m_lifetime_allocs;
}

int MemoryHeap::Get_Total()
{
	return m_start_size;
}

void MemoryHeap::Set_Memory(int* ptr, int count, int expected_value)
{
#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	for (int i = 0; i < count; i++)
	{
		ptr[i] = expected_value;
	}
#endif
}

bool MemoryHeap::Validate_Memory(int* ptr, int count, int expected_value)
{
#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	for (int i = 0; i < count; i++)
	{
		if (ptr[i] != expected_value)
			return false;
	}
#endif
	return true;
}

void MemoryHeap::Log_Block_Callstack(MemoryHeapBlock* block)
{
#ifdef MEMORY_HEAP_LOG_CALLSTACKS
	Platform* platform = Platform::Get();

	for (int i = 0; i < block->callstack_size; i++)
	{
		StackFrame& frame = block->callstack[i];		
		DecodedStackFrame output;
		platform->Resolve_Stack_Frame_Info(frame, output);

		DBG_LOG("\t[%i] %s (%i): %s", i, output.File, output.Line, output.Name); 
	}
#endif
}

void MemoryHeap::Validate_Heap()
{
//	if (!m_validate_heap)
//		return;

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	int total_size = 0;

	MemoryHeapBlock* block = m_free_chain.next;
	while (block != &m_free_chain)
	{
		bool result_a = Validate_Memory(block->guard_start, MEMORY_HEAP_GUARD_BYTE_COUNT, MEMORY_HEAP_GUARD_BYTE_MAGIC);
		bool result_b = Validate_Memory(block->guard_end,   MEMORY_HEAP_GUARD_BYTE_COUNT, MEMORY_HEAP_GUARD_BYTE_MAGIC);

		if (result_a == false || result_b == false)
		{
#ifdef MEMORY_HEAP_LOG_CALLSTACKS
			DBG_LOG("==============================================================");
			DBG_LOG("Guard bytes invalidated for block:");
			Log_Block_Callstack(block);
			DBG_LOG("Prev block:");
			if (block->prev == NULL)
			{
				DBG_LOG("\tNULL");
			}
			else
			{
				Log_Block_Callstack(block->prev);
			}
			DBG_LOG("==============================================================");
#endif
			DBG_ASSERT(false);
		}

		total_size += block->size + sizeof(MemoryHeapBlock);
		block = block->next;
	}

	DBG_ASSERT(total_size == m_start_size);

//	Log_Stats();
#endif
}

void MemoryHeap::Log_Large_Allocations()
{
	//MutexLock lock(m_mutex);
	
	MemoryHeapBlock* block = m_free_chain.next;
	while (block != &m_free_chain)
	{
		if (block->used == true && block->size > 8 * 1024 * 1024)
		{
			DBG_LOG("##### Block %.2f MB #####", block->size / 1024.0f / 1024.0f);
			Log_Block_Callstack(block);
		}		
		block = block->next;
	}
}

void MemoryHeap::Log_Stats()
{
	//MutexLock lock(m_mutex);

	int total = 0;
	int total_blocks = 0;
	int used = 0;
	int total_usable = 0;
	int largest_free_block = 0;

	MemoryHeapBlock* block = m_free_chain.next;
	while (block != &m_free_chain)
	{
		if (block->used == false)
		{
			total_usable += block->size;
			if (block->size > largest_free_block)
			{
				largest_free_block = block->size;
			}
		}
		else
		{			
			used += block->size;
		}
		DBG_ASSERT(block->size >= 0);
		total += sizeof(MemoryHeapBlock) + block->size;
		total_blocks++;
		block = block->next;
	}

	DBG_ASSERT(total > 0);

	DBG_LOG("allocs:%i total-blocks:%i total=%i used=%i total-usable:%i largest-free-block:%i fragmentation:%.2f", m_total_allocations, total_blocks, total, used, total_usable, largest_free_block, (float)largest_free_block / (float)total_usable);
}

void* MemoryHeap::Alloc_From_Block(MemoryHeapBlock* block, int size)
{
	// Split the right hand side of if we will have a lot of free space in block after
	// fulfilling alloc request.
	if (block->size > size + (int)sizeof(MemoryHeapBlock) + minimum_block_size)
	{	
		// Split off the right hand-side to a new free block.
		MemoryHeapBlock* right_value = (MemoryHeapBlock*)(block->data + size);
		right_value->data = ((char*)right_value) + sizeof(MemoryHeapBlock);
		right_value->size = (block->size - size) - sizeof(MemoryHeapBlock);
		right_value->used = false;

		// Add new block to block-chain.
		right_value->next = block->next;
		right_value->next->prev = right_value;
		right_value->prev = block;
		block->next = right_value;

		// Add new block to free-chain.
		right_value->next_free = block->next_free;
		right_value->next_free->prev_free = right_value;
		right_value->prev_free = block;
		block->next_free = right_value;

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
		Set_Memory(right_value->guard_start, MEMORY_HEAP_GUARD_BYTE_COUNT, MEMORY_HEAP_GUARD_BYTE_MAGIC);
		Set_Memory(right_value->guard_end, MEMORY_HEAP_GUARD_BYTE_COUNT, MEMORY_HEAP_GUARD_BYTE_MAGIC);
#endif

		//DBG_LOG("Split block of size %i into blocks of size %i and %i (%i overhead)", block->size, size, right_value->size, sizeof(MemoryHeapBlock));
		
		block->size = size;
	}

	// Remove from free-chain.
	block->prev_free->next_free = block->next_free;
	block->next_free->prev_free = block->prev_free;
	block->next_free = NULL;
	block->prev_free = NULL;

	// Mark block as used and return.
	DBG_ASSERT(block->used == false);
	block->used = true;
	block->alloc_index = m_allocation_index++;
	block->grouped = (m_group_index > 0);
	block->alloc_size = size;

	//if (block->alloc_index == 8253)
	//{
	//	DBG_LOG("ALLOC!");
	//}

#ifdef MEMORY_HEAP_LOG_CALLSTACKS
	Platform* plat = Platform::Try_Get();
	if (plat != NULL && m_logging_enabled == true)
		block->callstack_size = plat->Get_Stack_Trace(block->callstack, block->max_callstack_frames, NULL, 2);	
	else
		block->callstack_size = 0;
#endif

	m_total_allocations++;
	m_total_allocated_bytes += block->size;

	//DBG_LOG("ALLC : %i", block->size);

	return block->data;
}

void* MemoryHeap::Realloc(void* ptr, int bytes)
{
	MemoryHeapBlock* block = (MemoryHeapBlock*)(((char*)ptr) - sizeof(MemoryHeapBlock));

	void* result = Alloc(bytes);
	if (result == NULL)
	{
		Free(ptr);
		return NULL;
	}

	memcpy(result, block->data, block->size);
	Free(ptr);

	return result;

	/*
	MemoryHeapBlock* block = (MemoryHeapBlock*)(((char*)ptr) - sizeof(MemoryHeapBlock));
	
	//DBG_LOG("ALLC : %i", bytes);

	{
		//MutexLock lock(m_mutex);

		if (block->size >= bytes)
		{
			return ptr;
		}
		else if (block->next->used == false && (block->size + block->next->size + sizeof(MemoryHeapBlock)) >= bytes)
		{
			MemoryHeapBlock* right_block = block->next;

			// Remove right-block from free-chain.
			right_block->prev_free->next_free = right_block->next_free;
			right_block->next_free->prev_free = right_block->prev_free;
			right_block->next_free = NULL;
			right_block->prev_free = NULL;

			// Remove right-block from block-chain.
			right_block->prev->next = right_block->next;
			right_block->next->prev = right_block->prev;
			right_block->next = NULL;
			right_block->prev = NULL;

			// Resize left-block.
			int increment = right_block->size + sizeof(MemoryHeapBlock);
			int new_size = block->size + increment;

			block->size = new_size;
			
			// Split block into 2 if we have enough space.
			int excess = block->size - bytes;
			if (excess > minimum_block_size)
			{

			}

			m_total_allocated_bytes += increment;
			DBG_LOG("RLLC : %i", increment);

			return ptr;
		}
	}

	void* result = Alloc(bytes);
	if (result == NULL)
	{
		Free(ptr);
		return NULL;
	}

	memcpy(result, block->data, block->size);
	Free(ptr);

	return result;
	*/
}

void* MemoryHeap::Alloc(int bytes)
{	
	m_lifetime_allocs += bytes;

	if (bytes > 1024 * 1024)
	{
		DBG_LOG("[Memory] Large allocation requested: %.2f mb.", bytes / 1024.0f / 1024.0f);
	}

	/*
	static int counter = 0;
	counter++;
	if ((counter % 1000) == 0)
		Log_Large_Allocations();
	*/
	
//	if (bytes > 1024 * 256)
//	{
//		DBG_LOG("ALLC : %.2f MB", (bytes / 1024.0f) / 1024.0f);
//	}

	//DBG_LOG("ALLC : %i", bytes);
	//int attempts = 0;
	///while (attempts++ < 1000)
	//{
		{
			//MutexLock lock(m_mutex);

		#ifdef USE_MEMORY_HEAP_GUARD_BYTES
			Validate_Heap();
		#endif

			int searches = 0;

			MemoryHeapBlock* block = m_free_chain.next_free;
			while (block != &m_free_chain)
			{
				if (block->size >= bytes)
				{
					void* ptr = Alloc_From_Block(block, bytes);
		#ifdef USE_MEMORY_HEAP_GUARD_BYTES
		//			Validate_Heap();
		#endif
					DBG_ASSERT(ptr != (void*)0xFFFFFFFF);
					return ptr;
				}
				block = block->next_free;
				searches++;
			}
		}

		// SHITTY SHITTY HACK :( - Sort out these issues not whitewash them!
	//	if (attempts <= 1)
	//		DBG_LOG("Memory pool out of memory. Attempted to allocate %i (%.2f MB). Attempting to wait for memory to be freed.", bytes, (bytes / 1024.0f) / 1024.0f);
	//	Platform::Get()->Sleep(0.001f);
	//}

	//Log_Stats();
	//DBG_ASSERT_STR(false, "Memory pool out of memory. Attempted to allocate %i (%.2f MB).", bytes, (bytes / 1024.0f) / 1024.0f);
	return NULL;
}

void MemoryHeap::Free(void* ptr)
{
	//MutexLock lock(m_mutex);

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
//	Validate_Heap();
#endif

	MemoryHeapBlock* block = (MemoryHeapBlock*)(((char*)ptr) - sizeof(MemoryHeapBlock));
	Free_Block(block);
}

void MemoryHeap::Free_Block(MemoryHeapBlock* block)
{	
	if (block->used != true)
	{
		DBG_LOG("Detected double free or invalid pointer at 0x%08x", block);
		return;
	}

	// Mark block as free again.
	DBG_ASSERT(block->used == true);
	block->used = false;

	m_total_allocated_bytes -= block->size;
	m_total_allocations--;

	//DBG_LOG("FREE : %i", block->size);
	//Log_Stats();

	// Re-add the block to the free-chain.
	MemoryHeapBlock* old_head = m_free_chain.next_free;
	m_free_chain.next_free = block;
	block->prev_free = old_head->prev_free;
	block->next_free = old_head;
	old_head->prev_free = block;

	DBG_ASSERT(block->next_free != NULL);
	DBG_ASSERT(block->prev_free != NULL);

	// Try and find our right-most free contiguous block.
	MemoryHeapBlock* right_block = block;
	while (right_block->next != &m_free_chain && // Don't want to loop back to the start! Thats not contiguous!
		   right_block->next->used == false)
	{
		right_block = right_block->next;
	}

	// Coalesce all the free blocks towards the left.
	while (right_block->prev != & m_free_chain &&
		   right_block->prev->used == false)
	{
		MemoryHeapBlock* left_block = right_block->prev;

		// Remove right-block from free-chain.
		right_block->prev_free->next_free = right_block->next_free;
		right_block->next_free->prev_free = right_block->prev_free;
		right_block->next_free = NULL;
		right_block->prev_free = NULL;

		// Remove right-block from block-chain.
		right_block->prev->next = right_block->next;
		right_block->next->prev = right_block->prev;
		right_block->next = NULL;
		right_block->prev = NULL;

		// Resize left-block.
		int new_size = left_block->size + right_block->size + sizeof(MemoryHeapBlock);

		//DBG_LOG("Coalesced blocks of size %i and %i into block of size %i.", left_block->size, right_block->size, new_size);

		left_block->size = new_size;

		right_block = left_block;
	}

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
//	Validate_Heap();
#endif
}