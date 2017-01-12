/* *****************************************************************

		Memory.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _COMPILER_MEMORY_H_
#define _COMPILER_MEMORY_H_

#include <cstddef>         

class MemoryHeap;

void Init_Compiler_Memory();
void Deinit_Compiler_Memory();

void* Alloc_Compiler_Memory(int size);
void Free_Compiler_Memory(void* ptr);

// Compiler specific memory allocated, all classes allocated using this will be under the control
// of Init_Compiler_Memory/Deinit_Compiler_Memory()
#define COMPILER_ALLOCATOR()																				\
		public:																													\
			static void* operator new(std::size_t size)																			\
			{																													\
				return Alloc_Compiler_Memory(size);																				\
			}																													\
			static void* operator new[](std::size_t size)																		\
			{																													\
				return Alloc_Compiler_Memory(size);																				\
			}																													\
			static void* operator new(std::size_t size, void* ptr)																\
			{																													\
				return ptr;																										\
			}																													\
			static void operator delete(void* ptr)																				\
			{																													\
				return Free_Compiler_Memory(ptr);																				\
			}																													\
			static void operator delete[](void* ptr)																			\
			{																													\
				return Free_Compiler_Memory(ptr);																				\
			}																													\
		private:


#endif