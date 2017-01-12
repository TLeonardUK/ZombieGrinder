// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_PLATFORM_WIN32_ENTRYPOINT_
#define _ENGINE_PLATFORM_WIN32_ENTRYPOINT_

#include <Windows.h>

extern jmp_buf			 g_error_recovery_longjmp;
extern CRITICAL_SECTION  g_dbghelp_critical_section;
extern u32			     g_main_thread_id;
extern HANDLE			 g_main_thread_handle;

int main(int argc, char* argv[]);

#endif

#endif