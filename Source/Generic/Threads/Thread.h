// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_THREAD_
#define _GENERIC_THREAD_

#include <string>

struct ThreadPriority
{
	enum Type
	{
		Highest,
		High,
		Normal,
		Low,
		Lowest,

		COUNT,
	};
};

class Thread
{
	MEMORY_ALLOCATOR(Thread, "Platform");

protected:
	std::string m_name;

	Thread(const char* name);

	friend class Platform;

public:
	typedef void (*EntryPoint)(Thread* self, void* ptr);
	static Thread* Create(const char* name, EntryPoint entry_point, void* ptr);
	static Thread* Get_Current();

	// Control functions.
	virtual bool Is_Running() = 0;
	virtual void Start() = 0;
	virtual void Set_Priority(ThreadPriority::Type priority) = 0;
	virtual void Set_Affinity(int mask) = 0;

	// General functions.
	virtual void Sleep(float seconds) = 0;
	virtual int  Get_Core_Count() = 0;
	virtual void Join() = 0;

	INLINE std::string Get_Name()
	{
		return m_name;
	}
	void Set_Name(std::string name)
	{
		m_name = name;
	}


// Dammnit windows.
#ifdef Yield
#undef Yield
#endif
	virtual void Yield() = 0;

};

#endif

