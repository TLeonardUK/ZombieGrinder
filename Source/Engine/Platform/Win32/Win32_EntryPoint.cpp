// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Platform/Win32/Win32_Platform.h"
#include "Generic/Helper/StringHelper.h"

#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Win32/Win32_Thread.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/GameRunner.h"

#include "Engine/Renderer/Textures/PNG/PNGPixelmapFactory.h"
#include "Engine/Renderer/Text/FreeType/FreeType_FontFactory.h"
#include "Engine/IO/Win32/Win32_FileStreamFactory.h"
#include "Engine/Audio/FMod/FMod_SoundFactory.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Math/Random.h"

#include "Engine/Version.h"

#include <winsock2.h>
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

#include <cstdio>

#if defined(OPT_STEAM_PLATFORM) && defined(MASTER_BUILD)
#include "public/steam/steam_api.h"
#endif

jmp_buf			 g_error_recovery_longjmp;
CRITICAL_SECTION g_dbghelp_critical_section;
u32			     g_main_thread_id			= 0;
HANDLE			 g_main_thread_handle		= NULL;

std::string FormatSystemError(DWORD error)
{
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    std::string result = (const char*)lpMsgBuf;

    LocalFree(lpMsgBuf);

    return result;
}

LONG WINAPI ExceptionHandler(struct _EXCEPTION_POINTERS *exceptionInfo)
{
	Platform* platform = Platform::Get();

	// DEBUG - wait for debugger attach.
	/*while (true)
	{
		Platform::Get()->Sleep(10.0f);
	}*/

	std::string dump_dir  = platform->Get_Working_Dir() + "\\.crashes";
    std::string dump_path = dump_dir + "\\00000000.dmp";

    DBG_LOG("~~~~~~~~~~~~ UNHANDLED EXCEPTION OCCURRED ~~~~~~~~~~~");

    // Make sure dump file exists.
	if (!platform->Is_Directory(dump_dir.c_str()))
    {
        DBG_LOG("Attempt to create dump folder ...");
        DBG_LOG("Path: %s", dump_dir.c_str());

		if (!platform->Create_Directory(dump_dir.c_str(), true))
        {
            DBG_LOG("Failed to create dump folder. Aborting.");
            exit(0);
        }
    }

    // Find somewhere to dump the file.
    unsigned int index = 0;
    do
    {
		dump_path = dump_dir + "\\" + StringHelper::To_String(index) + ".dmp";
        index++;

	} while (platform->Is_File(dump_path.c_str()));

    // Dump the file!
    DBG_LOG("Attempt to create dump file ...");
    DBG_LOG("Path: %s", dump_path.c_str());

#if defined(OPT_STEAM_PLATFORM) && defined(MASTER_BUILD)
	SteamAPI_SetMiniDumpComment("Game Crashed :(");
	SteamAPI_WriteMiniDump(exceptionInfo->ExceptionRecord->ExceptionCode, exceptionInfo, 0);
#else

    HANDLE fileHandle = CreateFileA(dump_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == NULL)
    {
        DBG_LOG("Failed to create open dump file. Aborting.");
        exit(0);
    }

    // Duuuuuuuuump.
    HANDLE process = GetCurrentProcess();

    MINIDUMP_EXCEPTION_INFORMATION exceptionParam;
    exceptionParam.ThreadId          = GetCurrentThreadId();
    exceptionParam.ExceptionPointers = exceptionInfo;
    exceptionParam.ClientPointers    = FALSE;
	
	EnterCriticalSection(&g_dbghelp_critical_section);

    BOOL result = MiniDumpWriteDump(process,
                                    GetCurrentProcessId(),
                                    fileHandle,
#ifndef MASTER_BUILD
                                    MiniDumpWithFullMemory,
#else
                                    MiniDumpNormal,
#endif
                                    exceptionInfo == NULL ? NULL : &exceptionParam,
                                    NULL,
                                    NULL);

	LeaveCriticalSection(&g_dbghelp_critical_section);

    // Close the dump file handle.
	CloseHandle(fileHandle);

	// Check a file was created.
	if (result == FALSE)
	{
		u32 ec = GetLastError();
		DBG_LOG("Failed to create dump file, GetLastError()=%i (%s)", ec, FormatSystemError(ec).c_str());
	}
	else
	{
		DBG_LOG("Success!");
	}

#endif

	// Dump stack trace.
	DBG_LOG("");
	DBG_LOG("Call Stack:");

	StackFrame frames[256];
	int frameCount = platform->Get_Stack_Trace(frames, 256, exceptionInfo);
	for (int i = 0; i < frameCount; i++)
	{
		StackFrame& frame = frames[i];		
		
		DecodedStackFrame output;
		platform->Resolve_Stack_Frame_Info(frame, output);

		DBG_LOG("[%i] %s (%i): %s", i, output.File, output.Line, output.Name); 
	}

	DBG_LOG("");

	// We only try to recover from the first unhandled exception.
	static bool g_recovered_from_unhandled_exception = false;
	if (g_recovered_from_unhandled_exception == true)
	{
		DBG_LOG("As this is a secondary unhandled exception, game will now abort.")
		exit(0);
	    return EXCEPTION_EXECUTE_HANDLER;
	}
	g_recovered_from_unhandled_exception = true;

	// Bail the fuck out.
    // We use this to long jump back to the point after PlatformMain is called
    // so we can deinitialize everything.
	if (GetCurrentThreadId() == g_main_thread_id)
	{
	    DBG_LOG("Attempting recovery through longjmp ...");
		longjmp(g_error_recovery_longjmp, 1);
	}

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL WINAPI ConsoleCloseCallback(_In_ DWORD dwCtrlType)
{
	DBG_LOG("Recieved console close callback. Terminating game.");
	if (GameEngine::Try_Get())
	{
		GameEngine::Get()->Stop();
	}
	return TRUE;
}

int RealMain();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	if (strstr(pScmdline, "-log") != NULL)
	{
		Platform_Enable_Log_File();
	}

	return RealMain();
}

