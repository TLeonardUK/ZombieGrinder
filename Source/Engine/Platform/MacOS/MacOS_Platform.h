// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_PLATFORM_MACOS_PLATFORM_
#define _ENGINE_PLATFORM_MACOS_PLATFORM_

#include "Generic/Patterns/Singleton.h"
#include "Engine/Platform/Platform.h"

class MacOS_Platform : public Platform
{
	MEMORY_ALLOCATOR(MacOS_Platform, "Platform");

public:

	// Time functions.
	double Get_Ticks();
	void Sleep(float ms);
	int Get_Timestamp();

	// Hardware functions.
	int Get_Core_Count();

	// Path functions.
	void Crack_Path(const char* path, std::vector<std::string>& segments);
	std::string Join_Path(std::string a, std::string b);
	std::string Extract_Directory(std::string a);
	std::string Extract_Filename(std::string a);
	std::string Extract_Basename(std::string a);
	std::string Extract_Extension(std::string a);
	std::string Get_Absolute_Path(std::string a);
	std::string Normalize_Path(std::string a);
	bool Is_Path_Absolute(std::string a);
	bool Is_Path_Relative(std::string a);
	std::string	Get_Working_Dir();
	void Set_Working_Dir(const char* path);
	bool List_Directory(const char* path, std::vector<std::string>& entries, ListDirectoryFlags::Type flags = ListDirectoryFlags::Everything);
	bool Copy_File(const char* from, const char* to);
	bool Delete_File(const char* from);

	// Directory functions.
	bool Is_File(const char* path);
	bool Is_Directory(const char* path);
	bool Create_Directory(const char* path, bool recursive);

	// Debug functions.
	int Get_Stack_Trace(StackFrame* frames, int max_frames, void* platform_specific = NULL, int offset = 0);
	void Resolve_Stack_Frame_Info(StackFrame& frame, DecodedStackFrame& output);

	// Process functions.
	bool Spawn_Process(std::string process, std::string arguments);

	// GUID
	std::string	Generate_UUID();

private:
	friend class Platform;

	MacOS_Platform();

};

#endif

#endif