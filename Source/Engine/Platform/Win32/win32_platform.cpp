// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Platform/Win32/Win32_Platform.h"
#include "Generic/Helper/StringHelper.h"

#include "Engine/Display/GfxDisplay.h"

#include "Generic/Threads/Thread.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Renderer/Textures/PNG/PNGPixelmapFactory.h"
#include "Engine/Renderer/Text/FreeType/FreeType_FontFactory.h"
#include "Engine/IO/Win32/Win32_FileStreamFactory.h"
#include "Engine/Audio/FMod/FMod_SoundFactory.h"

#include <windows.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <setjmp.h>

#pragma warning(push)
#pragma warning(disable:4091) // 2015 introduced some extra warnings about typedef's, dbghelp.h/shlobj.h now spams them :(
#include <dbghelp.h>
#include <Shlobj.h>
#pragma warning(pop)

#include <stdarg.h>
#include <time.h>
#include <rpc.h>

#include <cstdio>

#include "Engine/Platform/Win32/Win32_EntryPoint.h"

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


	va_list va;
	va_start(va, format);

	char static_buffer[512];
	char* buffer = static_buffer;
	int num = vsnprintf(buffer, 512, format, va);
	if (num >= 511)
	{
		buffer = new char[num + 1];
		vsnprintf(buffer, num + 1, format, va);
	}

	va_end(va);

#if defined(OPT_OUTPUT_TO_LOG_FILE) && defined(MASTER_BUILD)
	if (g_log_to_file)
	{
#endif
		OutputDebugStringA(buffer);
		OutputDebugStringA("\n");
#if defined(OPT_OUTPUT_TO_LOG_FILE) && defined(MASTER_BUILD)
	}
#endif

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

	if (buffer != static_buffer)
	{
		delete[] buffer;
	}
}

void Platform_Fatal(bool graceful, const char* format, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args); // Buffer overflow, fix plz.
	va_end(args);
	
	// Break into debugger first if possible.
#ifdef DEBUG_BUILD
  	__debugbreak();
#endif

	// Show a message box explaining the problem.
	if (GfxDisplay::Try_Get() && GetCurrentThreadId() == g_main_thread_id)
	{
		GfxDisplay::Get()->Set_Hidden(true);
	}

	MessageBoxA(NULL, buffer, "Fatal Error", MB_OK|MB_ICONSTOP);

	// Gracefully exit?
	if (graceful)
	{
		_exit(0);
		return;
	}

	// Cause a crash to boot our minidump cleanup code into action.
	int* x = 0x0;
	*x = 0;

	// Still ok? Dafuck, ok lets just exit quietly.
	exit(1);
}

void Platform_Exit()
{
	_exit(0);
}

Win32_Platform::Win32_Platform()
{
}

void Win32_Platform::Sleep(float ms)
{
	::Sleep((int)ms);
}

int Win32_Platform::Get_Timestamp()
{
    time_t ltime;
    time(&ltime); 

    struct tm* timeinfo = gmtime(&ltime); 
    ltime = mktime(timeinfo); 

	return (int)ltime;
}

double Win32_Platform::Get_Ticks()
{
	static LARGE_INTEGER freq;			
	static LARGE_INTEGER start;
	static int state = 0; // 0=not initialized, -1=not supported, 1=supported

	if (state == 0)
	{
		if (QueryPerformanceFrequency(&freq) == 0)
		{
			state = -1;
		}
		else
		{
			QueryPerformanceCounter(&start);
			state = 1;
		}
	}

	// Not supported, use tick count.
	if (state == -1)
	{
		return (double)GetTickCount();
	}
	else
	{
		LARGE_INTEGER tickCount;
		QueryPerformanceCounter(&tickCount);

		double f = (double)freq.QuadPart / 1000.0f;
		return double(tickCount.QuadPart - start.QuadPart) / f;
	}	
}

void Win32_Platform::Crack_Path(const char* path, std::vector<std::string>& segments)
{
	StringHelper::Split(path, '/', segments);
}

std::string Win32_Platform::Join_Path(std::string a, std::string b)
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

std::string Win32_Platform::Normalize_Path(std::string value)
{
	value = StringHelper::Replace(value.c_str(), "\\", "/"); // Turn backslashes into forward slashes.
	value = StringHelper::Replace(value.c_str(), "//", "/"); // Remove duplicate path seperators.
	return value;
}

bool Win32_Platform::Is_Path_Absolute(std::string value)
{
	return !Is_Path_Relative(value);
}

bool Win32_Platform::Is_Path_Relative(std::string value)
{
	if (value.size() <= 2)
	{
		return true;
	}

	if (value.at(1) != ':')
	{
		return true;
	}

	return false;
}

std::string Win32_Platform::Get_Absolute_Path(std::string value)
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

std::string Win32_Platform::Extract_Directory(std::string a)
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

std::string Win32_Platform::Extract_Filename(std::string a)
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

std::string Win32_Platform::Extract_Basename(std::string a)
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

