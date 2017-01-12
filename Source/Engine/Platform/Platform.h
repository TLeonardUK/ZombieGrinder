// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PLATFORM_
#define _ENGINE_PLATFORM_

#include "Generic/Patterns/Singleton.h"

#include <string>
#include <vector>

class GameRunner;

// This function is overloaded by game instances to return
// the game runner instance for their game.
//extern GameRunner* New_Game();
extern void Game_Entry_Point(const char* command_line);
extern void Print_Game_Version();

// Debugging structures.
struct StackFrame
{
	int	Address;
};

struct DecodedStackFrame
{
	MEMORY_ALLOCATOR(DecodedStackFrame, "Platform");

public:
	enum
	{
		MAX_STRING_SIZE = 128
	};

	int	  Address;
	char  File[MAX_STRING_SIZE];
	char  Name[MAX_STRING_SIZE];
	int	  Line;
	int	  Column;
};

struct ListDirectoryFlags
{
	enum Type
	{
		Everything,
		FilesOnly,
		DirectoriesOnly
	};
};

class Platform : public Singleton<Platform>
{
	MEMORY_ALLOCATOR(Platform, "Platform");

public:
	static Platform* Create();

	// Time functions.
	virtual double Get_Ticks() = 0;
	virtual void Sleep(float ms) = 0;
	virtual int Get_Timestamp() = 0;

	// Hardware functions.
	virtual int Get_Core_Count() = 0;

	// Path functions.
	virtual void		Crack_Path			(const char* path, std::vector<std::string>& segments) = 0;
	virtual std::string Join_Path			(std::string a, std::string b) = 0;
	virtual std::string Extract_Directory	(std::string a) = 0;
	virtual std::string Extract_Filename	(std::string a) = 0;
	virtual std::string Extract_Basename	(std::string a) = 0;
	virtual std::string Extract_Extension	(std::string a) = 0;
	virtual std::string Get_Absolute_Path	(std::string a) = 0;
	virtual std::string Normalize_Path		(std::string a) = 0;
	virtual bool		Is_Path_Absolute	(std::string a) = 0;
	virtual bool		Is_Path_Relative	(std::string a) = 0;
	virtual std::string	Get_Working_Dir		() = 0;
	virtual void		Set_Working_Dir		(const char* path) = 0;
	virtual bool		List_Directory		(const char* path, std::vector<std::string>& entries, ListDirectoryFlags::Type flags = ListDirectoryFlags::Everything) = 0;
	virtual bool		Copy_File			(const char* from, const char* to) = 0;
	virtual bool		Delete_File			(const char* from) = 0;

	virtual std::string Change_Filename		(std::string original, std::string new_filename);
	virtual std::string Change_Extension	(std::string original, std::string new_filename);

	// Directory functions.
	virtual bool Is_File					(const char* path) = 0;
	virtual bool Is_Directory				(const char* path) = 0;
	virtual bool Create_Directory			(const char* path, bool recursive) = 0;

	// Debug functions.
	virtual int  Get_Stack_Trace			(StackFrame* frames, int max_frames, void* platform_specific = NULL, int offset = 0) = 0;
	virtual void Resolve_Stack_Frame_Info	(StackFrame& frame, DecodedStackFrame& output) = 0;

	// Spawning functionality.
	virtual bool		Spawn_Process		(std::string process, std::string arguments) = 0;

	// GUID
	virtual std::string	Generate_UUID() = 0;

	// Clipboard.
	//virtual std::string GetClipboard() = 0;
	//virtual void SetClipboard(std::string value) = 0;

};

#endif

