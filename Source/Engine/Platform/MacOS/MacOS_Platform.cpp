// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/Platform/MacOS/MacOS_Platform.h"
#include "Generic/Helper/StringHelper.h"

#include "Engine/Display/GfxDisplay.h"

#include "Generic/Threads/Thread.h"

//#include "Engine/Engine/GameEngine.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <stdarg.h>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <time.h>
#include <CoreServices/CoreServices.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <time.h>
#include <uuid/uuid.h>
#include <ctime>
#include <stdlib.h>

#include "Engine/Platform/MacOS/MacOS_EntryPoint.h"

using namespace abi;

extern char* g_executable_file_path;

static FILE* g_log_file = NULL;
static bool g_log_to_file = false;

void Platform_Enable_Log_File()
{
	g_log_to_file = true;
}

// These functions are declared in Define.h and are used through the project to 
// emit debug text.
void Platform_Print(const char* format, ...)
{
	// In master builds we disable much of this output for performance reasons.
#ifdef MASTER_BUILD
#ifdef OPT_OUTPUT_TO_LOG_FILE
	if (!g_log_to_file)
	{
		return;
	}
#else
	return;
#endif
#endif

	char buffer[2048];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 2048, format, args);
	va_end(args);

 	buffer[2047] = '\0';

	printf("%s\n", buffer);

#ifdef OPT_OUTPUT_TO_LOG_FILE
	if (g_log_to_file)
	{
		if (g_log_file == NULL)
		{
			g_log_file = fopen("game.log", "a+");
		}

		fwrite(buffer, 1, strlen(buffer), g_log_file);
		fwrite("\n", 1, 1, g_log_file);
	}
#endif
}

void Platform_Fatal(bool graceful, const char* format, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	
	// Break into debugger first if possible.
#ifdef DEBUG_BUILD
 	raise(SIGTRAP);
#endif

	// Show a tty message box explaining the problem.
	printf("Fatal Error:\n", buffer);
	printf("%s\n", buffer);

	// Show alert panel.
	if (GfxDisplay::Try_Get())
	{
		GfxDisplay::Get()->Set_Hidden(true);
	}

	Show_Alert("Fatal Error", buffer);

	// Gracefully exit?
	if (graceful)
	{
		_exit(0);
		return;
	}

	// Cause a crash to boot our core-dump code into action.
	int* x = 0x0;
	*x = 0;

	// Still ok? Dafuck, ok lets just exit quietly.
	exit(1);
}

void Platform_Exit()
{
	_exit(0);
}

MacOS_Platform::MacOS_Platform()
{
}

void MacOS_Platform::Sleep(float ms)
{
	usleep(ms * 1000);
}

int MacOS_Platform::Get_Timestamp()
{
    time_t ltime;
    time(&ltime); 

	return ltime;
}

double MacOS_Platform::Get_Ticks()
{
	return platform_get_ticks();
}

void MacOS_Platform::Crack_Path(const char* path, std::vector<std::string>& segments)
{
	StringHelper::Split(path, '/', segments);
}

std::string MacOS_Platform::Join_Path(std::string a, std::string b)
{
	if (a[a.size() - 1] == '/' || a[a.size() - 1] == '\\' ||
		b[0]			== '/' || b[0]			  == '\\')
	{
		return a + b;
	}
	else
	{
		return a + "/" + b;
	}
}

std::string MacOS_Platform::Normalize_Path(std::string value)
{
	value = StringHelper::Replace(value.c_str(), "\\", "/"); // Turn backslashes into forward slashes.
	value = StringHelper::Replace(value.c_str(), "//", "/"); // Remove duplicate path seperators.
	return value;
}

bool MacOS_Platform::Is_Path_Absolute(std::string value)
{
	return !Is_Path_Relative(value);
}

bool MacOS_Platform::Is_Path_Relative(std::string value)
{
	if (value.size() <= 1)
	{
		return true;
	}

	if (value.at(0) != '/')
	{
		return true;
	}

	return false;
}

