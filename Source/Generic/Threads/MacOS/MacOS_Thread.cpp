// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Generic/Threads/MacOS/MacOS_Thread.h"
#include "Generic/Threads/MacOS/MacOS_Mutex.h"
#include "Generic/Threads/MacOS/MacOS_Semaphore.h"
#include "Generic/Threads/MutexLock.h"

#include <unistd.h>
#include <time.h>
#include <CoreServices/CoreServices.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <pthread.h>
#include <sched.h>

// fucking macos dosen't support TLS keywords ;_;
static pthread_key_t tls_key = NULL;

MacOS_Thread* MacOS_Thread::Get_Current_Thread()
{
	return (MacOS_Thread*)pthread_getspecific(tls_key);
}

void MacOS_Thread::Setup_Main()
{	
	pthread_key_create(&tls_key, NULL);

	MacOS_Thread* thread = new MacOS_Thread(pthread_self());
	thread->Set_Name("Main Thread");
	pthread_setspecific(tls_key, thread);
}

MacOS_Thread::MacOS_Thread(const char* name, EntryPoint entry_point, void* ptr) 
	: Thread(name) 
	, m_finished(false)
	, m_thread(0)
{
	m_entry_point			= entry_point;
	m_entry_ptr				= ptr;
	m_suspended_semaphore	= Semaphore::Create();
	
	DBG_LOG("Thread 0x%08x '%s' created (data=0x%08x).", this, m_name.c_str(), m_entry_ptr);

	int ret = pthread_create(&m_thread, NULL, &MacOS_Thread::Start_Routine, this);
	DBG_ASSERT_STR(ret == 0, "Failed to create thread.");	
}

MacOS_Thread::MacOS_Thread(pthread_t id)
	: Thread(NULL)
	, m_finished(false)
	, m_thread(id)
{
}

MacOS_Thread::~MacOS_Thread()
{
	m_suspended_semaphore->Signal();

	// Wait for thread to terminate.
	while (!m_finished)
		Sleep(1.0f);

	SAFE_DELETE(m_suspended_semaphore);
}

void* MacOS_Thread::Start_Routine(void* arg)
{
	MacOS_Thread* thread = reinterpret_cast<MacOS_Thread*>(arg);	
	pthread_setspecific(tls_key, thread);

	thread->m_suspended_semaphore->Wait();
	DBG_LOG("Thread 0x%08x '%s' entered (data=0x%08x).", arg, thread->m_name.c_str(), thread->m_entry_ptr);
	thread->m_entry_point(thread, thread->m_entry_ptr);
	DBG_LOG("Thread 0x%08x '%s' exited (data=0x%08x).", arg, thread->m_name.c_str(), thread->m_entry_ptr);
}

bool MacOS_Thread::Is_Running()
{
	return !m_finished;
}

void MacOS_Thread::Start()
{
	m_suspended_semaphore->Signal();
}

void MacOS_Thread::Join()
{
	pthread_join(m_thread, NULL);
}

void MacOS_Thread::Set_Priority(ThreadPriority::Type priority)
{
	int min_priority = sched_get_priority_min(SCHED_RR);
	int max_priority = sched_get_priority_max(SCHED_RR);

	float scalar = (float)priority / (float)ThreadPriority::COUNT;

	// Not convinced this is the way to do it ...
	struct sched_param sched_param;
	sched_param.sched_priority = min_priority + ((max_priority - min_priority) * scalar);
	pthread_setschedparam(pthread_self(), SCHED_RR, &sched_param);
}

void MacOS_Thread::Set_Affinity(int mask)
{
	// thread_affinity_policy_data_t / thread_policy_set
	// Dosen't seem like macos's affinity settings really do shit. So ignoring for now?
}

void MacOS_Thread::Sleep(float ms)
{
	usleep(ms * 1000);
}

int MacOS_Thread::Get_Core_Count()
{
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

void MacOS_Thread::Yield()
{
	sched_yield();
}

#endif