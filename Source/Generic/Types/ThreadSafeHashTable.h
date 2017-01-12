// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_THREADSAFEHASHTABLE_
#define _GENERIC_THREADSAFEHASHTABLE_

#define HASH_TABLE_INITIAL_BUCKETS	32
#define HASH_TABLE_BUCKET_INCREMENT	2		// Keep as 2, for speed optimizations bucket capacities should always be power of 2
#define HASH_TABLE_MAX_LOAD			0.75f

#include <string.h>
#include <new>

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include "Generic/Types/CachedMemoryPool.h"

template <typename ValueType, typename KeyType>
class ThreadSafeHashTable
{
	MEMORY_ALLOCATOR(ThreadSafeHashTable<ValueType COMMA KeyType>, "Data Types");

private:

	// Value stored in hash table.
	struct HashTableValue
	{
		//MEMORY_ALLOCATOR(HashTableValue, "Data Types");

	public:
		ValueType		Value;
		bool			HasValue;
		KeyType			Hash;
		HashTableValue*	Next;
	};

public:

	struct Iterator
	{
		MEMORY_ALLOCATOR(Iterator, "Data Types");

	private:
		ThreadSafeHashTable<ValueType, KeyType>* m_hash_table;
		HashTableValue* m_node;
		int m_bucket_index;

	public:
		Iterator(ThreadSafeHashTable<ValueType, KeyType>* table, int bucket_index, HashTableValue* node)
			: m_hash_table(table)
			, m_bucket_index(bucket_index)
			, m_node(node)
		{
		}

		Iterator()
			: m_bucket_index(-1)
			, m_node(NULL)
			, m_hash_table(NULL)
		{
		}

		void operator++() 
		{ 
			// Next node in current bucket?
			if (m_node->Next != NULL)
			{
				m_node = m_node->Next;
				return;
			}

			// Look for next bucket?
			m_bucket_index++;
			for (; m_bucket_index < m_hash_table->m_allocated; m_bucket_index++)
			{
				HashTableValue* bucket = m_hash_table->m_data[m_bucket_index];
				if (bucket != NULL)
				{
					m_node = bucket;
					return;
				}
			}

			// Nothing left :(
			m_bucket_index	= -1;
			m_node			= NULL;
			m_hash_table	= NULL;
		}

		void operator++(int value) 
		{ 
			operator++();
		}

		bool operator==(Iterator iter) const 
		{ 
			return m_bucket_index == iter.m_bucket_index && m_node == iter.m_node; 
		} 

		bool operator!=(Iterator iter) const 
		{ 
			return !operator==(iter); 
		} 

		ValueType& operator*() const
		{
			return m_node->Value;
		}

		KeyType& Get_Key() const
		{
			return m_node->Hash;
		}

		ValueType& Get_Value() const
		{
			return m_node->Value;
		}
	};

private:

	HashTableValue**					m_data;
	int									m_size;
	int									m_allocated;

	CachedMemoryPool<HashTableValue>	m_memory_pool;

	bool m_disposed;

	Mutex*								m_mutex;

protected:

	// Gets the bucket a hash should be in.
	unsigned int Get_Bucket(KeyType hash) const
	{
		DBG_ASSERT(m_allocated > 0);
		return static_cast<unsigned int>(hash) & (m_allocated - 1);
	}

	// Gets a value from a hash.
	HashTableValue* Get_Value(KeyType hash) const
	{
		// Figure out bucket index.
		int bucket_index = Get_Bucket(hash);

		// Find end of current bucket chain.
		// TODO: Double linked list, faster.
		HashTableValue* bucket = m_data[bucket_index];
		while (bucket != NULL)
		{
			if (bucket->Hash == hash)
			{
				return bucket;
			}

			bucket = bucket->Next;
		}

		return NULL;
	}