std::string MacOS_Platform::Get_Absolute_Path(std::string value)
{	
	// Add current directory.
	if (Is_Path_Relative(value) == true)
	{
		value = Get_Working_Dir() + "/" + value;
	}

	value = Normalize_Path(value);
	
	// Strip out all .. and . references.
	std::vector<std::string> crackedPath;
	StringHelper::Split(value.c_str(), '/', crackedPath);

	std::string				 finalPath   = "";
	int						 skip_count = 0;

	for (int i = crackedPath.size() - 1; i >= 0; i--)
	{
		std::string part = crackedPath.at(i);

		if (part == "..")
		{
			skip_count++;
		}
		else if (part == ".")
		{
			continue;
		}
		else
		{
			if (skip_count > 0)
			{
				skip_count--;
				continue;
			}

			if (finalPath == "")
			{
				finalPath = part;
			}
			else
			{
				finalPath = part + "/" + finalPath;
			}
		}
	}
	
	if (value[value.size() - 1] == '/')
	{
		finalPath += "/";
	}

	return finalPath;
}

std::string MacOS_Platform::Extract_Directory(std::string a)
{
	int index = a.find_last_of("/\\");
	if (index == std::string::npos)
	{
		return a;
	}
	else
	{
		return a.substr(0, index);
	}
}

std::string MacOS_Platform::Extract_Filename(std::string a)
{
	int index = a.find_last_of("/\\");
	if (index == std::string::npos)
	{
		return a;
	}
	else
	{
		return a.substr(index + 1);
	}
}

std::string MacOS_Platform::Extract_Basename(std::string a)
{
	int index = a.find_last_of("/\\");
	int period_index = a.find_last_of(".");

	if (index != std::string::npos && period_index != std::string::npos && period_index > index)
	{
		return a.substr(index + 1, (period_index - index) - 1);
	}
	else if (index != std::string::npos)
	{
		return a.substr(index + 1);
	}
	else if (period_index != std::string::npos)
	{
		return a.substr(0, (period_index - index) - 1);
	}
	else
	{
		return a;
	}
}

std::string MacOS_Platform::Extract_Extension(std::string a)
{
	int index = a.find_last_of(".");
	if (index == std::string::npos)
	{
		return "";
	}
	else
	{
		return a.substr(index + 1);
	}
}

bool MacOS_Platform::Is_File(const char* path)
{
	if (access(path, 0) != 0)
	{
		return false;
	}

	struct stat status;
	stat(path, &status);
	
	if ((status.st_mode & S_IFDIR) != 0)
	{
		return false;
	}	

	return true;
}

bool MacOS_Platform::Is_Directory(const char* path)
{
	if (access(path, 0) != 0)
	{
		return false;
	}

	struct stat status;
	stat(path, &status);

	if ((status.st_mode & S_IFDIR) == 0)
	{
		return false;
	}	

	return true;
}

bool MacOS_Platform::Create_Directory(const char* path, bool recursive)
{
	if (recursive == true)
	{
		std::vector<std::string> cracked;
		std::string crack_path = "";

		Crack_Path(path, cracked);
		
		for (int i = 0; i < (int)cracked.size(); i++)
		{
			if (crack_path != "")
			{
				crack_path += "/";
			}
			crack_path += cracked.at(i);
			
			if (!Is_Directory(crack_path.c_str()))
			{
				bool result = Create_Directory(crack_path.c_str(), false);
				if (result == false)
				{
					return false;
				}
			}
		}

		return true;
	}
	else
	{
		mkdir(path, 0777);
		return Is_Directory(path);
	}
}

std::string	MacOS_Platform::Get_Working_Dir()
{
	char buffer[512];
	getcwd(buffer, 512);
	return std::string(buffer);
}

void MacOS_Platform::Set_Working_Dir(const char* path)
{
	chdir(path);
}

bool MacOS_Platform::Delete_File(const char* from)
{
	return unlink(from) == 0;
}

bool MacOS_Platform::Copy_File(const char* src, const char* dst)
{
	if (Is_File(dst) == true)
	{
		unlink(dst);
	}

	int source_fd = open(src, O_RDWR);
	int dest_fd   = open(dst, O_RDWR|O_CREAT|O_TRUNC, 0777);  

	struct stat stats;
	fstat(source_fd, &stats);
	
    char buf[1024];
    size_t size;

	while ((size = read(source_fd, buf, 1024)) > 0) 
	{
        write(dest_fd, buf, size);
    }

	close(source_fd);
	close(dest_fd);
}

