// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MEMORY_ALLOCATOR_
#define _GENERIC_MEMORY_ALLOCATOR_

#include <stddef.h>
#include <typeinfo>
#include <stdlib.h>

// Define this in any class who's allocations you wish to manually control
// through an allocator. All allocations by default go through the global heap.
//
//		class_name		= name of class this define is inside of.
//		category_name	= category used when reporting allocations ("audio", "graphics", etc)
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
#define MEMORY_ALLOCATOR(class_name, category_name)																				\
		public:																													\
			static void* operator new(size_t size)																				\
			{																													\
				MemoryAllocator* allocator = MemoryManager::Get_Allocator<class_name>(category_name);							\
				void* memory = allocator->Alloc(size + sizeof(MemoryAllocator*));												\
				*reinterpret_cast<MemoryAllocator**>(memory) = allocator;														\
				void* block = (u8*)memory + sizeof(MemoryAllocator*);															\
				return block;																									\
			}																													\
			static void* operator new[](size_t size)																			\
			{																													\
				MemoryAllocator* allocator = MemoryManager::Get_Allocator<class_name>(category_name);							\
				void* memory = allocator->Alloc(size + sizeof(MemoryAllocator*));												\
				*reinterpret_cast<MemoryAllocator**>(memory) = allocator;														\
				void* block = (u8*)memory + sizeof(MemoryAllocator*);															\
				return block;																									\
			}																													\
			static void* operator new(size_t size, void* ptr)																	\
			{																													\
				return ptr;																										\
			}																													\
			static void operator delete(void* ptr)																				\
			{																													\
				void* block_start = (u8*)ptr - sizeof(MemoryAllocator*);														\
				MemoryAllocator* allocator = *reinterpret_cast<MemoryAllocator**>(block_start);									\
				allocator->Free(block_start);																					\
			}																													\
			static void operator delete[](void* ptr)																			\
			{																													\
				void* block_start = (u8*)ptr - sizeof(MemoryAllocator*);														\
				MemoryAllocator* allocator = *reinterpret_cast<MemoryAllocator**>(block_start);									\
				allocator->Free(block_start);																					\
			}																													\
			virtual MemoryAllocator* Get_Allocator()																			\
			{																													\
				return MemoryManager::Get_Allocator<class_name>(category_name);													\
			}																													\
		private:
#else
#define MEMORY_ALLOCATOR(class_name, category_name)																				\
		public:																													\
			static void* operator new(size_t size)																				\
			{																													\
				return new char[size];																							\
			}																													\
			static void* operator new[](size_t size)																			\
			{																													\
				return new char[size];																							\
			}																													\
			static void* operator new(size_t size, void* ptr)																	\
			{																													\
				return ptr;																										\
			}																													\
			static void operator delete(void* ptr)																				\
			{																													\
				delete[] ((char*)ptr);																										\
			}																													\
			static void operator delete[](void* ptr)																			\
			{																													\
				delete[] ((char*)ptr);																										\
			}																													\
			virtual MemoryAllocator* Get_Allocator()																			\
			{																													\
				return MemoryManager::Get_Allocator<class_name>(category_name);													\
			}																													\
		private:
#endif

// Memory allocator base class.
class MemoryAllocator
{
private:
	const char* m_class_name;
	const char* m_category;
	int			m_bytes_allocated;

public:
	MemoryAllocator(const char* class_name, const char* category);
	~MemoryAllocator();

	int Get_Bytes_Allocated();
	const char* Get_Class_Name();
	const char* Get_Category();

	void* Alloc(size_t size);
	void* Realloc(void* ptr, size_t size);
	void  Free(void* ptr);

	template <typename T>
	T* New(int elements = 1)
	{
		void* data = Alloc(sizeof(u32) + (elements * sizeof(T)));
		*reinterpret_cast<u32*>(data) = elements;
		T* buff = reinterpret_cast<T*>(((u8*)data) + sizeof(u32));
		for (int i = 0; i < elements; i++)
		{
			new(buff[i]) T();
		}
		return reinterpret_cast<T*>((char*)buff + sizeof(u32));
	}

	template <typename T>
	void Delete(T* buffer)
	{
		void* data = reinterpret_cast<void*>((char*)buffer - sizeof(u32));
		int elements = *reinterpret_cast<u32*>(data);
		for (int i = 0; i < elements; i++)
		{
			buffer[i].~T();
		}
		Free(data);

	}
	

};

#endif