	// Allocates our buckets, and keeps old data if neccessary.
	void Allocate(int size, bool keep_old)
	{
		HashTableValue**	 old_data		= m_data;
		int					 old_alloc_size = m_allocated;
		int					 allocate_size	=  size;

		if (allocate_size > m_allocated || m_data == NULL)
		{
			m_data = new HashTableValue*[allocate_size];
			DBG_ASSERT(m_data != NULL);

			m_allocated = allocate_size;
			memset(m_data, 0, allocate_size * sizeof(HashTableValue*));

			if (keep_old == true && old_data != NULL)
			{
				m_size = 0;

				// Reinsert the data into the new memory block.
				for (int i = 0; i < old_alloc_size; i++)
				{
					HashTableValue* val = old_data[i];
					while (val != NULL)
					{
						Set(val->Hash, val->Value);
						val = val->Next;
					}						
				}
			}
			else
			{
				m_size = 0;
			}

			// Dispose of any old data.
			if (old_data != NULL)
			{
				for (int i = 0; i < old_alloc_size; i++)
				{
					HashTableValue* val = old_data[i];
					while (val != NULL)
					{
						HashTableValue* next = val->Next;

						//delete val;
						m_memory_pool.Release(val);

						val = next;
					}						
				}

				SAFE_DELETE_ARRAY(old_data);
			}
		}
	}

	// Checks if buckets are overloaded, if they are it increases
	// the amounts of buckets and redistributes nodes.
	void Check_Load()
	{
		float current_load = (float)m_size / (float)m_allocated;
		if (current_load > HASH_TABLE_MAX_LOAD)
		{
			float new_load_size = (float)m_allocated * HASH_TABLE_BUCKET_INCREMENT;

			// TODO: Is this neccessary?
			while (true)
			{
				float new_load = float(m_size) / float(new_load_size);
				if (new_load < HASH_TABLE_MAX_LOAD)
				{
					break;
				}
				new_load_size *= HASH_TABLE_BUCKET_INCREMENT;
			}

			Allocate((int)new_load_size, true);
		}
	}

public:

