/* *****************************************************************

		CCompiler.cpp

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */

#include "XScript/Memory.h"
#include "Generic/Types/MemoryHeap.h"

#define COMPILER_MEMORY_SIZE 16 * 1024 * 1024

static char*		g_compiler_memory = NULL;
static MemoryHeap	g_compiler_memory_heap;
static bool			g_compiler_memory_heap_setup = false;

void Init_Compiler_Memory()
{
	if (g_compiler_memory_heap_setup)
		Deinit_Compiler_Memory();

	if (g_compiler_memory == NULL)
		g_compiler_memory = new char[COMPILER_MEMORY_SIZE];

	g_compiler_memory_heap.Setup_With_Memory(g_compiler_memory, COMPILER_MEMORY_SIZE);
	g_compiler_memory_heap_setup = true;
}

void Deinit_Compiler_Memory()
{
	g_compiler_memory_heap_setup = false;
}

void* Alloc_Compiler_Memory(int size)
{
	if (g_compiler_memory_heap_setup == false)
		return malloc(size); // Static instances ;_;
	else
		return g_compiler_memory_heap.Alloc(size);

	return NULL;
}

void Free_Compiler_Memory(void* ptr)
{
	g_compiler_memory_heap.Free(ptr);
}