std::string Win32_Platform::Extract_Extension(std::string a)
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

bool Win32_Platform::Is_File(const char* path)
{
	DWORD flags = GetFileAttributesA(path);

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
		return false;
	}

	return true;
}

bool Win32_Platform::Is_Directory(const char* path)
{
	DWORD flags = GetFileAttributesA(path);

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}

	return true;
}

bool Win32_Platform::Create_Directory(const char* path, bool recursive)
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
		int result = CreateDirectoryA(path, NULL);
		return (result != 0);
	}
}

std::string	Win32_Platform::Get_Working_Dir()
{
    char path[512];
    
	if (GetCurrentDirectoryA(512, path) == 0)
    {
        return "";
    }

    return path;
}

void Win32_Platform::Set_Working_Dir(const char* path)
{
	SetCurrentDirectoryA(path);
}

bool Win32_Platform::Copy_File(const char* from, const char* to)
{
	BOOL ret = CopyFileA(from, to, false);
	return ret != 0;
}

bool Win32_Platform::Delete_File(const char* from)
{
	BOOL ret = DeleteFileA(from);
	return ret != 0;
}

int Win32_Platform::Get_Stack_Trace(StackFrame* frames, int max_frames, void* platform_specific, int offset)
{	
	DBG_ASSERT(max_frames <= 512);	
	memset(frames, 0, sizeof(StackFrame) * max_frames);

	int frame_count = 0;

	if (platform_specific != NULL)
	{
		// If thread is not given, use current.
		HANDLE thread = GetCurrentThread();
		struct _EXCEPTION_POINTERS* exceptioninfo = reinterpret_cast<struct _EXCEPTION_POINTERS *>(platform_specific);

		// Capture context for this thread.
		CONTEXT context;
		ZeroMemory(&context, sizeof(CONTEXT));
	
		// 32bit has no function call to get the current
		// threads context, so its asm trickery time :3
		#ifdef PLATFORM_32BIT
		
			if (exceptioninfo != NULL)
			{
				memcpy(&context, exceptioninfo->ContextRecord, sizeof(CONTEXT));
			}
			else
			{
				context.ContextFlags = CONTEXT_CONTROL;	
				__asm
				{
				Label:
					mov [context.Ebp], ebp;
					mov [context.Esp], esp;
					mov eax, [Label];
					mov [context.Eip], eax;
				}
			}

		// 64bit does though, w00t.
		#else
	
			if (exceptioninfo != NULL)
			{
				memcpy(&context, exceptioninfo->ContextRecord, sizeof(CONTEXT));
			}
			else
			{
				RtlCaptureContext(&context);
			}

		#endif
			
		// Build the initial stack frame.
		STACKFRAME64 stackFrame;
		DWORD		 machineType;
		ZeroMemory(&stackFrame, sizeof(STACKFRAME64));
		
		#ifdef PLATFORM_X86
			machineType                 = IMAGE_FILE_MACHINE_I386;
			stackFrame.AddrPC.Offset    = context.Eip;
			stackFrame.AddrPC.Mode      = AddrModeFlat;
			stackFrame.AddrFrame.Offset = context.Ebp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
			stackFrame.AddrStack.Offset = context.Esp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
		#elif PLATFORM_AMD64
			machineType                 = IMAGE_FILE_MACHINE_AMD64;
			stackFrame.AddrPC.Offset    = context.Rip;
			stackFrame.AddrPC.Mode      = AddrModeFlat;
			stackFrame.AddrFrame.Offset = context.Rsp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
			stackFrame.AddrStack.Offset = context.Rsp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
		#elif PLATFORM_ITANIUM64
			machineType                 = IMAGE_FILE_MACHINE_IA64;
			stackFrame.AddrPC.Offset    = context.StIIP;
			stackFrame.AddrPC.Mode      = AddrModeFlat;
			stackFrame.AddrFrame.Offset = context.IntSp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;
			stackFrame.AddrBStore.Offset= context.RsBSP;
			stackFrame.AddrBStore.Mode  = AddrModeFlat;
			stackFrame.AddrStack.Offset = context.IntSp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
		#else
			DBG_LOG("Platform does not support stack walking.");
			return 0;

		#endif

		EnterCriticalSection(&g_dbghelp_critical_section);
		
		// Stack trace!
		int trace_offset = 0;
		while (frame_count < max_frames)
		{
			if (!StackWalk64(machineType, GetCurrentProcess(), thread, &stackFrame, 
								&context, 
								NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
			{
				// Failure :(
				break;
			}

			if (++trace_offset <= offset)
			{
				continue;
			}
		
			if (stackFrame.AddrPC.Offset != 0)
			{
		#ifdef PLATFORM_64BIT
				frames[frame_count].Address = stackFrame.AddrPC.Offset;
		#else
				frames[frame_count].Address = (int)stackFrame.AddrPC.Offset;
		#endif
				frame_count++;
			}
			else
			{
				// Reached the base of the stack.
				break;
			}
		}
		
		LeaveCriticalSection(&g_dbghelp_critical_section);
	}
	else
	{
		void* sys_frames[512];
		frame_count = CaptureStackBackTrace(1 + offset, max_frames, sys_frames, NULL);
		for (int i = 0; i < frame_count; i++)
		{
			frames[i].Address = (u64)sys_frames[i];
		}
	}

	return frame_count;
}

void Win32_Platform::Resolve_Stack_Frame_Info(StackFrame& frame, DecodedStackFrame& output)
{
	DWORD64 displacement = 0;

	char buffer[sizeof(SYMBOL_INFO) + DecodedStackFrame::MAX_STRING_SIZE * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol	= (PSYMBOL_INFO)buffer;
	pSymbol->SizeOfStruct	= sizeof(SYMBOL_INFO);
	pSymbol->MaxNameLen		= DecodedStackFrame::MAX_STRING_SIZE;

	sprintf_s(output.Name, DecodedStackFrame::MAX_STRING_SIZE, "0x%.16x", output.Address);

	EnterCriticalSection(&g_dbghelp_critical_section);

	if (SymFromAddr(GetCurrentProcess(), frame.Address, &displacement, pSymbol) == TRUE)
	{
		strcpy_s(output.Name, DecodedStackFrame::MAX_STRING_SIZE, pSymbol->Name);

		IMAGEHLP_LINE64 line;
		DWORD lineDisplacement;
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		if (SymGetLineFromAddr64(GetCurrentProcess(), frame.Address, &lineDisplacement, &line))
		{
			output.Line = line.LineNumber;
			strcpy_s(output.File, DecodedStackFrame::MAX_STRING_SIZE, line.FileName);
		}
		else
		{
			output.Line = 0;
			strcpy_s(output.File, DecodedStackFrame::MAX_STRING_SIZE, "<unknown>");
		}
	}
	else
	{
		s32 err = GetLastError();
		strcpy_s(output.File, DecodedStackFrame::MAX_STRING_SIZE, "<unknown>");
		output.Line = 0;
	}
			
	LeaveCriticalSection(&g_dbghelp_critical_section);
}

bool Win32_Platform::List_Directory(const char* path, std::vector<std::string>& entries, ListDirectoryFlags::Type flags)
{
	std::string value = path ;
	if (value.size() == 0 || value.at(value.size() - 1) != '/')
	{
		value += "/";
	}
	
	WIN32_FIND_DATAA			data;
	HANDLE						handle;

	handle = FindFirstFileA((value + "*").c_str(), &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		while (true)
		{
			std::string filename = data.cFileName;
			std::string full_path = value + filename;
			if (filename != "." && filename != "..")
			{			
				if (Is_File(full_path.c_str()))
				{
					if (flags == ListDirectoryFlags::Everything || flags == ListDirectoryFlags::FilesOnly)
					{
						entries.push_back(data.cFileName);
					}
				}
				else
				{
					if (flags == ListDirectoryFlags::DirectoriesOnly)
					{
						entries.push_back(data.cFileName);
					}
				}
			}

			if (FindNextFileA(handle, &data) == 0)
			{
				break;
			}
		}
		FindClose(handle);
	}
	else
	{
		return false;
	}

	return true;
}

int Win32_Platform::Get_Core_Count()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return sysinfo.dwNumberOfProcessors; // Note: This is logical processors (aka cores) not physical ones!
}

bool Win32_Platform::Spawn_Process(std::string process, std::string arguments)
{
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFOA		si = { sizeof(si) };

	if (Is_Path_Relative(process))
	{
		char exe_path[512];
		GetModuleFileName(NULL, exe_path, 512);

		process = Extract_Directory(exe_path) + "/" + process;
	}

	std::string dir = Extract_Directory(process);
	std::string working_dir = Get_Working_Dir();

	if (!CreateProcessA(NULL, (LPSTR)("\"" + process + "\" " + arguments).c_str(), NULL, NULL, false, CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_CONSOLE, NULL, (LPSTR)working_dir.c_str(), &si, &pi)) 
	{
		int err = GetLastError();
		return false;		
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return true;

//	WaitForSingleObject(pi.hProcess, INFINITE);

//	int res = GetExitCodeProcess(pi.hProcess, (DWORD*)&res) ? res : -1;

//	CloseHandle(pi.hProcess);
//	CloseHandle(pi.hThread);

//	return (res == 0);
}


std::string	Win32_Platform::Generate_UUID()
{
	UUID uuid;
	unsigned char * str;

	RPC_STATUS status;
	status = UuidCreate(&uuid);

	if (status == RPC_S_OK || status == RPC_S_UUID_LOCAL_ONLY)
	{
		status = UuidToStringA(&uuid, &str);
		if (status == RPC_S_OK)
		{
			std::string result((char*)str);
			RpcStringFreeA(&str);

			return result;
		}	
		else
		{
			return "";
		}
	}

	return "";
}

#endif