	// Constructors.
	ThreadSafeHashTable()
		: m_data(NULL)
		, m_allocated(0)
		, m_size(0)
		, m_disposed(false)
		//, m_memory_pool(sizeof(HashTableValue))
	{
		Allocate(HASH_TABLE_INITIAL_BUCKETS, false);

		m_mutex = Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);
	}

	ThreadSafeHashTable(const ThreadSafeHashTable& other)
		: m_data(NULL)
		, m_allocated(0)
		, m_size(0)
		, m_disposed(false)
	{
		Allocate(HASH_TABLE_INITIAL_BUCKETS, false);

		m_mutex = Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);

		for (int i = 0; i < other.m_allocated; i++)
		{	
			const HashTableValue* bucket = other.m_data[i];
			while (bucket != NULL)
			{	
				Set(bucket->Hash, bucket->Value);
				bucket = bucket->Next;
			}
		}
	}

	ThreadSafeHashTable& operator=( const ThreadSafeHashTable& other ) 
	{	 
		m_data = NULL;
		m_allocated = 0;
		m_size = 0;
		m_disposed = false;

		Allocate(HASH_TABLE_INITIAL_BUCKETS, false);

		m_mutex = Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);

		for (int i = 0; i < other.m_allocated; i++)
		{	
			const HashTableValue* bucket = other.m_data[i];
			while (bucket != NULL)
			{	
				Set(bucket->Hash, bucket->Value);
				bucket = bucket->Next;
			}
		}

		return *this;
	}


	~ThreadSafeHashTable()
	{
		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			while (bucket != NULL)
			{
				HashTableValue* next = bucket->Next;

				//delete next;
				if (next != NULL)
				{
					m_memory_pool.Release(next);
				}

				bucket = next;
			}
		}

		m_disposed = true;

		SAFE_DELETE_ARRAY(m_data);
		m_allocated = 0;
	}

	// Iterator stuff!
	Iterator Begin()
	{
		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			if (bucket != NULL)
			{
				return Iterator(this, i, bucket);			
			}
		}

		return Iterator();
	}

	Iterator End()
	{
		return Iterator();
	}

	// Returns true if we contain the given hash.
	bool Contains(KeyType hash) const
	{
		MutexLock lock(m_mutex);
		return Get_Value(hash) != NULL;
	}

	// Returns the size of the hash table.
	int Size() const
	{
		MutexLock lock(m_mutex);
		return m_size;
	}

	// Clears the hash table of values.
	void Clear()
	{
		MutexLock lock(m_mutex);

		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			while (bucket != NULL)
			{
				HashTableValue* next = bucket->Next;

				//delete next;
				m_memory_pool.Release(bucket);

				bucket = next;
			}

			m_data[i] = NULL;
		}

		m_size = 0;
	}

	// Removes a hash value.
	void Remove(KeyType hash)
	{
		MutexLock lock(m_mutex);

		// Figure out bucket index.
		unsigned int bucket_index = Get_Bucket(hash);

		// Find end of current bucket chain.
		// TODO: Double linked list, faster.
		HashTableValue* bucket = m_data[bucket_index];
		HashTableValue* prev   = NULL;
		while (bucket != NULL)
		{
			if (bucket->Hash == hash)
			{
				bucket->HasValue = false;

				if (prev != NULL)
				{
					prev->Next = bucket->Next;					
				}
				else
				{
					m_data[bucket_index] = bucket->Next;
				}

				//delete bucket;
				m_memory_pool.Release(bucket);

				m_size--;

				return;
			}

			prev = bucket;
			bucket = bucket->Next;
		}

		//DBG_ASSERT(false);
	}

	// Gets an "index" of the hash table. This is used for interating, and is slow
	// as shit, you should only use this when doing things like deleting eveyrthing in the hash table.
	void Get_Index(int index, KeyType& hash, ValueType& value)
	{
		MutexLock lock(m_mutex);

		int at_index = 0;

		for (int i = 0; i < m_allocated; i++)
		{	
			HashTableValue* bucket = m_data[i];
			while (bucket != NULL)
			{
				if (at_index == index)
				{
					hash = bucket->Hash;
					value = bucket->Value;
					return;
				}

				at_index++;
				bucket = bucket->Next;
			}
		}
	}

	// Gets the value for the given hash.
	ValueType Get(KeyType hash) const
	{
		MutexLock lock(m_mutex);

		HashTableValue* val = Get_Value(hash);
		return val == NULL ? NULL : val->Value;
	}

	// Gets the value for the given hash.
	bool Get(KeyType hash, ValueType& val_out) const
	{
		MutexLock lock(m_mutex);

		HashTableValue* val = Get_Value(hash);
		if (val != NULL)
		{
			val_out = val->Value;
		}

		return val != NULL;
	}

	// Gets a ptr to the value for the given hash.
	bool Get_Ptr(KeyType hash, ValueType*& val_out) const
	{
		MutexLock lock(m_mutex);

		DBG_ASSERT(!m_disposed);

		HashTableValue* val = Get_Value(hash);
		if (val != NULL)
		{
			val_out = &val->Value;
		}

		return val != NULL;
	}

	// Sets the value for the given hash.
	void Set(KeyType hash, ValueType value)
	{
		MutexLock lock(m_mutex);

		// Figure out bucket index.
		unsigned int bucket_index = Get_Bucket(hash);

		// Find end of current bucket chain.
		// TODO: Double linked list, faster.
		HashTableValue* bucket = m_data[bucket_index];
		while (bucket != NULL)
		{
			//if (value == NULL)
			//{
			//	if (bucket->Hash == hash)
			//	{
			//		bucket->Value = NULL;
			//		return;
			//	}
			//}
			//else
			//{
			if (bucket->HasValue == false)
			{
				bucket->Value = value;
				bucket->HasValue = true;
				return;
			}
			else
			{
				DBG_ASSERT(bucket->Hash != hash);
			}
			//}

			HashTableValue* next = bucket->Next;

			if (next == NULL)
			{
				break;					
			}

			bucket = next;
		}

		// Insert the new item.
		HashTableValue* val = new(m_memory_pool.Allocate()) HashTableValue;
		val->Hash		= hash;
		val->Value		= value;
		val->Next		= NULL;
		val->HasValue	= true;

		if (bucket == NULL)
		{
			m_data[bucket_index] = val;
		}
		else
		{
			bucket->Next = val;			
		}

		m_size++;

		// Expand if we are over the load limit.
		Check_Load();
	}

};

#endif