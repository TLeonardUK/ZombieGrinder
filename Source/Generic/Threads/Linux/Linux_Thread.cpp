// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Threads/Linux/Linux_Thread.h"
#include "Generic/Threads/Linux/Linux_Mutex.h"
#include "Generic/Threads/Linux/Linux_Semaphore.h"
#include "Generic/Threads/MutexLock.h"

#include <unistd.h>
#include <time.h>

static pthread_key_t tls_key;

Linux_Thread* Linux_Thread::Get_Current_Thread()
{
	Linux_Thread* thread = (Linux_Thread*)pthread_getspecific(tls_key);
	DBG_ASSERT(thread != NULL);
	return thread;
}

void Linux_Thread::Setup_Main()
{	
	pthread_key_create(&tls_key, NULL);

	Linux_Thread* thread = new Linux_Thread(pthread_self());
	thread->Set_Name("Main Thread");
	pthread_setspecific(tls_key, thread);
}

Linux_Thread::Linux_Thread(const char* name, EntryPoint entry_point, void* ptr) 
	: Thread(name) 
	, m_finished(false)
	, m_thread(0)
{
	m_entry_point			= entry_point;
	m_entry_ptr				= ptr;
	m_suspended_semaphore	= Semaphore::Create();
	
	DBG_LOG("Thread 0x%08x '%s' created (data=0x%08x).", this, m_name.c_str(), m_entry_ptr);

	int ret = pthread_create(&m_thread, NULL, &Linux_Thread::Start_Routine, this);
	DBG_ASSERT_STR(ret == 0, "Failed to create thread.");	
}

Linux_Thread::Linux_Thread(pthread_t id)
	: Thread(NULL)
	, m_finished(false)
	, m_thread(id)
{
}

Linux_Thread::~Linux_Thread()
{
	m_suspended_semaphore->Signal();

	// Wait for thread to terminate.
	while (!m_finished)
		Sleep(1.0f);

	SAFE_DELETE(m_suspended_semaphore);
}

void* Linux_Thread::Start_Routine(void* arg)
{
	Linux_Thread* thread = reinterpret_cast<Linux_Thread*>(arg);	
	pthread_setspecific(tls_key, thread);

	thread->m_suspended_semaphore->Wait();
	DBG_LOG("Thread 0x%08x '%s' entered (data=0x%08x).", arg, thread->m_name.c_str(), thread->m_entry_ptr);
	thread->m_entry_point(thread, thread->m_entry_ptr);
	DBG_LOG("Thread 0x%08x '%s' exited (data=0x%08x).", arg, thread->m_name.c_str(), thread->m_entry_ptr);
}

bool Linux_Thread::Is_Running()
{
	return !m_finished;
}

void Linux_Thread::Start()
{
	m_suspended_semaphore->Signal();
}

void Linux_Thread::Join()
{
	pthread_join(m_thread, NULL);
}

void Linux_Thread::Set_Priority(ThreadPriority::Type priority)
{
	int min_priority = sched_get_priority_min(SCHED_RR);
	int max_priority = sched_get_priority_max(SCHED_RR);

	float scalar = (float)priority / (float)ThreadPriority::COUNT;

	// Not convinced this is the way to do it ...
	struct sched_param sched_param;
	sched_param.sched_priority = min_priority + ((max_priority - min_priority) * scalar);
	pthread_setschedparam(pthread_self(), SCHED_RR, &sched_param);
}

void Linux_Thread::Set_Affinity(int mask)
{
	cpu_set_t affinity;
	CPU_ZERO(&affinity);

	for (int i = 0, i_mask = 1; i < 32; i++, i_mask *= 2)
	{
		if (mask & i_mask)
		{
			CPU_SET(i, &affinity);
		}
	}

	pthread_setaffinity_np(m_thread, sizeof(cpu_set_t), &affinity);
}

void Linux_Thread::Sleep(float ms)
{
	usleep(ms * 1000);
}

int Linux_Thread::Get_Core_Count()
{
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

void Linux_Thread::Yield()
{
	pthread_yield();
}

#endif