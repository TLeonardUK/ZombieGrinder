// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/IO/FileWatcher.h"
#include "Engine/IO/StreamFactory.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Platform/Platform.h"

#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include <algorithm>

Mutex* FileWatcher::g_mutex = NULL;
Thread* FileWatcher::g_thread = NULL;
std::vector<FileWatcher*> FileWatcher::g_file_watchers;

void FileWatcher::Entry_Point(Thread* self, void* ptr)
{
	while (true)
	{
		{
			MutexLock lock(g_mutex);

			for (std::vector<FileWatcher*>::iterator iter = g_file_watchers.begin(); iter != g_file_watchers.end(); iter++)
			{
				FileWatcher* watcher = *iter;

				// Previous state change not accepted yet, so ignore.
				if (watcher->m_has_changed == true)
				{
					continue;
				}

				// Time to recheck?
				double elapsed = (Platform::Get()->Get_Ticks() - watcher->m_check_time);
				if (watcher->m_check_time == 0.0f || elapsed > FILE_WATCHER_INTERVAL)
				{
					u64 old = watcher->m_last_change_time;
					watcher->m_last_change_time = StreamFactory::Get_Last_Modified(watcher->m_path.c_str());
					watcher->m_has_changed = (old != 0 && old != watcher->m_last_change_time);
					watcher->m_check_time = Platform::Get()->Get_Ticks();
				}
			}
		}

		// Yield time, we are not an important thread :)
		self->Sleep(FILE_WATCHER_YIELD_TIME);
	}
}

FileWatcher::FileWatcher(const char* path)
	: m_path(path)
	, m_has_changed(false)
	, m_last_change_time(0)
	, m_check_time(0.0f)
{
	if (g_thread == NULL)
	{
		g_mutex = Mutex::Create();

		g_thread = Thread::Create("File Watcher", Entry_Point, NULL);
		g_thread->Set_Priority(ThreadPriority::Low);
		g_thread->Start();
	}

	{
		MutexLock lock(g_mutex);
		g_file_watchers.push_back(this);
	}

	DBG_LOG("File watcher created for: %s", path);
}

FileWatcher::~FileWatcher()
{
	{
		MutexLock lock(g_mutex);
		g_file_watchers.erase(std::find(g_file_watchers.begin(), g_file_watchers.end(), this));
	}
}

bool FileWatcher::Has_Changed()
{
	bool has_changed = m_has_changed;
	m_has_changed = false;
	return has_changed;
}

