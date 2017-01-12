// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VIRTUALARRAY3_
#define _GENERIC_VIRTUALARRAY3_

#include "Generic/Types/HashTable.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

//#define VIRTUALARRAY3_THREAD_SYNC

template <typename T>
class VirtualArray3
{
//	MEMORY_ALLOCATOR(VirtualArray3<T>, "Data Types");

protected:

	HashTable<T, int> m_hash_table;

#ifdef VIRTUALARRAY3_THREAD_SYNC
	Mutex*			  m_mutex;
#endif

	// Used by the hashing function to produce interleaved hashs
	// to create a z-order curve hash.
	INLINE int ZOrder(int x) const
	{
		x &= 0x3FF;
		x  = (x | (x<<16)) & 4278190335;
		x  = (x | (x<<8))  & 251719695;
		x  = (x | (x<<4))  & 3272356035;
		x  = (x | (x<<2))  & 1227133513;
		return x;
	}

public:

	VirtualArray3()
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		m_mutex = Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);
#endif
	}

	~VirtualArray3()
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		SAFE_DELETE(m_mutex);
#endif
	}

	// Produces a hash value out of the given coordinates.
	int	Hash(int x, int y, int z) const
	{
		return ZOrder(x) + (ZOrder(y) << 1) + (ZOrder(z) << 2);
	}
	
	// Gets the value at the given coordinates.
	T Get(int x, int y, int z) const
	{
		return Get(Hash(x, y, z));
	}

	// Gets the value at the given hashed coordinates.
	T Get(int hash) const
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		return m_hash_table.Get(hash);
	}

	// Sets the value at the given coordinates.
	void Set(int x, int y, int z, T value)
	{
		Set(Hash(x, y, z), value);
	}
	
	// Sets the value at the given hashed coordinates.
	void Set(int hash, T value)
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		m_hash_table.Set(hash, value);
	}
	
	// Sets the number of entries in the array.
	int Size() const
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		return m_hash_table.Size();
	}
	
	// Gets the value at an index (SLOOOOOOW, do not use for anything important!).
	void Get_Index(int index,  int& hash, T& value)
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		m_hash_table.Get_Index(index, hash, value);
	}
	
	// Removes an element from the varray.
	void Remove(int x, int y, int z)
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		m_hash_table.Remove(Hash(x, y, z));
	}
	
	// Returns true if we contain the given value.
	bool Contains(int x, int y, int z)
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		return m_hash_table.Contains(Hash(x, y, z));
	}

	// Clears array of entries.
	void Clear()
	{
#ifdef VIRTUALARRAY3_THREAD_SYNC
		MutexLock lock(m_mutex);
#endif
		m_hash_table.Clear();
	}


};

#endif