int main(int argc, char* argv[])
{	
	for (int i = 0; i < argc; i++)
	{
		if (stricmp(argv[i], "-log") == 0)
		{
			Platform_Enable_Log_File();
		}
	}
	
	std::string command_line = "";
	for (int i = 0; i < argc; i++)
	{
		command_line += StringHelper::Format("\"%s\" ", argv[i]);
	}

	return RealMain();
}

int RealMain()
{	
	PROFILE_THREAD("Main Thread");

	// Before anything else we initialize memory allocations!
	if (!MemoryManager::Init())
	{
		DBG_LOG("Failed to initialize platform memory.");
		return 1;
	}
		
	// Initialize platform.
	DBG_LOG("Initialising platform singleton.");
	{
		Platform::Create();
	}

	// Set working directory to executable directory. (Except if debugging in which case we will use our own directory).
#ifndef DEBUG_BUILD
//	if (!IsDebuggerPresent())
//		Platform::Get()->Set_Working_Dir(Platform::Get()->Extract_Directory(argv[0]).c_str());
#endif

	// Print out debugging information.
	DBG_LOG("--------------------------------------------------------------------------------------");
#ifdef DEBUG_BUILD
	DBG_LOG(" XGE Debug              Version %s, Built %s/%s/%s %s:%s", EngineAutoVersion::FULLVERSION_STRING, EngineAutoVersion::DAY, EngineAutoVersion::MONTH, EngineAutoVersion::YEAR, EngineAutoVersion::HOUR, EngineAutoVersion::MINUTE);
#else
	DBG_LOG(" XGE Release            Version %s, Built %s/%s/%s %s:%s", EngineAutoVersion::FULLVERSION_STRING, EngineAutoVersion::DAY, EngineAutoVersion::MONTH, EngineAutoVersion::YEAR, EngineAutoVersion::HOUR, EngineAutoVersion::MINUTE);
#endif
	Print_Game_Version();
	DBG_LOG("--------------------------------------------------------------------------------------");
	DBG_LOG(" Copyright (C) 2013-2014 TwinDrills");
	DBG_LOG("--------------------------------------------------------------------------------------");

	// Disable any pita popups that occassionally occur when checking drive file
	// sizes, errors in libraries, etc.	
	DBG_LOG("Setting error mode mask.");
	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOALIGNMENTFAULTEXCEPT|SEM_NOOPENFILEERRORBOX|SEM_NOGPFAULTERRORBOX);
	
	// Initialize the globals we need for exception handling.
	InitializeCriticalSection(&g_dbghelp_critical_section);
	g_main_thread_id		= GetCurrentThreadId();
	g_main_thread_handle	= GetCurrentThread();

	Win32_Thread::Setup_Main();
	
	// Symbols need to be initialized if we want debug stack traces and such
	// when the allocators are initialized.
	DBG_LOG("Installing debug symbols.");
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
	if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
	{    
		DBG_LOG("Debug symbols could not be found.");
		// :(. Well we can survive without symbols.		
	}

	// Install exception filter.	
	// On non-steam builds we want to install an exception handler, steam uploads crash dumps for us though, so no need to do this.
