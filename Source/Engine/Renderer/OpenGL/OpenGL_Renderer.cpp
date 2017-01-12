// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Renderer/OpenGL/OpenGL_Renderer.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/Actor.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/Light.h"
#include "Engine/Renderer/Drawable.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/EngineOptions.h"

#include "Generic/Threads/Thread.h"

#include "Engine/Engine/FrameStats.h"

#include "Generic/Math/Math.h"
#include "Generic/Types/Matrix4.h"
#include "Generic/Types/Rect2D.h"

#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

#include "Engine/Renderer/OpenGL/OpenGL_Texture.h"
#include "Engine/Renderer/OpenGL/OpenGL_ShaderProgram.h"
#include "Engine/Renderer/OpenGL/OpenGL_RenderTarget.h"
#include "Engine/Renderer/OpenGL/OpenGL_Geometry.h"
#include "Engine/Renderer/OpenGL/OpenGL_RenderQuery.h"

#include "Engine/Options/OptionRegistry.h"

#ifdef PLATFORM_WIN32
#include "Engine/Display/Win32/Win32_GfxDisplay.h"
#endif

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/Stream.h"

#include <vector>
#include <cstdio>

// Arrrrrgh XGL has a bunch of name conflicts with our code ;_; Fuckers.
#if defined(PLATFORM_LINUX)

namespace XGL
{
	#include <GL/glx.h>
	#undef None
};
using namespace XGL;

// MacOS needs some special shit to get entry points.
#elif defined(PLATFORM_MACOS)

#import <mach-o/dyld.h>

// As per: 
// https://developer.apple.com/library/mac/documentation/graphicsimaging/conceptual/opengl-macprogguide/opengl_entrypts/opengl_entrypts.html
void* glGetProcAddress(const char *name)
{
    NSSymbol symbol;

	char* symbolName = (char*)malloc(strlen(name) + 2); 
	strcpy(symbolName + 1, name); 
    symbolName[0] = '_'; 
    symbol = NULL;

    if (NSIsSymbolNameDefined(symbolName)) 
	{
        symbol = NSLookupAndBindSymbol(symbolName);
	}

    free(symbolName); 

    return symbol ? NSAddressOfSymbol(symbol) : NULL; 
}

#endif