int MacOS_Platform::Get_Stack_Trace(StackFrame* frames, int max_frames, void* platform_specific, int offset)
{	
	static int MAX_STACK_FRAMES = 512;

	DBG_ASSERT(max_frames <= MAX_STACK_FRAMES);	
	memset(frames, 0, sizeof(StackFrame) * max_frames);

	void* stack_frames[MAX_STACK_FRAMES];
	int frame_count = backtrace(stack_frames, max_frames);

	// We increment offset so we don't get this frame.
	offset++;

	for (int i = offset; i < frame_count; i++)
	{
		StackFrame& frame	= frames[i - offset];
		frame.Address		= reinterpret_cast<int*>(stack_frames)[i];
	}

	return frame_count - offset;
}

void MacOS_Platform::Resolve_Stack_Frame_Info(StackFrame& frame, DecodedStackFrame& output)
{	
	int status = 0;
	size_t size = 0;
    Dl_info symbol_info;

	void* void_addr = reinterpret_cast<void*>(frame.Address);
	
	output.Name[0] = '\0';
	output.File[0] = '\0';
	output.Line    = 0;
	output.Column  = 0;

    if (!dladdr(void_addr, &symbol_info))
        return;
	
	// Get pointer difference.
	ptrdiff_t offset = (char*)void_addr - (char*)symbol_info.dli_saddr;

	// Demangle and copy symbol name.
	char* demangled = __cxa_demangle(symbol_info.dli_sname, NULL, &size, &status);
	if (demangled != NULL && status == 0)
	{			
		strncpy(output.Name, demangled, size);
	}
	else
	{
		strcpy(output.Name, symbol_info.dli_sname);
	}

	// Append the offset to the filename.
	if (strlen(output.Name) == 0)
	{
		sprintf(output.Name, "0x%08x+0x%08x [0x%08x]", reinterpret_cast<int>(void_addr), (int)(offset), reinterpret_cast<int>(void_addr));
	}
	else
	{
		sprintf(output.Name, "%s+0x%08x [0x%08x]", output.Name, (int)(offset), reinterpret_cast<int>(void_addr));
	}
	
	// Copy filename.
	strcpy(output.File, symbol_info.dli_fname);
	if (strlen(output.File) == 0)
	{
		strcpy(output.File, "<unknown>");
	}
}

bool MacOS_Platform::List_Directory(const char* path, std::vector<std::string>& entries, ListDirectoryFlags::Type flags)
{
	std::string value = path ;
	if (value.size() == 0 || value.at(value.size() - 1) != '/')
	{
		value += "/";
	}
	
	DIR*			dir;
	struct dirent*	file;

	dir = opendir(value.c_str());
	if (dir != NULL)
	{
		while (true)
		{
			file = readdir(dir);
			if (file == NULL)
			{
				break;
			}
			
			std::string filename = file->d_name;
			std::string full_path = value + filename;
			if (filename != "." && filename != "..")
			{			
				if (Is_File(full_path.c_str()))
				{
					if (flags == ListDirectoryFlags::Everything || flags == ListDirectoryFlags::FilesOnly)
					{
						entries.push_back(file->d_name);
					}
				}
				else
				{
					if (flags == ListDirectoryFlags::DirectoriesOnly)
					{
						entries.push_back(file->d_name);
					}
				}
			}
		}
		closedir(dir);
	}
	else
	{
		return false;
	}

	return true;
}

int MacOS_Platform::Get_Core_Count()
{
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

bool MacOS_Platform::Spawn_Process(std::string path, std::string arguments)
{	
	pid_t pid = fork();
	int status;

	switch (pid) 
	{
	case -1: 
		return 0;

	case 0: 
		execl(path.c_str(), path.c_str(), arguments.c_str(), NULL); 
		exit(1);

	default: 
		while (!WIFEXITED(status)) 
		{
			waitpid(pid, &status, 0); 
		}

		return (WEXITSTATUS(status) == 0);
	}
}

std::string	MacOS_Platform::Generate_UUID()
{
	uuid_t uuid;
	uuid_generate_random(uuid);

	char result[37];
	uuid_unparse(uuid, result);

	return result;
}

#endif