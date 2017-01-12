// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_IO_FILEWATCHER_
#define _ENGINE_IO_FILEWATCHER_

#include <string>
#include <vector>

class Thread;
class Mutex;
class Thread;

#ifdef DEBUG_BUILD
#define FILE_WATCHER_INTERVAL	1 * 1000.0f
#define FILE_WATCHER_YIELD_TIME	1.0f
#else
#define FILE_WATCHER_INTERVAL	10 * 1000.0f
#define FILE_WATCHER_YIELD_TIME	1.0f
#endif

class FileWatcher
{
	MEMORY_ALLOCATOR(FileWatcher, "IO");

private:
	static Mutex* g_mutex;
	static Thread* g_thread;
	static std::vector<FileWatcher*> g_file_watchers;

	std::string m_path;
	u64			m_last_change_time;
	bool		m_has_changed;
	double		m_check_time;

public:
	static void Entry_Point(Thread* self, void* ptr);

	FileWatcher(const char* path);
	~FileWatcher();

	bool Has_Changed();

};

#endif