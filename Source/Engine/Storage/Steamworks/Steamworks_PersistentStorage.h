// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STORAGE_STEAMWORKS_PERSISTENT_STORAGE_
#define _ENGINE_STORAGE_STEAMWORKS_PERSISTENT_STORAGE_

#include "Generic/Patterns/Singleton.h"
#include "Engine/Storage/PersistentStorage.h"
#include "Generic/Types/DataBuffer.h"
#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"
#include "Generic/Threads/Semaphore.h"

#include <vector>
#include <string>

struct Steamworks_PersistentStoragePendingWrite
{
	std::string filename;
	DataBuffer	buffer;
};

class Steamworks_PersistentStorage : public PersistentStorage
{
	MEMORY_ALLOCATOR(Steamworks_PersistentStorage, "Engine");

public:
	Mutex* m_pending_writes_mutex;
	Thread* m_pending_writes_thread;
	Semaphore* m_pending_writes_semaphore;
	std::vector<Steamworks_PersistentStoragePendingWrite> m_pending_writes;
	bool m_closing;

private:
	friend class PersistentStorage;

	static void EntryPoint(Thread* self, void* ptr);

	Steamworks_PersistentStorage();
	virtual ~Steamworks_PersistentStorage();
	
	bool File_Exists(const char* url);
	int File_Size  (const char* url);

	bool Write_File(const char* url, void* data, int size);
	int  Read_File (const char* url, void* data, int size);

	void Wait_For_Pending_Writes();

};

#endif

