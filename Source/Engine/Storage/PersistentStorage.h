// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STORAGE_PERSISTENT_STORAGE_
#define _ENGINE_STORAGE_PERSISTENT_STORAGE_

#include "Generic/Patterns/Singleton.h"

#include <string>
#include <vector>

class GameRunner;

class PersistentStorage : public Singleton<PersistentStorage>
{
	MEMORY_ALLOCATOR(PersistentStorage, "Engine");

public:
	static PersistentStorage* Create();

	virtual bool File_Exists(const char* url) = 0;
	virtual int  File_Size  (const char* url) = 0;

	virtual bool Write_File(const char* url, void* data, int size) = 0;
	virtual int  Read_File (const char* url, void* data, int size) = 0;

	virtual void Wait_For_Pending_Writes() = 0;

};

#endif

