// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// This is a special file, it is included by default in every other file
// so be careful what you put in here! It's mainly just for definitions
// of things that should be globally available (very little!).

#ifndef NULL
	#define NULL 0
#endif

// We use this for passing template names to maros, which don't work normally ;_;, eg:
//		MACRO(class<T,Y>)			// Throws error about to many parameters
//		MACRO(class<T COMMA Y>)		// Works!
#ifndef COMMA
	#define COMMA ,
#endif

#ifdef _WIN32
	#define PLATFORM_WIN32
#elif defined(__linux__)
	#define PLATFORM_LINUX
#elif defined(__APPLE__)
	#include "TargetConditionals.h"
	#if defined(TARGET_OS_MAC)
		#define PLATFORM_MACOS
	#elif defined(TARGET_OS_IPHONE) || defined(TARGET_IPHONE_SIMULATOR)
		#error "Unsupported platform."
	#else
		#error "Unsupported platform."
	#endif
#else
	#error "Unsupported platform."
#endif

#ifdef NDEBUG
#define RELEASE_BUILD
#else
#define DEBUG_BUILD
#endif

#ifdef _MASTER
#define MASTER_BUILD
#endif

#ifdef _DEDICATED_SERVER
#define DEDICATED_SERVER_BUILD
#endif

// Architecture types.
#if defined(__i386__) || defined(_M_IX86) || defined(_X86_)
	#define PLATFORM_32BIT
    #define PLATFORM_X86     
#elif defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
	#define PLATFORM_64BIT
    #define PLATFORM_X64     
    #define PLATFORM_AMD64   
#elif defined(__amd64__) || defined(_M_IA64)
	#define PLATFORM_64BIT
    #define PLATFORM_ITANIUM64   
#elif defined(_M_PPC)
	#define PLATFORM_32BIT
	#define PLATFORM_PPC		
#else
	#error "Unsupported platform."
#endif

// Endianness defines
#if defined(_M_PPCBE) || defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN)
    #define PLATFORM_BIG_ENDIAN  1
#elif defined(_M_PPC) || defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN)
    #define PLATFORM_LITTLE_ENDIAN   1
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == _LITTLE_ENDIAN
    #define PLATFORM_LITTLE_ENDIAN   1
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == _BIG_ENDIAN
    #define PLATFORM_BIG_ENDIAN      1
#elif defined(__sparc) || defined(__sparc__) \
    || defined(_POWER) || defined(__powerpc__) \
    || defined(__ppc__) || defined(__hpux) \
    || defined(_MIPSEB) || defined(_POWER) \
    || defined(__s390__)
    #define PLATFORM_BIG_ENDIAN      1
#elif defined(__i386__) || defined(__alpha__) \
    || defined(__ia64) || defined(__ia64__) \
    || defined(_M_IX86) || defined(_M_IA64) \
    || defined(_M_ALPHA) || defined(__amd64) \
    || defined(__amd64__) || defined(_M_AMD64) \
    || defined(__x86_64) || defined(__x86_64__) \
    || defined(_M_X64) || defined(__bfin__)
    #define PLATFORM_LITTLE_ENDIAN   1
#else
	#error "Unsupported platform."
#endif

// Language standard?
#if (__cplusplus == 201103L) 
	#define LANGUAGE_STANDARD_CPP11
	
// __cplusplus values are broken in GCC versions prior to 4.7.0 (aka, all mac builds ;_;)
#else
//#elif (__cplusplus == 199711L) 
	#define LANGUAGE_STANDARD_CPP97
//#else
//	#error "Unknown language standard."

#endif

// Libraries always included for each platform.
//#ifdef PLATFORM_WIN32
//#include <stdio.h>
//#include <algorithm>
//#include <stddef.h>
//#endif

// Switch some string compares for standardised platforms.
#ifndef PLATFORM_WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

// If set we override memory allocations and push everything through custom allocator
// classes. Useful for keeping track of where memory is going.
// TODO:
// Be warned: Currently broken on linux, think its an alignment issue, fix and re-enable plz.
//#define OPT_OVERRIDE_DEFAULT_ALLOCATORS

// Debugging macros!
extern void Platform_Enable_Log_File();
extern void Platform_Print(const char* format, ...);
extern void Platform_Fatal(bool graceful, const char* format, ...);
extern void Platform_Exit();

