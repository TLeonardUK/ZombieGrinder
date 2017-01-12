// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_EVENT_EVENT_
#define _GENERIC_EVENT_EVENT_

#include <vector>

#include "Generic/Events/Delegate.h"

#include <algorithm>

class EventBase
{
	MEMORY_ALLOCATOR(EventBase, "Generic");

protected:
};

template <typename DataType>
class Event : public EventBase
{
	MEMORY_ALLOCATOR(Event, "Generic");

private:
	std::vector<DelegateBase*> m_delegates;

public:
	Event()
	{
	}

	~Event()
	{
	}

	Event& operator+=(DelegateBase* rhs)
	{
		m_delegates.push_back(rhs);
		return *this;
	}

	Event& operator-=(DelegateBase* rhs)
	{
		std::vector<DelegateBase*>::iterator iter = std::find(m_delegates.begin(), m_delegates.end(), rhs);
		if (iter != m_delegates.end())
		{
			m_delegates.erase(iter);
		}

		return *this;
	}
	
	bool Registered()
	{
		return m_delegates.size() > 0;
	}

	void Fire(DataType* data)
	{
		for (std::vector<DelegateBase*>::iterator iter = m_delegates.begin(); iter != m_delegates.end(); iter++)
		{
			(*iter)->Fire(data);
		}
	}

};

#endif