// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_EVENT_DELEGATE_
#define _GENERIC_EVENT_DELEGATE_

class DelegateBase
{
	MEMORY_ALLOCATOR(DelegateBase, "Generic");

public:
	virtual void Fire(void* data) = 0;
};

template <typename ClassType, typename DataType>
class Delegate : public DelegateBase
{
	MEMORY_ALLOCATOR(Delegate, "Generic");

public:
	typedef void (ClassType::*MemberCallback)(DataType* data);

private:
	ClassType*		m_instance;
	MemberCallback  m_callback;

public:
	void Fire(void* data)
	{
		(m_instance->*m_callback)(reinterpret_cast<DataType*>(data));
	}

public:
	Delegate(ClassType* instance, MemberCallback callback)
		: m_instance(instance)
		, m_callback(callback)
	{
	}

	~Delegate()
	{
		m_instance = NULL;
		m_callback = NULL;
	}

};

#endif