// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_THREADLOCALDATA_
#define _GENERIC_THREADLOCALDATA_

class Platform_ThreadLocalData
{
public:
	static Platform_ThreadLocalData* Create();

	virtual void* Get() = 0;
	virtual void  Set(void*) = 0;
};

template <typename T>
class ThreadLocalData
{
	MEMORY_ALLOCATOR(ThreadLocalData, "Platform");

protected:
	Platform_ThreadLocalData* m_plat;

public:
	ThreadLocalData()
	{
		m_plat = Platform_ThreadLocalData::Create();
	}
	/*
	ThreadLocalData(T value)
	{
		m_plat = Platform_ThreadLocalData::Create();
		Set(value);
	}
	*/
	~ThreadLocalData()
	{
		// TODO: fix this shit. macos asserts on exit(0) due to these destructors.
		//SAFE_DELETE(m_plat);
	}

	T Get()
	{
		return reinterpret_cast<T>(m_plat->Get());
	}

	void Set(T value)
	{
		m_plat->Set(reinterpret_cast<void*>(value));
	}
	
	DBG_STATIC_ASSERT_STR(sizeof(T) == sizeof(void*), "Thread local data must be the size of a pointer.");

};

#endif