// Does some debug checking when loading gl extensions.
#if defined(PLATFORM_WIN32)
	#define LOAD_GL_EXTENSION(func, type, name) \
		{ \
			func = (type)wglGetProcAddress(#name); \
			DBG_ASSERT_STR(func != NULL, "OpenGL implementation does not contain entry point for function '%s'.", #name); \
		}
	#define TRY_LOAD_GL_EXTENSION(func, type, name) \
		{ \
			func = (type)wglGetProcAddress(#name); \
		}

#elif defined(PLATFORM_LINUX)
	#define LOAD_GL_EXTENSION(func, type, name) \
		{ \
			func = (type)glXGetProcAddress(reinterpret_cast<const GLubyte*>(#name)); \
			DBG_ASSERT_STR(func != NULL, "OpenGL implementation does not contain entry point for function '%s'.", #name); \
		}
	#define TRY_LOAD_GL_EXTENSION(func, type, name) \
		{ \
			func = (type)glXGetProcAddress(reinterpret_cast<const GLubyte*>(#name)); \
		}

#elif defined(PLATFORM_MACOS)
	#define LOAD_GL_EXTENSION(func, type, name) \
		{ \
			func = (type)glGetProcAddress(#name); \
			DBG_ASSERT_STR(func != NULL, "OpenGL implementation does not contain entry point for function '%s'.", #name); \
		}
	#define TRY_LOAD_GL_EXTENSION(func, type, name) \
		{ \
			func = (type)glGetProcAddress(#name); \
		}

#else
	#error Unsupported platform.

#endif

#define GL_REQUIRED_EXTENSION(id)
#define GL_EXTENSION_FALLBACK(return_type, name, parameters, entry_point, required)	
#define GL_EXTENSION(return_type, name, parameters, entry_point, required) funcptr_##name name = NULL;
#include "Engine/Renderer/OpenGL/OpenGL_Extensions.inc"
#undef GL_EXTENSION
#undef GL_EXTENSION_FALLBACK
#undef GL_REQUIRED_EXTENSION

OpenGL_Renderer::OpenGL_Renderer()
	: m_initialized(false)
	, m_display(NULL)
	, m_binded_shader_program(NULL)
	, m_binded_render_target(NULL)
	, m_binded_material(NULL)
	, m_viewport(0, 0, 0, 0)
	, m_render_thread(NULL)
	, m_global_primitive_color(1.0f, 1.0f, 1.0f, 1.0f)
	, m_scissor_test(false)
	, m_depth_function(RendererOption::E_None)
	, m_depth_test(false)
	, m_alpha_test(false)
	, m_blend_function(RendererOption::E_None)
	, m_blend(false)
	, m_clear_depth(false)
	, m_line_width(1.0f)
	, m_pending_vsync_set(false)
	, m_cull_face(RendererOption::E_Back)
	, m_in_batch(false)
	, m_vsync_enabled(true)
	, m_last_known_display_mode(GfxDisplayMode::Windowed)
	, m_logging_enabled(false)
#if defined(PLATFORM_WIN32) && defined(USE_DWM_FLUSH)
	, m_dwm_lib(NULL)
	, DwmFlush(NULL)
#endif
{
	memset(m_binded_textures, 0, sizeof(OpenGL_Texture*) * MAX_BINDED_TEXTURES);
	memset(m_client_states, 0, sizeof(m_client_states));

	m_warmup_textures_mutex = Mutex::Create();
	m_defer_mutex = Mutex::Create();

	for (int i = 0; i < RendererFeature::COUNT; i++)
	{
		bSupportsFeatureIsCached[i] = false;
	}
}

OpenGL_Renderer::~OpenGL_Renderer()
{
	// TODO: Destroy all shaders etc.

	SAFE_DELETE(m_warmup_textures_mutex);

	// Unload compositor.
#if defined(PLATFORM_WIN32) && defined(USE_DWM_FLUSH)
	if (m_dwm_lib != NULL)
	{
		FreeLibrary(m_dwm_lib);
		m_dwm_lib = NULL;
	}
#endif
}

void APIENTRY OpenGlDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// Ignore certain spammy messages.
	if (id == 0x00020084) return; // Texture state usage warning: Texture 0 is base level inconsistent. Check texture size - Caused by glClear.

	char severity_str[64];
	char type_str[64];
	char source_str[64];

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			strcpy(severity_str, "Notification");
			break;
		}
		case GL_DEBUG_SEVERITY_LOW:
		{
			strcpy(severity_str, "Low");
			break;
		}
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			strcpy(severity_str, "Medium");
			break;
		}
		case GL_DEBUG_SEVERITY_HIGH:
		{
			strcpy(severity_str, "High");
			break;
		}
		default:
		{
			sprintf(severity_str, "Unknown[%i]", severity);
			break;
		}
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
		{
			strcpy(type_str, "Error");
			break;
		}
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		{
			strcpy(type_str, "Deprecated");
			break;
		}
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		{
			strcpy(type_str, "Undefined");
			break;
		}
		case GL_DEBUG_TYPE_PORTABILITY:
		{
			strcpy(type_str, "Portability");
			break;
		}
		case GL_DEBUG_TYPE_PERFORMANCE:
		{
			strcpy(type_str, "Peformance");
			break;
		}
		case GL_DEBUG_TYPE_OTHER:
		{
			strcpy(type_str, "Other");
			break;
		}
		default:
		{
			sprintf(type_str, "Unknown[%i]", type);
			break;
		}
	}

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
		{
			strcpy(source_str, "API");
			break;
		}
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		{
			strcpy(source_str, "Window-System");
			break;
		}
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		{
			strcpy(source_str, "Shader-Compiler");
			break;
		}
		case GL_DEBUG_SOURCE_THIRD_PARTY:
		{
			strcpy(source_str, "Third-Party");
			break;
		}
		case GL_DEBUG_SOURCE_APPLICATION:
		{
			strcpy(source_str, "Application");
			break;
		}
		case GL_DEBUG_SOURCE_OTHER:
		{
			strcpy(source_str, "Other");
			break;
		}
		default:
		{
			sprintf(source_str, "Unknown[%i]", type);
			break;
		}
	}

	DBG_LOG("[OpenGL Debug] [%s][%s][%s][%i] %s", severity_str, type_str, source_str, id, message);
}

void OpenGL_Renderer::Initialize_OpenGL()
{
	// Write out some information first.
	const char* vendor			= (const char*)glGetString(GL_VENDOR);
	const char* renderer		= (const char*)glGetString(GL_RENDERER);
	const char* version			= (const char*)glGetString(GL_VERSION);
	const char* shader_version	= (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    const char* extensions		= (const char*)glGetString(GL_EXTENSIONS);

	DBG_LOG("GL_VENDOR                   : %s", vendor == NULL ? "NULL" : vendor);
	DBG_LOG("GL_RENDERER                 : %s", renderer == NULL ? "NULL" : renderer);
	DBG_LOG("GL_VERSION                  : %s", version == NULL ? "NULL" : version);
	DBG_LOG("GL_SHADING_LANGUAGE_VERSION : %s", shader_version == NULL ? "NULL" : shader_version);
	DBG_LOG("GL_EXTENSIONS               : ");

	const char* extension_start = extensions;
	const char* extension_end = extensions;
	while (extensions != NULL && *extension_end != '\0')
    {
		if (*extension_end == ' ')
		{
			char extension[1024];

			int len = Min(extension_end - extension_start, 1023);
			memcpy(extension, extension_start, len);
			extension[len] = '\0';

			extension_start = extension_end + 1;

			DBG_LOG("\t%s", extension);
		}
        extension_end++;
    }
	
#define GL_REQUIRED_EXTENSION(id) DBG_ASSERT_STR_STRAIGHT(Is_Extension_Supported(#id), "Your GPU drivers do not appear to support the '%s' opengl extension.\n\nPlease make sure you meet minimum specs and you have the latest drivers from your GPU vendor installed.", #id);
#define GL_EXTENSION(return_type, name, parameters, entry_point, required) \
	{ \
		if (required) \
		{ \
			LOAD_GL_EXTENSION(name, funcptr_##name, entry_point); \
			if (name != NULL) \
				DBG_LOG("Loaded opengl extension '%s'.", #entry_point); \
		} \
		else \
		{ \
			TRY_LOAD_GL_EXTENSION(name, funcptr_##name, entry_point); \
			if (name != NULL) \
			{ \
				DBG_LOG("Loaded opengl extension '%s'.", #entry_point); \
			} \
			else \
			{ \
				DBG_LOG("Couldn't load optional opengl extension '%s'.", #entry_point); \
			} \
		} \
	}
#define GL_EXTENSION_FALLBACK(return_type, name, parameters, entry_point, required)	\
	if (name != NULL) \
	{ \
		GL_EXTENSION(return_type, name, parameters, entry_point, required) \
	}

#include "Engine/Renderer/OpenGL/OpenGL_Extensions.inc"
#undef GL_EXTENSION
#undef GL_EXTENSION_FALLBACK
#undef GL_REQUIRED_EXTENSION

	// Turn on legacy mode if we do not support shaders/fbo/etc.
	DBG_LOG("Supports Shaders? - %s", Supports(RendererFeature::Shaders) ? "Yes" : "No");
	DBG_LOG("Supports FBO's? - %s",   Supports(RendererFeature::FBO)     ? "Yes" : "No");
	DBG_LOG("Supports Queries? - %s", Supports(RendererFeature::Querys)  ? "Yes" : "No");

	// Enable logging?
	if (*EngineOptions::render_logging)
	{
		Set_Logging_Enabled(true);

		if (glDebugMessageCallback && glDebugMessageControl)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
			glDebugMessageCallback(OpenGlDebugCallback, this);
		}
	}

//	DBG_ASSERT_STR(Supports(RendererFeature::Shaders), "Your grapShics drivers do not appear to support programmable shaders.\n\nThis game cannot continue, as we no longer support graphics drivers old enough to require the fixed-function pipeline.");
	bool original_state = *EngineOptions::render_legacy;
	*EngineOptions::render_legacy = false;

	if (!Supports(RendererFeature::FBO) ||
		!Supports(RendererFeature::Shaders))
	{
		DBG_LOG("Graphics drivers do not support FBO's, falling back to legacy rendering.");
		*EngineOptions::render_legacy = true;
		*EngineOptions::render_legacy_was_forced = true;
	}

	*EngineOptions::render_legacy = *EngineOptions::render_legacy || original_state;

	// Store the rendering thread.
	m_render_thread = Thread::Get_Current();

	// Turn off vsync.
	Set_VSync(false);

	// Load some compositor dll functions on windows.
#if defined(PLATFORM_WIN32) && defined(USE_DWM_FLUSH)
	m_dwm_lib = LoadLibrary("Dwmapi.dll");
	if (m_dwm_lib != NULL)
	{
		DwmFlush = (PDwmFlush)GetProcAddress(m_dwm_lib, "DwmFlush");
	}
#endif

	// Clear screen.							
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	
	CHECK_GL_ERROR();

	glDisable(GL_CULL_FACE);
	CHECK_GL_ERROR();

	glEnable(GL_TEXTURE_2D);	
	CHECK_GL_ERROR();

	glDisable(GL_LIGHTING);	
	CHECK_GL_ERROR();

	glEnable(GL_ALPHA_TEST);
	CHECK_GL_ERROR();

	glAlphaFunc(GL_GREATER, (1.0f / 255.0f) * 2); // Hack: Fix the images, some of them have a bunch of 1alpha pixels >_>
	CHECK_GL_ERROR();
	//glDepthMask(true);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	m_depth_function = RendererOption::E_Always;
	m_depth_test = true;
	m_depth_write = false;

	Set_Clear_Color(Color::Black);
	Set_Clear_Depth(1.0f);
	Set_Depth_Test(true);
	Set_Depth_Write(true);
	Set_Depth_Function(RendererOption::E_Greater_Or_Equal);
	Set_Cull_Face(RendererOption::E_Back);
	Set_Scissor_Test(false);
	Set_Line_Size(1.0f);
	
	// We almost always use these, so might as well enable them off the bat.
	Set_Client_State_Enabled(GL_VERTEX_ARRAY, true);
	//Set_Client_State_Enabled(GL_NORMAL_ARRAY, true);
	Set_Client_State_Enabled(GL_COLOR_ARRAY, true);
	Set_Client_State_Enabled(GL_TEXTURE_COORD_ARRAY, true);
}

bool OpenGL_Renderer::Is_Logging_Enabled()
{
	return m_logging_enabled;
}

void OpenGL_Renderer::Set_Logging_Enabled(bool bEnabled)
{
	m_logging_enabled = bEnabled;
}

bool OpenGL_Renderer::Supports(RendererFeature::Type feature)
{
	if (bSupportsFeatureIsCached[(int)feature])
	{
		return bSupportsFeatureCache[(int)feature];
	}

	std::vector<void*> required_functions;

	switch (feature)
	{
	case RendererFeature::FBO:
		{
			// FBO's not supported in legacy mode.
			//if (*EngineOptions::render_legacy)
			//{
			//	return false;
			//}

			required_functions.push_back((void*)glDrawBuffers);
			required_functions.push_back((void*)glGenFramebuffers);
			required_functions.push_back((void*)glDeleteFramebuffers);
			required_functions.push_back((void*)glBindFramebuffer);
			required_functions.push_back((void*)glFramebufferTexture2D);
			required_functions.push_back((void*)glCheckFramebufferStatus);
			break;
		}
	case RendererFeature::Shaders:
		{
			// FBO's not supported in legacy mode.
			//if (*EngineOptions::render_legacy)
			//{
			//	return false;
			//}

			required_functions.push_back((void*)glCreateShader);
			required_functions.push_back((void*)glDeleteShader);
			required_functions.push_back((void*)glAttachShader);
			required_functions.push_back((void*)glShaderSource);
			required_functions.push_back((void*)glCompileShader);
			required_functions.push_back((void*)glCreateProgram);
			required_functions.push_back((void*)glLinkProgram);
			required_functions.push_back((void*)glDeleteProgram);
			required_functions.push_back((void*)glGetProgramiv);
			required_functions.push_back((void*)glGetShaderiv);
			required_functions.push_back((void*)glGetShaderInfoLog);
			required_functions.push_back((void*)glGetProgramInfoLog);
			required_functions.push_back((void*)glUseProgram);
			required_functions.push_back((void*)glActiveTexture);
			required_functions.push_back((void*)glGetUniformLocation);
			required_functions.push_back((void*)glUniform1i);
			required_functions.push_back((void*)glUniform1f);
			required_functions.push_back((void*)glUniform3f);
			required_functions.push_back((void*)glUniform4f);
			required_functions.push_back((void*)glUniformMatrix4fv);
			required_functions.push_back((void*)glDrawBuffers);
			break;
		}
	/*case RendererFeature::VBO:
		{
			required_functions.push_back(glGenBuffersARB);
			required_functions.push_back(glBindBufferARB);
			required_functions.push_back(glBufferDataARB);
			required_functions.push_back(glDeleteBuffersARB);
			//required_functions.push_back(glBufferSubDataARB); - Not required but more efficient.
			break;
		}
	*/
	case RendererFeature::Querys:
		{
#if !defined(PLATFORM_MACOS)
			required_functions.push_back((void*)glGenQueries);
			required_functions.push_back((void*)glDeleteQueries);
			required_functions.push_back((void*)glBeginQuery);
			required_functions.push_back((void*)glEndQuery);
			required_functions.push_back((void*)glGetQueryObjectiv);
			required_functions.push_back((void*)glGetQueryObjectuiv);
			required_functions.push_back((void*)glGetQueryObjecti64v);
			required_functions.push_back((void*)glQueryCounter);
#else
			return false;
#endif
			break;
		}
	default:
		{
			return false;
		}
	}

	bool bResult = true;

	for (std::vector<void*>::iterator iter = required_functions.begin(); iter != required_functions.end(); iter++)
	{
		void* func = *iter;
		if (func == NULL)
		{
			bResult = false;
			break;
		}
	}
	
	bSupportsFeatureIsCached[(int)feature] = true;
	bSupportsFeatureCache[(int)feature] = bResult;

	return bResult;
}

void OpenGL_Renderer::DeferBufferDelete(GLuint id)
{
	MutexLock lock(m_defer_mutex);
	m_defer_buffer_deletes.push_back(id);
}

void OpenGL_Renderer::Set_Client_State_Enabled(GLint state, bool enabled)
{
	// HACK: This may be platform-specific? Is GL_VERTEX_ARRAY guaranteed to have the same value cross-platform?
	int state_index = state - GL_VERTEX_ARRAY;
	DBG_ASSERT(state_index >= 0 && state_index < MAX_CLIENT_STATES);

	if (m_client_states[state_index] != enabled)
	{
		if (enabled == true)
		{
			glEnableClientState(state);
		}
		else
		{
			glDisableClientState(state);
		}
		CHECK_GL_ERROR();

		m_client_states[state_index] = enabled;
	}
}

bool OpenGL_Renderer::Is_Extension_Supported(const char* szTargetExtension)
{
	// Nicked from NeHe, because I'm lazy as fuck.

    const unsigned char *pszExtensions = NULL;
    const unsigned char *pszStart;
    unsigned char *pszWhere, *pszTerminator;
 
    // Extension names should not have spaces
    pszWhere = (unsigned char *)strchr(szTargetExtension, ' ');
	if (pszWhere || *szTargetExtension == '\0')
	{
        return false;
	}

    // Get Extensions String
    pszExtensions = glGetString(GL_EXTENSIONS);
	CHECK_GL_ERROR();

	if (pszExtensions == NULL)
	{
		return false;
	}

    // Search The Extensions String For An Exact Copy
    pszStart = pszExtensions;
    for (;;)
    {
        pszWhere = (unsigned char *)strstr((const char *)pszStart, szTargetExtension);
        if (!pszWhere)
		{
            break;
		}

		pszTerminator = pszWhere + strlen(szTargetExtension);
        if (pszWhere == pszStart || *( pszWhere - 1 ) == ' ')
		{
            if (*pszTerminator == ' ' || *pszTerminator == '\0')
			{
                return true;
			}
		}
        pszStart = pszTerminator;
    }

    return false;
}

void OpenGL_Renderer::Flip(const FrameTime& time)
{
	// Kill deferred delete buffers.
	{
		MutexLock lock(m_defer_mutex);

		for (std::vector<GLuint>::iterator iter = m_defer_buffer_deletes.begin(); iter != m_defer_buffer_deletes.end(); iter++)
		{
			GLuint val = *iter;
			glDeleteBuffers(1, &val);
			CHECK_GL_ERROR();
		}
		m_defer_buffer_deletes.clear();
	}

	if (m_pending_vsync_set == true)
	{
#if defined(PLATFORM_WIN32)
		if (wglSwapInterval != NULL)
		{
#if defined(USE_DWM_FLUSH)		
			if (GfxDisplay::Get()->Get_Mode() == GfxDisplayMode::Fullscreen)
			{
				wglSwapInterval(m_pending_vsync_value ? 1 : 0);
			}
			else
			{
				wglSwapInterval(0); // we use DwmFlush instead.
			}
#else
			wglSwapInterval(m_pending_vsync_value ? 1 : 0);
#endif
		}
#elif defined(PLATFORM_LINUX)
		if (glXSwapInterval != NULL)
		{
			glXSwapInterval(m_pending_vsync_value ? 1 : 0);
		}
#elif defined(PLATFORM_MACOS)
		static_cast<MacOS_GfxDisplay*>(m_display)->Set_VSync(m_pending_vsync_value);
#endif	

		m_vsync_enabled = (m_pending_vsync_value != 0);
		m_pending_vsync_set = false;
	}

// MacOS needs a bunch of states reset to prevent black-screen with overlay -_-
//#if defined(PLATFORM_MACOS)

	Set_Client_State_Enabled(GL_VERTEX_ARRAY, false);
	Set_Client_State_Enabled(GL_NORMAL_ARRAY, false);
	Set_Client_State_Enabled(GL_COLOR_ARRAY, false);
	Set_Client_State_Enabled(GL_TEXTURE_COORD_ARRAY, false);

//	glVertexPointer		(3, GL_FLOAT, 0, 0);
//	CHECK_GL_ERROR();
//	glTexCoordPointer	(2, GL_FLOAT, 0, 0);
//	CHECK_GL_ERROR();
//	glNormalPointer		(GL_FLOAT, 0,	 0);
//	CHECK_GL_ERROR();
//	glColorPointer		(4, GL_FLOAT, 0, 0);
//	CHECK_GL_ERROR();

	Bind_Vertex_Pointer(3, GL_FLOAT, 0, 0);
	Bind_TexCoord_Pointer(2, GL_FLOAT, 0, 0);
	Bind_Color_Pointer(4, GL_FLOAT, 0, 0);

	// If legacy-renderer, make sure everything is unbound.
	if (*EngineOptions::render_legacy)
	{
		if (glBindFramebuffer != NULL)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		if (glUseProgram != NULL)
		{
			glUseProgram(0);
		}
	}

//#endif

	// Swap front/back buffers.
	m_display->Swap_Buffers();

#if defined(PLATFORM_WIN32) && defined(USE_DWM_FLUSH)
	if (m_vsync_enabled)
	{
		if (GfxDisplay::Get()->Get_Mode() != GfxDisplayMode::Fullscreen)
		{
			if (DwmFlush != NULL)
			{
				DwmFlush();
			}
		}
	}

	// If display mode has changed we need to update vsync mode.
	GfxDisplayMode::Type display_mode = GfxDisplay::Get()->Get_Mode();
	if (display_mode != m_last_known_display_mode)
	{
		Set_VSync(m_vsync_enabled);
		m_last_known_display_mode = display_mode;
	}
#endif
}

bool OpenGL_Renderer::Set_Display(::GfxDisplay* display)
{
	m_display = display;

	if (m_initialized == false)
	{
		Initialize_OpenGL();
		m_initialized = true;
	}

	return true;
}

void OpenGL_Renderer::Set_VSync(bool vsync)
{
	m_pending_vsync_set = true;
	m_pending_vsync_value = vsync;
}

void OpenGL_Renderer::Bind_Shader_Program(ShaderProgram* shader)
{
	bool bShadersSupported = Supports(RendererFeature::Shaders);

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	OpenGL_ShaderProgram* glShader = dynamic_cast<OpenGL_ShaderProgram*>(shader);

	if (m_binded_shader_program != glShader)
	{
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Shader Binds");
#endif

		if (bShadersSupported)
		{
			if (glShader != NULL)
			{
				glUseProgram(glShader->Get_ID());
				CHECK_GL_ERROR();
			}
			else
			{
				glUseProgram(0);
				CHECK_GL_ERROR();
			}
		}

		m_binded_shader_program = glShader;
	}	
}

GLuint Compile_Shader(char* source, ShaderType::Type type)
{
	GLuint shader = -1;

	switch (type)
	{
	case ShaderType::Vertex:	
		{
			shader = glCreateShader(GL_VERTEX_SHADER);		
			CHECK_GL_ERROR();
			break;
		}
	case ShaderType::Fragment:	
		{
			shader = glCreateShader(GL_FRAGMENT_SHADER);	
			CHECK_GL_ERROR();
			break;
		}
	}

	if (shader == 0)
	{
		DBG_LOG("glCreateShader failed with result of %i.", shader);
		return NULL;
	}

#if defined(PLATFORM_WIN32)
	glShaderSource(shader, 1, &source, NULL);
#elif defined(PLATFORM_LINUX)
	const GLchar* glsource = reinterpret_cast<const GLchar*>(source);
	glShaderSource(shader, 1, &glsource, NULL);
#elif defined(PLATFORM_MACOS)
	glShaderSource(shader, 1, &source, NULL);
#endif
	CHECK_GL_ERROR();

	glCompileShader(shader);
	CHECK_GL_ERROR();

	GLint	result		= GL_FALSE;
	int		logLength	= 0;
	char*	log			= NULL;

	glGetShaderiv(shader, GL_COMPILE_STATUS,  &result);
	CHECK_GL_ERROR();

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	CHECK_GL_ERROR();

	if (logLength > 1)
	{
		log = new char[logLength + 1];
		memset(log, 0, logLength + 1);

		glGetShaderInfoLog(shader, logLength, NULL, log);
		CHECK_GL_ERROR();

		DBG_LOG("=== Shader compile log ===\n%s\n", log);

		SAFE_DELETE_ARRAY(log);
	}

	if (result != GL_TRUE)
	{	
		DBG_LOG("glCompileShader failed with compile status of %i (with log length of %i).", result, logLength);
		DBG_LOG("Source:\n%s", source);
		return 0;
	}
	else
	{
		return shader;
	}
}

ShaderProgram* OpenGL_Renderer::Create_Shader_Program(CompiledShaderHeader* data)
{
	bool bShadersSupported = Supports(RendererFeature::Shaders);
	if (!bShadersSupported)
	{
		std::vector<GLuint> shaders;
		shaders.push_back(-1);
		shaders.push_back(-1);

		return new OpenGL_ShaderProgram(-1, data, shaders);
	}

	GLuint program = glCreateProgram();

	GLuint frag_shader = Compile_Shader(data->fragment_source, ShaderType::Fragment);
	GLuint vert_shader = Compile_Shader(data->vertex_source, ShaderType::Vertex);

	std::vector<GLuint> shaders;

	if (frag_shader == 0 || vert_shader == 0)
	{
		if (frag_shader == 0)
		{
			DBG_LOG("Fragment shader was 0, failed to compile program.");
		}

		if (vert_shader == 0)
		{
			DBG_LOG("Vertex shader was 0, failed to compile program.");
		}

		return NULL;
	}

	glAttachShader(program, frag_shader);
	CHECK_GL_ERROR();
	shaders.push_back(frag_shader);

	glAttachShader(program, vert_shader);
	CHECK_GL_ERROR();
	shaders.push_back(vert_shader);

	glLinkProgram(program);
	CHECK_GL_ERROR();

	GLint	result = GL_FALSE;
	int		logLength = 0;
	char*	log = NULL;

    glGetProgramiv(program, GL_LINK_STATUS,  &result);
	CHECK_GL_ERROR();

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	CHECK_GL_ERROR();

	if (logLength > 1)
	{
		log = new char[logLength + 1];
		memset(log, 0, logLength + 1);
		
		glGetProgramInfoLog(program, logLength, NULL, log);
		CHECK_GL_ERROR();

		DBG_LOG("=== Shader program link log ===\n%s", log);

		SAFE_DELETE_ARRAY(log);
	}
	else
	{
		DBG_LOG("No shader link log available.");
	}

	if (result != GL_TRUE)
	{
		DBG_LOG("glLinkProgram failed with compile status of %i (with log length of %i).", result, logLength);
		return NULL;
	}
	else
	{
		return new OpenGL_ShaderProgram(program, data, shaders);
	}
}

void OpenGL_Renderer::Bind_Render_Target(RenderTarget* target)
{
	bool bSupported = Renderer::Get()->Supports(RendererFeature::FBO);
	if (!bSupported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	OpenGL_RenderTarget* glTarget = dynamic_cast<OpenGL_RenderTarget*>(target);

	if (m_binded_render_target != glTarget)
	{
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Rendertarget Binds");
#endif

		// Make sure commands on previous buffer are complete.
		//glFlush();

		if (glTarget != NULL)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, glTarget->Get_ID());
			CHECK_GL_ERROR();
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			CHECK_GL_ERROR();
		}

		m_binded_render_target = glTarget;
	}
}

RenderTarget* OpenGL_Renderer::Create_Render_Target()
{
	bool bSupported = Renderer::Get()->Supports(RendererFeature::FBO);

	GLuint id = -1;
	if (bSupported)
	{
		glGenFramebuffers(1, &id);
		CHECK_GL_ERROR();
	}

	if (id >= 0)
	{
		if (bSupported)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, id);
			CHECK_GL_ERROR();
		}

		return new OpenGL_RenderTarget(id);
	}
	else
	{
		return NULL;
	}
}


void OpenGL_Renderer::Bind_Array_Buffer(GLuint buffer)
{
	if (buffer != m_array_buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER_ARB, buffer);
	}
	m_array_buffer = buffer;
}

void OpenGL_Renderer::Bind_Vertex_Pointer(int element_count, int format, int stride, GLvoid* buffer)
{
	//if (m_vertex_pointer_state.element_count != element_count ||
	//	m_vertex_pointer_state.format != format ||
	//	m_vertex_pointer_state.stride != stride ||
	//	m_vertex_pointer_state.buffer != buffer)
	{
		glVertexPointer(element_count, format, stride, reinterpret_cast<GLvoid*>(buffer));
	}
	m_vertex_pointer_state.element_count = element_count;
	m_vertex_pointer_state.format = format;
	m_vertex_pointer_state.stride = stride;
	m_vertex_pointer_state.buffer = buffer;
}

void OpenGL_Renderer::Bind_TexCoord_Pointer(int element_count, int format, int stride, GLvoid* buffer)
{
	//if (m_texcoord_pointer_state.element_count != element_count ||
	//	m_texcoord_pointer_state.format != format ||
	//	m_texcoord_pointer_state.stride != stride ||
	//	m_texcoord_pointer_state.buffer != buffer)
	{
		glTexCoordPointer(element_count, format, stride, reinterpret_cast<GLvoid*>(buffer));
	}
	m_texcoord_pointer_state.element_count = element_count;
	m_texcoord_pointer_state.format = format;
	m_texcoord_pointer_state.stride = stride;
	m_texcoord_pointer_state.buffer = buffer;
}

void OpenGL_Renderer::Bind_Color_Pointer(int element_count, int format, int stride, GLvoid* buffer)
{
	//if (m_color_pointer_state.element_count != element_count ||
	//	m_color_pointer_state.format != format ||
	//	m_color_pointer_state.stride != stride ||
	//	m_color_pointer_state.buffer != buffer)
	{
		glColorPointer(element_count, format, stride, reinterpret_cast<GLvoid*>(buffer));
	}
	m_color_pointer_state.element_count = element_count;
	m_color_pointer_state.format = format;
	m_color_pointer_state.stride = stride;
	m_color_pointer_state.buffer = buffer;
}

void OpenGL_Renderer::Bind_Texture(const Texture* texture, int index)
{
	bool shadersSupported = Supports(RendererFeature::Shaders);

	if (*EngineOptions::render_legacy)
	{
		shadersSupported = false;
	}

	OpenGL_Texture* glTexture = const_cast<OpenGL_Texture*>(dynamic_cast<const OpenGL_Texture*>(texture));

	DBG_ASSERT(index >= 0 && index < MAX_BINDED_TEXTURES);

	if (glTexture != NULL)
	{
		glTexture->Update();
	}

	if (m_binded_textures[index] != glTexture)
	{		
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Texture Binds");
#endif

		if (shadersSupported)
		{
			glActiveTexture(GL_TEXTURE0 + index);
			CHECK_GL_ERROR();
		}

		if (glTexture == NULL)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			CHECK_GL_ERROR();
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, glTexture->Get_ID());
			CHECK_GL_ERROR();
		}
		
		m_binded_textures[index] = glTexture;
	}
}

void OpenGL_Renderer::Bind_Material(Material* material)
{
	//if (m_binded_material != material)
	//{
		Bind_Texture(material->Get_Texture()->Get(), 0);

		m_binded_material = material;
	//}
}
	
Material* OpenGL_Renderer::Get_Material()
{
	return m_binded_material;
}
	
void OpenGL_Renderer::Set_Output_Buffers(std::vector<OutputBufferType::Type>& outputs)
{
	if (!Supports(RendererFeature::FBO) || !Supports(RendererFeature::Shaders))
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	GLenum real_outputs[16];
	DBG_ASSERT(outputs.size() < 16);

	int index = 0;
	for (std::vector<OutputBufferType::Type>::iterator iter = outputs.begin(); iter != outputs.end(); iter++, index++)
	{
		OutputBufferType::Type type = *iter;

		switch (type)
		{
		case OutputBufferType::BackBuffer:				real_outputs[index] = GL_BACK;				break;
		case OutputBufferType::RenderTargetTexture0:	real_outputs[index] = GL_COLOR_ATTACHMENT0;	break;
		case OutputBufferType::RenderTargetTexture1:	real_outputs[index] = GL_COLOR_ATTACHMENT1;	break;
		case OutputBufferType::RenderTargetTexture2:	real_outputs[index] = GL_COLOR_ATTACHMENT2;	break;
		case OutputBufferType::RenderTargetTexture3:	real_outputs[index] = GL_COLOR_ATTACHMENT3;	break;
		case OutputBufferType::RenderTargetTexture4:	real_outputs[index] = GL_COLOR_ATTACHMENT4;	break;
		case OutputBufferType::RenderTargetTexture5:	real_outputs[index] = GL_COLOR_ATTACHMENT5;	break;
		case OutputBufferType::RenderTargetTexture6:	real_outputs[index] = GL_COLOR_ATTACHMENT6;	break;
		case OutputBufferType::RenderTargetTexture7:	real_outputs[index] = GL_COLOR_ATTACHMENT7;	break;
		case OutputBufferType::RenderTargetTexture8:	real_outputs[index] = GL_COLOR_ATTACHMENT8;	break;
		case OutputBufferType::RenderTargetTexture9:	real_outputs[index] = GL_COLOR_ATTACHMENT9;	break;
		default:										DBG_ASSERT(false);							break;
		}
	}
	
#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Output Buffer Binds");
#endif

	glDrawBuffers(outputs.size(), real_outputs);
	CHECK_GL_ERROR();
}

void OpenGL_Renderer::Set_Clear_Color(Color color)
{
	if (m_clear_color != color)
	{
		glClearColor(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f);		
		CHECK_GL_ERROR();

		m_clear_color = color;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Clear Color Changes");
#endif
	}

}

Color OpenGL_Renderer::Get_Clear_Color()
{
	return m_clear_color;
}

void OpenGL_Renderer::Set_Clear_Depth(float depth)
{
	if (m_clear_depth != depth)
	{
		glClearDepth(depth);
		CHECK_GL_ERROR();

		m_clear_depth = depth;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Clear Depth Changes");
#endif
	}
}

float OpenGL_Renderer::Get_Clear_Depth()
{
	return m_clear_depth;
}

void OpenGL_Renderer::Set_Cull_Face(RendererOption::Type option)
{
	if (option != m_cull_face)
	{
		glCullFace(option == RendererOption::E_Back ? GL_BACK : GL_FRONT);
		CHECK_GL_ERROR();

		m_cull_face = option;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Cull Face Changes");
#endif
	}
}

RendererOption::Type OpenGL_Renderer::Get_Cull_Face()
{
	return m_cull_face;
}

void OpenGL_Renderer::Set_Depth_Function(RendererOption::Type option)
{
	if (m_depth_function != option)
	{
		if (m_depth_test == true)
		{
			switch (option)
			{
				case RendererOption::E_Lower_Or_Equal:		glDepthFunc(GL_LEQUAL);		break;
				case RendererOption::E_Lower:				glDepthFunc(GL_LESS);		break;
				case RendererOption::E_Greater_Or_Equal:	glDepthFunc(GL_GEQUAL);		break;
				case RendererOption::E_Greater:				glDepthFunc(GL_GREATER);	break;
				case RendererOption::E_Equal:				glDepthFunc(GL_EQUAL);		break;
				case RendererOption::E_Not_Equal:			glDepthFunc(GL_NOTEQUAL);	break;
				case RendererOption::E_Never:				glDepthFunc(GL_NEVER);		break;
				case RendererOption::E_Always:				glDepthFunc(GL_ALWAYS);		break;
				default:									DBG_ASSERT(false);			break;
			}
		}
		CHECK_GL_ERROR();

		m_depth_function = option;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Depth Function Changes");
#endif
	}
}

RendererOption::Type OpenGL_Renderer::Get_Depth_Function()
{
	return m_depth_function;
}

void OpenGL_Renderer::Set_Depth_Test(bool read)
{
	if (read != m_depth_test)
	{
		if (read)
		{
			switch (m_depth_function)
			{
			case RendererOption::E_Lower_Or_Equal:		glDepthFunc(GL_LEQUAL);		break;
			case RendererOption::E_Lower:				glDepthFunc(GL_LESS);		break;
			case RendererOption::E_Greater_Or_Equal:	glDepthFunc(GL_GEQUAL);		break;
			case RendererOption::E_Greater:				glDepthFunc(GL_GREATER);	break;
			case RendererOption::E_Equal:				glDepthFunc(GL_EQUAL);		break;
			case RendererOption::E_Not_Equal:			glDepthFunc(GL_NOTEQUAL);	break;
			case RendererOption::E_Never:				glDepthFunc(GL_NEVER);		break;
			case RendererOption::E_Always:				glDepthFunc(GL_ALWAYS);		break;
			default:									DBG_ASSERT(false);			break;
			}
		}
		else
		{
			glDepthFunc(GL_ALWAYS);
			CHECK_GL_ERROR();
		}

		m_depth_test = read;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Depth Test Changes");
#endif
	}
}

bool OpenGL_Renderer::Get_Depth_Test()
{
	return m_depth_test;
}

void OpenGL_Renderer::Set_Depth_Write(bool write)
{
	if (write != m_depth_write)
	{
		if (write)
		{
			glDepthMask(GL_TRUE);	
			CHECK_GL_ERROR();
		}
		else
		{
			glDepthMask(GL_FALSE);	
			CHECK_GL_ERROR();
		}

		m_depth_write = write;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Depth Test Changes");
#endif
	}
}

bool OpenGL_Renderer::Get_Depth_Write()
{
	return m_depth_write;
}

void OpenGL_Renderer::Set_Alpha_Test(bool depth)
{
	if (m_alpha_test != depth)
	{
		if (depth)
		{
			glEnable(GL_ALPHA_TEST);
			CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
			CHECK_GL_ERROR();
		}

		m_alpha_test = depth;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Alpha Test Changes");
#endif
	}
}

bool OpenGL_Renderer::Get_Alpha_Test()
{
	return m_alpha_test;
}

void OpenGL_Renderer::Set_Blend_Function(RendererOption::Type option)
{
	if (option != m_blend_function)
	{
		switch (option)
		{
			case RendererOption::E_One_One:								glBlendFunc(GL_ONE, GL_ONE);						break;
			case RendererOption::E_Src_Alpha_One_Minus_Src_Alpha:		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	break;
			case RendererOption::E_Src_Alpha_One:						glBlendFunc(GL_SRC_ALPHA, GL_ONE);					break;
			default:													DBG_ASSERT(false);									break;
		}
		CHECK_GL_ERROR();

		m_blend_function = option;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Blend Function Changes");
#endif
	}
}

RendererOption::Type OpenGL_Renderer::Get_Blend_Function()
{
	return m_blend_function;
}

void OpenGL_Renderer::Set_Blend(bool blend)
{
	if (blend != m_blend)
	{
		if (blend)
		{
			glEnable(GL_BLEND);
			CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_BLEND);
			CHECK_GL_ERROR();
		}

		m_blend = blend;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Blend Changes");
#endif
	}
}

bool OpenGL_Renderer::Get_Blend()
{
	return m_blend;
}

void OpenGL_Renderer::Set_Viewport(Rect2D viewport)
{
	::GfxDisplay* display = ::GfxDisplay::Get();

	m_viewport			= viewport; 
	m_viewport.X		= viewport.X;
	m_viewport.Y		= (display->Get_Height() - (viewport.Y + viewport.Height));
	m_viewport.Width	= Max(viewport.Width, 0);
	m_viewport.Height	= Max(viewport.Height, 0);

	glViewport((GLsizei)m_viewport.X, (GLsizei)m_viewport.Y, (GLsizei)m_viewport.Width, (GLsizei)m_viewport.Height);
	CHECK_GL_ERROR();

#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Viewport Changes");
#endif
}

Rect2D OpenGL_Renderer::Set_Viewport()
{
	return m_viewport;
}

void OpenGL_Renderer::Set_Scissor_Test(bool test)
{
	if (test != m_scissor_test)
	{
		if (test)
		{
			glEnable(GL_SCISSOR_TEST);
			CHECK_GL_ERROR();
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
			CHECK_GL_ERROR();
		}

		m_scissor_test = test;
#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Scissor Changes");
#endif
	}
}

void OpenGL_Renderer::Set_Scissor_Rectangle(Rect2D rect)
{
	::GfxDisplay* display = ::GfxDisplay::Get();

	m_scissor_rect			= rect; 
	m_scissor_rect.X		= rect.X;
	m_scissor_rect.Y		= (display->Get_Height() - (rect.Y + rect.Height));
	m_scissor_rect.Width	= Max(rect.Width, 0);
	m_scissor_rect.Height	= Max(rect.Height, 0);

	glScissor((GLsizei)m_scissor_rect.X, (GLsizei)m_scissor_rect.Y, (GLsizei)m_scissor_rect.Width, (GLsizei)m_scissor_rect.Height);
#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Scissor Rectangle Changes");
#endif
}

bool OpenGL_Renderer::Get_Scissor_Test()
{
	return m_scissor_test;
}

Rect2D OpenGL_Renderer:: Get_Scissor_Rectangle()
{
	return m_scissor_rect;
}

void OpenGL_Renderer::Clear_Buffer(bool color, bool depth)
{
	int bits = 0;
	if (color) bits |= GL_COLOR_BUFFER_BIT;
	if (depth) bits |= GL_DEPTH_BUFFER_BIT;
	
	glClear(bits);
	CHECK_GL_ERROR();

#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Buffer Clears");
#endif
}

Texture* OpenGL_Renderer::Create_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags)
{	
	OpenGL_Texture* texture = new OpenGL_Texture(width, height, pitch, format, flags);

	// If we are on the render thread, upload to GPU, otherwise defer to next Bind operation.
	Thread* thread = Thread::Get_Current();
	if (thread == m_render_thread)
	{
		texture->Update();
	}
	else
	{
		MutexLock lock(m_warmup_textures_mutex);
		m_warmup_textures.push_back(texture);
	}

	return texture;
}

Texture* OpenGL_Renderer::Create_Texture(::Pixelmap* Pixelmap, TextureFlags::Type flags)
{	
	OpenGL_Texture* texture = new OpenGL_Texture(Pixelmap, flags);
	
	// If we are on the render thread, upload to GPU, otherwise defer to next Bind operation.
	Thread* thread = Thread::Get_Current();
	if (thread == m_render_thread)
	{
		texture->Update();
	}
	else
	{
		MutexLock lock(m_warmup_textures_mutex);
		m_warmup_textures.push_back(texture);
	}
	
	return texture;
}

void OpenGL_Renderer::Warmup_Textures()
{
	MutexLock lock(m_warmup_textures_mutex);
	for (std::vector<OpenGL_Texture*>::iterator iter = m_warmup_textures.begin(); iter != m_warmup_textures.end(); iter++)
	{
		OpenGL_Texture* tex = *iter;
		DBG_LOG("Warming up texture 0x%08x.", tex);
		tex->Update();
	}
	m_warmup_textures.clear();
}

bool OpenGL_Renderer::Warmup_Textures_Pending()
{
	MutexLock lock(m_warmup_textures_mutex);
	return (m_warmup_textures.size() > 0);
}

void OpenGL_Renderer::Set_World_Matrix(Matrix4 matrix)
{
	m_world_matrix = matrix;
}

Matrix4 OpenGL_Renderer::Get_World_Matrix()
{
	return m_world_matrix;
}

void OpenGL_Renderer::Set_View_Matrix(Matrix4 matrix)
{
	m_view_matrix = matrix;

	// If in legacy mode we cannot render using shaders, so its time
	// to do some immediate rendering funs.
	if (*EngineOptions::render_legacy)
	{
		glMatrixMode(GL_MODELVIEW); 
		glLoadMatrixf(matrix.Elements);
	}
}

Matrix4 OpenGL_Renderer::Get_View_Matrix()
{
	return m_view_matrix;
}

void OpenGL_Renderer::Set_Projection_Matrix(Matrix4 matrix)
{
	m_projection_matrix = matrix;

	// If in legacy mode we cannot render using shaders, so its time
	// to do some immediate rendering funs.
	if (*EngineOptions::render_legacy)
	{
		glMatrixMode(GL_PROJECTION); 
		glLoadMatrixf(matrix.Elements);
	}
}

Matrix4 OpenGL_Renderer::Get_Projection_Matrix()
{
	return m_projection_matrix;
}

Geometry* OpenGL_Renderer::Create_Geometry(GeometryBufferFormat format, PrimitiveType::Type type, int primitives, bool dynamic)
{
	return new OpenGL_Geometry(format, type, primitives, dynamic);
}

RenderQuery* OpenGL_Renderer::Create_Query(RenderQueryType::Type type)
{
	return new OpenGL_RenderQuery(type);
}

Pixelmap* OpenGL_Renderer::Capture_Backbuffer()
{
	if (Renderer::Get()->Supports(RendererFeature::FBO))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		CHECK_GL_ERROR();
	}

	int width = GfxDisplay::Get()->Get_Width();
	int height = GfxDisplay::Get()->Get_Height();

	u8* pixels = new u8[width * height * 4];
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	CHECK_GL_ERROR();

	Pixelmap* pix = new Pixelmap(pixels, width, height, width * 4, PixelmapFormat::R8G8B8A8, true);
	return pix;
}

void OpenGL_Renderer::Begin_Batch(PrimitiveType::Type type)
{	
	DBG_ASSERT_STR(m_in_batch == false, "Tried to start a batch while already in one.");

	m_in_batch = true;
	m_batch_primitive = type;
	m_batch_primitive_count = 0;
	//m_batch_primitive_color = Color::White.To_Vector4();
	m_batch_primitive_colors_counter = 0;
	m_batch_primitive_normals_counter = 0;
	m_batch_primitive_vertices_counter = 0;
	m_batch_primitive_texcoords_counter = 0;
	
#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Batches Rendered");
#endif
}	

void OpenGL_Renderer::Add_Batch_Vertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty, float r, float g, float b, float a)
{
	m_batch_primitive_colors[m_batch_primitive_colors_counter + 0] = r * m_global_primitive_color.X;
	m_batch_primitive_colors[m_batch_primitive_colors_counter + 1] = g * m_global_primitive_color.Y;
	m_batch_primitive_colors[m_batch_primitive_colors_counter + 2] = b * m_global_primitive_color.Z;
	m_batch_primitive_colors[m_batch_primitive_colors_counter + 3] = a * m_global_primitive_color.W;
	m_batch_primitive_normals[m_batch_primitive_normals_counter + 0] = nx;
	m_batch_primitive_normals[m_batch_primitive_normals_counter + 1] = ny;
	m_batch_primitive_normals[m_batch_primitive_normals_counter + 2] = nz;
	m_batch_primitive_texcoords[m_batch_primitive_texcoords_counter + 0] = tx;
	m_batch_primitive_texcoords[m_batch_primitive_texcoords_counter + 1] = ty;
	m_batch_primitive_verticies[m_batch_primitive_vertices_counter + 0] = x;
	m_batch_primitive_verticies[m_batch_primitive_vertices_counter + 1] = y;
	m_batch_primitive_verticies[m_batch_primitive_vertices_counter + 2] = z;

	m_batch_primitive_normals_counter	+= 3;
	m_batch_primitive_colors_counter	+= 4;
	m_batch_primitive_texcoords_counter += 2;
	m_batch_primitive_vertices_counter	+= 3;
}

void OpenGL_Renderer::End_Batch()
{
	DBG_ASSERT_STR(m_in_batch == true, "Tried to end a non-existant batch.");

	Set_Client_State_Enabled(GL_VERTEX_ARRAY, true);
	//Set_Client_State_Enabled(GL_NORMAL_ARRAY, true);
	Set_Client_State_Enabled(GL_COLOR_ARRAY, true);
	Set_Client_State_Enabled(GL_TEXTURE_COORD_ARRAY, true);

	//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);

	Bind_Vertex_Pointer(3, GL_FLOAT, 0, m_batch_primitive_verticies);
	Bind_TexCoord_Pointer(2, GL_FLOAT, 0, m_batch_primitive_texcoords);
	Bind_Color_Pointer(4, GL_FLOAT, 0, m_batch_primitive_colors);

	/*
	CHECK_GL_ERROR();
	glVertexPointer		(3, GL_FLOAT, 0, m_batch_primitive_verticies);
	CHECK_GL_ERROR();

	//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	CHECK_GL_ERROR();
	glTexCoordPointer	(2, GL_FLOAT, 0, m_batch_primitive_texcoords);
	CHECK_GL_ERROR();

	//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	CHECK_GL_ERROR();
	glNormalPointer		(GL_FLOAT, 0,	 m_batch_primitive_normals);
	CHECK_GL_ERROR();

	//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	CHECK_GL_ERROR();
	glColorPointer		(4, GL_FLOAT, 0, m_batch_primitive_colors);
	CHECK_GL_ERROR();
	*/

	switch (m_batch_primitive)
	{
	case PrimitiveType::Line:
		DBG_ASSERT(m_batch_primitive_vertices_counter == m_batch_primitive_count * 6);
		glLineWidth(m_line_width);
		glDrawArrays(GL_LINES, 0, m_batch_primitive_count * 2);
		CHECK_GL_ERROR();
		break;
	case PrimitiveType::Quad:
		DBG_ASSERT(m_batch_primitive_vertices_counter == m_batch_primitive_count * 12);
		glDrawArrays(GL_QUADS, 0, m_batch_primitive_count * 4);
		CHECK_GL_ERROR();
		break;
	case PrimitiveType::Triangle:
		DBG_ASSERT(m_batch_primitive_vertices_counter == m_batch_primitive_count * 9);
		glDrawArrays(GL_TRIANGLES, 0, m_batch_primitive_count * 3);
		CHECK_GL_ERROR();
		break;
	}

	m_in_batch = false;
}

void OpenGL_Renderer::Draw_Buffer(void* data, PrimitiveBufferFormat::Type data_format, PrimitiveType::Type prim_type, int prim_count)
{
	DBG_ASSERT_STR(m_in_batch == true, "Tried to end a non-existant batch.");
	
	char* char_data = reinterpret_cast<char*>(data);

	int stride = 0;
	int stride_offset = 0;
	
	// Calculate stride-size of format.
	if (data_format & PrimitiveBufferFormat::Vertex)
	{
		stride += (sizeof(float) * 3);
	}
	if (data_format & PrimitiveBufferFormat::TexCoord)
	{
		stride += (sizeof(float) * 2);
	}
	if (data_format & PrimitiveBufferFormat::Normal)
	{
		stride += (sizeof(float) * 3);
	}
	if (data_format & PrimitiveBufferFormat::Color)
	{
		stride += (sizeof(float) * 4);
	}

	// Bind all buffers with correct strides.
	if (data_format & PrimitiveBufferFormat::Vertex)
	{
		Set_Client_State_Enabled(GL_VERTEX_ARRAY, true);

		glEnableClientState(GL_VERTEX_ARRAY);
		CHECK_GL_ERROR();
			
		//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		//CHECK_GL_ERROR();
		Bind_Vertex_Pointer(3, GL_FLOAT, stride, char_data + stride_offset);

	//	glVertexPointer(3, GL_FLOAT, stride, char_data + stride_offset);
	//	CHECK_GL_ERROR();

		stride_offset += (sizeof(float) * 3);
	}
	else
	{
		Set_Client_State_Enabled(GL_VERTEX_ARRAY, false);
	}
	
	if (data_format & PrimitiveBufferFormat::TexCoord)
	{
		Set_Client_State_Enabled(GL_TEXTURE_COORD_ARRAY, true);
		
		//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		//CHECK_GL_ERROR();
		Bind_Vertex_Pointer(2, GL_FLOAT, stride, char_data + stride_offset);

		//glTexCoordPointer(2, GL_FLOAT, stride, char_data + stride_offset);
		//CHECK_GL_ERROR();

		stride_offset += (sizeof(float) * 2);
	}
	else
	{
		Set_Client_State_Enabled(GL_TEXTURE_COORD_ARRAY, false);
	}
	
	/*
	if (data_format & PrimitiveBufferFormat::Normal)
	{
		Set_Client_State_Enabled(GL_NORMAL_ARRAY, true);
		
		//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		//CHECK_GL_ERROR();
		glNormalPointer(GL_FLOAT, stride, char_data + stride_offset);
		CHECK_GL_ERROR();

		stride_offset += (sizeof(float) * 3);
	}
	else
	{
		Set_Client_State_Enabled(GL_NORMAL_ARRAY, false);
	}
	*/

	if (data_format & PrimitiveBufferFormat::Color)
	{
		Set_Client_State_Enabled(GL_COLOR_ARRAY, true);

		//glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
		//CHECK_GL_ERROR();
		//glColorPointer(4, GL_FLOAT, stride, char_data + stride_offset);
		//CHECK_GL_ERROR();

		Bind_Color_Pointer(4, GL_FLOAT, stride, char_data + stride_offset);

		stride_offset += (sizeof(float) * 4);
	}
	else
	{
		Set_Client_State_Enabled(GL_COLOR_ARRAY, false);
	}

	// Draw the buffaaaars
	switch (prim_type)
	{
	case PrimitiveType::Line:
		glDrawArrays(GL_LINES, 0, prim_count * 2);
		CHECK_GL_ERROR();
		break;

	case PrimitiveType::Quad:
		glDrawArrays(GL_QUADS, 0, prim_count * 4);
		CHECK_GL_ERROR();
		break;

	case PrimitiveType::Triangle:
		glDrawArrays(GL_TRIANGLES, 0, prim_count * 3);
		CHECK_GL_ERROR();
		break;
	}
}	

void OpenGL_Renderer::Set_Line_Size(float size)
{	
	if (size != m_line_width)
	{
		DBG_ASSERT_STR(m_in_batch == false, "Function cannot be called during batch rendering.");

		glLineWidth(size); 
		glPointSize(size);
		CHECK_GL_ERROR();

		m_line_width = size;

	#ifdef ENABLE_STAT_COLLECTION
		FrameStats::Increment("Line Size Changes");
	#endif
	}
}

float OpenGL_Renderer::Get_Line_Size()
{
	return m_line_width;
}

void OpenGL_Renderer::Set_Vertex_Color(Color color)
{
//	DBG_ASSERT_STR(m_in_batch == false, "Function cannot be called during batch rendering.");

	m_batch_primitive_color = color.To_Vector4();

#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Vertex Color Changes");
#endif
}

void OpenGL_Renderer::Set_Global_Vertex_Color(Color color) 
{
	m_global_primitive_color = color.To_Vector4();

#ifdef ENABLE_STAT_COLLECTION
	FrameStats::Increment("Vertex Color Changes");
#endif
}

void OpenGL_Renderer::Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2)
{	
	DBG_ASSERT_STR(m_in_batch == true, "Function cannot be called outside batch rendering.");
	DBG_ASSERT_STR(m_in_batch == false || m_batch_primitive == PrimitiveType::Line, "Invalid batch primitive draw call.");
	
	if (m_batch_primitive_count >= MAX_BATCH_PRIMITIVES)
	{
		End_Batch();
		Begin_Batch(m_batch_primitive);
	}
	
	Add_Batch_Vertex(x1, y1, z1, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(x2, y2, z2, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);

	m_batch_primitive_count++;
}

void OpenGL_Renderer::Draw_Quad(Vector3 v1, Vector3 v2, Vector3 v3, Vector3 v4)
{
	DBG_ASSERT_STR(m_in_batch == true, "Function cannot be called outside batch rendering.");
	DBG_ASSERT_STR(m_in_batch == false || m_batch_primitive == PrimitiveType::Quad, "Invalid batch primitive draw call.");

	if (m_batch_primitive_count >= MAX_BATCH_PRIMITIVES)
	{
		End_Batch();
		Begin_Batch(m_batch_primitive);
	}

	Add_Batch_Vertex(v1.X, v1.Y, v1.Z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v2.X, v2.Y, v2.Z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v3.X, v3.Y, v3.Z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v4.X, v4.Y, v4.Z, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);

	m_batch_primitive_count++;
}

void OpenGL_Renderer::Draw_Quad(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3, Vector3 v4, Vector2 uv4)
{
	DBG_ASSERT_STR(m_in_batch == true, "Function cannot be called outside batch rendering.");
	DBG_ASSERT_STR(m_in_batch == false || m_batch_primitive == PrimitiveType::Quad, "Invalid batch primitive draw call.");

	if (m_batch_primitive_count >= MAX_BATCH_PRIMITIVES)
	{
		End_Batch();
		Begin_Batch(m_batch_primitive);
	}

	Add_Batch_Vertex(v1.X, v1.Y, v1.Z, 0.0f, 0.0f, 0.0f, uv1.X, uv1.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v2.X, v2.Y, v2.Z, 0.0f, 0.0f, 0.0f, uv2.X, uv2.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v3.X, v3.Y, v3.Z, 0.0f, 0.0f, 0.0f, uv3.X, uv3.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v4.X, v4.Y, v4.Z, 0.0f, 0.0f, 0.0f, uv4.X, uv4.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);

	m_batch_primitive_count++;
}

void OpenGL_Renderer::Draw_Quad(Rect2D bounds, Rect2D uv, float depth)
{
	DBG_ASSERT_STR(m_in_batch == true, "Function cannot be called outside batch rendering.");
	DBG_ASSERT_STR(m_in_batch == false || m_batch_primitive == PrimitiveType::Quad, "Invalid batch primitive draw call.");

	if (m_batch_primitive_count >= MAX_BATCH_PRIMITIVES)
	{
		End_Batch();
		Begin_Batch(m_batch_primitive);
	}

	float left = bounds.X;
	float top = bounds.Y;
	float right = left + bounds.Width;
	float bottom = top + bounds.Height;

	float uv_left	= uv.X;
	float uv_right	= uv.X + uv.Width;
	float uv_top	= 1.0f - uv.Y;
	float uv_bottom = uv_top - uv.Height;

	Add_Batch_Vertex(left, top, depth, 0.0f, 0.0f, 0.0f, uv_left, uv_top, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(left, bottom, depth, 0.0f, 0.0f, 0.0f, uv_left, uv_bottom, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(right, bottom, depth, 0.0f, 0.0f, 0.0f, uv_right, uv_bottom, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(right, top, depth, 0.0f, 0.0f, 0.0f, uv_right, uv_top, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);

	m_batch_primitive_count++;
}

void OpenGL_Renderer::Draw_Triangle(Vector3 v1, Vector2 uv1, Vector3 v2, Vector2 uv2, Vector3 v3, Vector2 uv3)
{
	DBG_ASSERT_STR(m_in_batch == true, "Function cannot be called outside batch rendering.");
	DBG_ASSERT_STR(m_in_batch == false || m_batch_primitive == PrimitiveType::Triangle, "Invalid batch primitive draw call.");

	if (m_batch_primitive_count >= MAX_BATCH_PRIMITIVES)
	{
		End_Batch();
		Begin_Batch(m_batch_primitive);
	}

	Add_Batch_Vertex(v1.X, v1.Y, v1.Z, 0.0f, 0.0f, 0.0f, uv1.X, uv1.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v2.X, v2.Y, v2.Z, 0.0f, 0.0f, 0.0f, uv2.X, uv2.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);
	Add_Batch_Vertex(v3.X, v3.Y, v3.Z, 0.0f, 0.0f, 0.0f, uv3.X, uv3.Y, m_batch_primitive_color.X, m_batch_primitive_color.Y, m_batch_primitive_color.Z, m_batch_primitive_color.W);

	m_batch_primitive_count++;
}

#endif