int platform_get_used_memory();
int	platform_get_run_time();
extern int game_get_frame_index();
extern int game_get_gc_size();

#if OPT_OVERRIDE_DEFAULT_ALLOCATORS
#define DBG_LOG(format, ...) \
	{ \
		const int s__run__time = platform_get_run_time(); \
		const int s__run__time__seconds = s__run__time / 1000; \
		const int s__hours = (s__run__time__seconds / 60) / 60; \
		const int s__minutes = (s__run__time__seconds / 60) % 60; \
		const int s__seconds = s__run__time__seconds % 60; \
		Platform_Print("[%02d:%02d:%02d:%03d, %i] [%0.2f mb] " format, s__hours, s__minutes, s__seconds, s__run__time % 1000, game_get_frame_index(), (platform_get_used_memory() / 1024.0f) / 1024.0f, ##__VA_ARGS__); \
	}
#else
#define DBG_LOG(format, ...) \
{ \
	const int s__run__time = platform_get_run_time(); \
	const int s__run__time__seconds = s__run__time / 1000; \
	const int s__hours = (s__run__time__seconds / 60) / 60; \
	const int s__minutes = (s__run__time__seconds / 60) % 60; \
	const int s__seconds = s__run__time__seconds % 60; \
	Platform_Print("[%02d:%02d:%02d:%03d, %i, %.1fmb] " format, s__hours, s__minutes, s__seconds, s__run__time % 1000, game_get_frame_index(), game_get_gc_size() / 1024.0f / 1024.0f, ##__VA_ARGS__); \
}
#endif

#define DBG_ASSERT_STR(cond, format, ...) \
	if (!(cond)) \
	{ \
		DBG_LOG("====== ASSERT FAILED ======"); \
		DBG_LOG("%s:%i", __FILE__, __LINE__); \
		DBG_LOG("%s", #cond); \
		DBG_LOG("Message: " format "\n", ##__VA_ARGS__); \
		Platform_Fatal(false, "Assert Failed\n%s:%i\n%s\n\n" format " \n\nPlease report this issue to the developers and help us fix this :).",  __FILE__, __LINE__, #cond, ##__VA_ARGS__); \
	}

#define DBG_ASSERT_STR_STRAIGHT(cond, format, ...) \
	if (!(cond)) \
	{ \
		DBG_LOG("====== ASSERT FAILED ======"); \
		DBG_LOG("%s:%i", __FILE__, __LINE__); \
		DBG_LOG("%s", #cond); \
		DBG_LOG("Message: " format "\n", ##__VA_ARGS__); \
		Platform_Fatal(false, format, ##__VA_ARGS__); \
	}

#define DBG_ASSERT(cond) \
	if (!(cond)) \
	{ \
		DBG_LOG("====== ASSERT FAILED ======"); \
		DBG_LOG("%s:%i", __FILE__, __LINE__); \
		DBG_LOG("%s", #cond); \
		Platform_Fatal(false, "Assert Failed\n%s:%i\n%s\n\nPlease report this issue to the developers and help us fix this :).",  __FILE__, __LINE__, #cond); \
	}

// Check is the same as assert except it exits gracefully.
#define DBG_CHECK_STR(cond, format, ...) \
	if (!(cond)) \
	{ \
		DBG_LOG("====== CHECK FAILED ======"); \
		DBG_LOG("%s:%i", __FILE__, __LINE__); \
		DBG_LOG("%s", #cond); \
		DBG_LOG("Message: " format "\n", ##__VA_ARGS__); \
		Platform_Fatal(true, "Check Failed\n%s:%i\n%s\n\n" format,  __FILE__, __LINE__, #cond, ##__VA_ARGS__); \
	}

#define DBG_CHECK(cond) \
	if (!(cond)) \
	{ \
		DBG_LOG("====== CHECK FAILED ======"); \
		DBG_LOG("%s:%i", __FILE__, __LINE__); \
		DBG_LOG("%s", #cond); \
		Platform_Fatal(true, "Check Failed\n%s:%i\n%s",  __FILE__, __LINE__, #cond); \
	}

#ifndef LANGUAGE_STANDARD_CPP11
#define DBG_STATIC_ASSERT(cond) \
	char static_assert_[cond ? 1 : -1];
#define DBG_STATIC_ASSERT_STR(cond, msg) \
	char static_assert_[cond ? 1 : -1];
#else
#define DBG_STATIC_ASSERT(cond) \
	static_assert(cond, "Static assert failed.");
#define DBG_STATIC_ASSERT_STR(cond, msg) \
	static_assert(cond, msg);
#endif

// Memory management whowhar.
#define SAFE_DELETE(x) \
		if ((x)) \
		{ \
			delete x; \
			x = NULL; \
		} 
#define SAFE_DELETE_ARRAY(x) \
		if ((x)) \
		{ \
			delete[] x; \
			x = NULL; \
		} 

// Some general handy stuff.
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(*arr))

// Dammnit windows, this is cross platform code, I don't care about your
// secure functions.
#if defined(_MSC_VER)
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS (1)
	#endif
	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE (1)
	#endif
	#pragma warning(disable : 4996)
	#pragma warning(disable : 4995)
	#pragma warning(disable : 4722)
//#pragma warning(disable : 4150) // Deletion of poiter to incomplete class - breaks use of foreward-declared shared-ptrs D:
#endif

// Special compiler keywords.
#if defined(_MSC_VER)	
	#define INLINE __forceinline
	#define NO_EXCEPT(bool_val) // Fucking visual studio dosen't have a version of this D:
#elif defined(__GNUC__)
	#define INLINE __attribute__((always_inline))

	#ifdef LANGUAGE_STANDARD_CPP11
		#define NO_EXCEPT(bool_val) noexcept(bool_val)
	#else
		#define NO_EXCEPT(bool_val) 	
	#endif
#else
	#error "Unsupported platform."
#endif

// Special types.
#define s8  char
#define u8  unsigned char
#define s16 short
#define u16 unsigned short
#define s32 int
#define u32 unsigned int
#define s64 long long int
#define u64 unsigned long long int
#define f32 float
#define f64 double

// Project-specific, this really needs to go elsewhere.
#define PROJECT_TITLE "Zombie Grinder"

// Some generic toggles.
#ifndef MASTER_BUILD
#define ENABLE_PROFILING
//#define ENABLE_STAT_COLLECTION
#ifdef PLATFORM_WIN3
//#define USE_VS_CONCURRENCY_MARKERS
#endif
#endif

#ifndef MASTER_BUILD
//#define USE_BROFILER_MARKERS 1
#endif

#define OPT_STEAM_PLATFORM			// If defined we use the steam platform for online communication.
#define OPT_DISABLE_HOT_RELOADING	// TODO: Hot-reloading is pretty fucked atm, so lets just disable it till we have time to sort it out.

#define STEAM_APP_ID 263920

//#if 0
#ifndef MASTER_BUILD
#define OPT_STEAM_BSD_SOCKETS
#endif

// Allows duplicate connections by ignoring duplicate ticket errors.
#ifndef MASTER_BUILD
#ifdef OPT_STEAM_BSD_SOCKETS
#define OPT_STEAM_ALLOW_DUPLICATE_CONNECTIONS
#define OPT_DISABLE_NET_TIMEOUTS
#define OPT_DISABLE_TICKETING
#endif
#endif
//#endif

#define OPT_OUTPUT_TO_LOG_FILE

#ifdef DEBUG_BUILD
#define OPT_SUPRESS_STEAM_RESTART	// Supresses steam reboot if SteamAPI_RestartAppIfNecessary returns true.
#endif

// Enable "premium" account features. 
// Update: We're not doing this anymore. Straight up payments now.
//#define OPT_PREMIUM_ACCOUNTS 1

// Forces all IsDlcPurchased type calls to return false.
//#define OPT_ASSUME_NO_DLC 1

// Enable "micro-transaction" stuff.
//#define OPT_MICROTRANSACTION_ITEMS 1

// Force a single camera rendering view even in split-screen.
//#define OPT_FORCE_CAMERA_COUNT 1

// Disables various HUD rendering things.
//#define OPT_DISABLE_HUD_RENDERING 1

// Disables cutscenes.
#ifdef MASTER_BUILD
#define OPT_DISABLE_CUTSCENES 1
#endif

//#define VM_ALLOW_MULTITHREADING		// Allows VM multithreading - EXPERIMENTAL, may fuck up royally.

// Include memory function overrides.
#include "Generic/Memory/Memory.h"
