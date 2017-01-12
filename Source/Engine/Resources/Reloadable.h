// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_RELOADABLE_
#define _ENGINE_RESOURCES_RELOADABLE_

#include "Generic/Types/LinkedList.h"
#include "Engine/IO/Stream.h"
#include "Engine/IO/FileWatcher.h"

#include <vector>

class GameEngine;

class Reloadable
{
	MEMORY_ALLOCATOR(Reloadable, "Engine");

private:
	static std::vector<Reloadable*> m_reloadables;
	std::vector<FileWatcher*> m_watchers;
	int m_version;

protected:
	void Add_Reload_Trigger_File(const char* path);
	void Reset_Reload_Trigger_Files();
	void Increment_Version();

	static void Check_For_Reloads();

	friend class GameEngine;

public:

	// Constructors
	Reloadable();	
	virtual ~Reloadable();	

	// Derived methods.
	virtual void Reload() = 0;
	int Get_Version();

};

#endif