//#ifndef OPT_STEAM_PLATFORM
	DBG_LOG("Installing unhandled exception filter.");
	SetUnhandledExceptionFilter(ExceptionHandler);
	SetConsoleCtrlHandler(ConsoleCloseCallback, true);
//#endif

	// Instantiate factories we will be using. (I wish static instantiation for auto-registration worked
	//											over static library boundries ;_;)
	DBG_LOG("Instantiating factories.");
	Win32_FileStreamFactory fileStreamFactory;
	PNGPixelmapFactory		pngFactory;
	FreeType_FontFactory	freeTypeFactory;

	// Set main thread affinity to the first core only.
#ifndef DEBUG_BUILD	// If we run multiple game instances locally this causes process contention.
	//DBG_LOG("Setting main thread affinity to core 0.");
	//Thread::Get_Current()->Set_Affinity(1 << 0);
	//Thread::Get_Current()->Set_Priority(ThreadPriority::High);
#endif

	// Seed random.
	int seed = (int)GetTickCount();
	DBG_LOG("Seeding random to 0x%08x", seed);
	srand(seed);
	Random::Seed_Static(seed);

	// Engine variables that we need to setup.
	//GameRunner* runner;
	//GameEngine* engine;
	
	// Initialize Winsock
	WSADATA winsockData;
	int ret = WSAStartup(MAKEWORD(2,2), &winsockData);
	DBG_ASSERT_STR(ret == 0, "Failed to initialize winsock library.");

	// Concatinate the command line into a beast.
	std::string command_line = GetCommandLineA();

	// Set the long jump varaible. We use this to bail out and continue
	// cleaning up if the unhandled exception filter is invoked.	
	DBG_LOG("Creating setjmp handle for error recovery.");
	int exitcode = setjmp(g_error_recovery_longjmp);
	if (exitcode == 0)
	{
		Game_Entry_Point(command_line.c_str());

	/*	DBG_LOG("Creating new game instance.");
		runner = New_Game();

		if (runner == NULL)
		{
			DBG_LOG("No game instance provided, assuming raw entry point.");
			Game_Entry_Point(command_line.c_str());
		}
		else
		{
			DBG_LOG("Creating new engine instance.");
			engine = new GameEngine(runner, command_line.c_str());

			DBG_LOG("Running game engine.");
			engine->Run();
		}
	*/
	}
	else
	{
		DBG_LOG("Recovered from unhandled exception. Beginning cleanup.");
	}

	// Clean everything up.
	//DBG_LOG("Disposing of engine.");
	//SAFE_DELETE(engine);

	//DBG_LOG("Disposing of game.");
	//SAFE_DELETE(runner);

	// Bye bye winsock.
	WSACleanup();

	// All done!
	return 0;
}

#endif