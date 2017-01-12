// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef GL_EXTENSION
#error "GL_EXTENSION must be defined when including this file!"
#endif
#ifndef GL_EXTENSION_FALLBACK
#error "GL_EXTENSION_FALLBACK must be defined when including this file!"
#endif
#ifndef GL_REQUIRED_EXTENSION
#error "GL_EXTENSION must be defined when including this file!"
#endif

// Required extensions in the glstring.
GL_REQUIRED_EXTENSION(GL_ARB_vertex_buffer_object)

// NOTE: 
// Try to keep required GL-extensions to version 2.1.

// MacOS dosen't need any of these defines.
#if !defined(PLATFORM_MACOS)

//			 Return Type		Name						// Parameters																							// Entry Vector2 Name			// If true throw error if not found.		
GL_EXTENSION(void,				glGenBuffers,				(GLsizei n COMMA GLuint *buffers),																		glGenBuffersARB,			true)
GL_EXTENSION(void,				glBindBuffer,				(GLenum target COMMA GLuint buffer),																	glBindBufferARB,			true)
GL_EXTENSION(void,				glBufferData,				(GLenum target COMMA GLsizeiptr size COMMA const GLvoid *data COMMA GLenum usage),						glBufferDataARB,			true)
GL_EXTENSION(void,				glDeleteBuffers,			(GLsizei n COMMA const GLuint *buffers),																glDeleteBuffersARB,			true)
GL_EXTENSION(void,				glBufferSubData,			(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data),									glBufferSubDataARB,			false)

#ifndef PLATFORM_LINUX
//GL_EXTENSION(void,				glCompressedTexImage2D,		(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data), glCompressedTexImage2D,		true);
#endif

#ifdef PLATFORM_WIN32
GL_EXTENSION(void,				wglSwapInterval,			(int interval),																							wglSwapIntervalEXT,			false)
#endif
#ifdef PLATFORM_LINUX
GL_EXTENSION(void,				glXSwapInterval,			(int interval),																							glXSwapIntervalEXT,			false)
GL_EXTENSION_FALLBACK(void,		glXSwapInterval,			(int interval),																							glXSwapIntervalMESA,		false)
GL_EXTENSION_FALLBACK(void,		glXSwapInterval,			(int interval),																							glXSwapIntervalSGI,			false)
#endif

// Shaders are optional.
GL_EXTENSION(GLuint,			glCreateShader,				(GLenum type),																							glCreateShader,				false)
GL_EXTENSION(void,				glDeleteShader,				(GLuint shader),																						glDeleteShader,				false)
GL_EXTENSION(void,				glAttachShader,				(GLuint program COMMA GLuint shader),																	glAttachShader,				false)
GL_EXTENSION(void,				glShaderSource,				(GLuint shader COMMA GLsizei count COMMA const GLchar* const *string COMMA const GLint *length),		glShaderSource,				false)
GL_EXTENSION(void,				glCompileShader,			(GLuint shader),																						glCompileShader,			false)
GL_EXTENSION(GLuint,			glCreateProgram,			(),																										glCreateProgram,			false)
GL_EXTENSION(void,				glLinkProgram,				(GLuint program),																						glLinkProgram,				false)
GL_EXTENSION(void,				glDeleteProgram,			(GLuint program),																						glDeleteProgram,			false)
GL_EXTENSION(void,				glGetProgramiv,				(GLuint program COMMA GLenum pname COMMA GLint *params),												glGetProgramiv,				false)
GL_EXTENSION(void,				glGetShaderiv,				(GLuint shader COMMA GLenum pname COMMA GLint *params),													glGetShaderiv,				false)
GL_EXTENSION(void,				glGetShaderInfoLog,			(GLuint shader COMMA GLsizei bufSize COMMA GLsizei *length COMMA GLchar *infoLog),						glGetShaderInfoLog,			false)
GL_EXTENSION(void,				glGetProgramInfoLog,		(GLuint program COMMA GLsizei bufSize COMMA GLsizei *length COMMA GLchar *infoLog),						glGetProgramInfoLog,		false)
GL_EXTENSION(void,				glUseProgram,				(GLuint program),																						glUseProgram,				false)

#ifdef PLATFORM_WIN32
GL_EXTENSION(void,				glActiveTexture,			(GLenum texture),																						glActiveTexture,			false)
#endif

GL_EXTENSION(GLint,				glGetUniformLocation,		(GLuint program COMMA const GLchar *name),																glGetUniformLocation,		false)
GL_EXTENSION(void,				glUniform1i,				(GLint location COMMA GLint v0),																		glUniform1i,				false)
GL_EXTENSION(void,				glUniform1f,				(GLint location COMMA GLfloat v0),																		glUniform1f,				false)
GL_EXTENSION(void,				glUniform3f,				(GLint location COMMA GLfloat v0 COMMA GLfloat v1 COMMA GLfloat v2),									glUniform3f,				false)
GL_EXTENSION(void,				glUniform4f,				(GLint location COMMA GLfloat v0 COMMA GLfloat v1 COMMA GLfloat v2 COMMA GLfloat v3),					glUniform4f,				false)
GL_EXTENSION(void,				glUniformMatrix4fv,			(GLint location COMMA GLsizei count COMMA GLboolean transpose COMMA const GLfloat *value),				glUniformMatrix4fv,			false)
GL_EXTENSION(void,				glDrawBuffers,				(GLsizei n COMMA const GLenum *bufs),																	glDrawBuffers,				false)

// These are used if we use deferred rendering, if not available we fallback to rendering geometry and nothing else.
// All modern cards will support this, but there are still some without support.
GL_EXTENSION(void,				glGenFramebuffers,			(GLsizei n COMMA GLuint *framebuffers),																	glGenFramebuffers,			false)
GL_EXTENSION(void,				glBindFramebuffer,			(GLenum target COMMA GLuint framebuffer),																glBindFramebuffer,			false)
GL_EXTENSION(void,				glFramebufferTexture2D,		(GLenum target COMMA GLenum attachment COMMA GLenum textarget COMMA GLuint texture COMMA GLint level),	glFramebufferTexture2D,		false)
GL_EXTENSION(GLuint,			glCheckFramebufferStatus,	(GLenum target),																						glCheckFramebufferStatus,	false)
GL_EXTENSION(void,				glDeleteFramebuffers,		(GLsizei n COMMA GLuint *framebuffers),																	glDeleteFramebuffers,		false)

// Query stuff is all optional, useful for capturing gpu timings, not much else.
GL_EXTENSION(void,				glGenQueries,				(GLsizei n, GLuint *ids),																				glGenQueries,				false)
GL_EXTENSION(void,				glDeleteQueries,			(GLsizei n, const GLuint *ids),																			glDeleteQueries,			false)
GL_EXTENSION(void,				glBeginQuery,				(GLenum target, GLuint id),																				glBeginQuery,				false)
GL_EXTENSION(void,				glEndQuery,					(GLenum target),																						glEndQuery,					false)
GL_EXTENSION(void,				glGetQueryObjectiv,			(GLuint id, GLenum pname, GLint *params),																glGetQueryObjectiv,			false)
GL_EXTENSION(void,				glGetQueryObjectuiv,		(GLuint id, GLenum pname, GLuint *params),																glGetQueryObjectuiv,		false)
GL_EXTENSION(void,				glGetQueryObjecti64v,		(GLuint id, GLenum pname, GLint64 * params),															glGetQueryObjecti64v,		false)
GL_EXTENSION(void,				glQueryCounter,				(GLuint id, GLenum target),																				glQueryCounter,				false)

// MacOS does need these though, because fucking macos.
#else

#define GL_DEBUG_TYPE_ERROR					0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR	0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR	0x824E
#define GL_DEBUG_TYPE_PORTABILITY			0x824F
#define GL_DEBUG_TYPE_PERFORMANCE			0x8250
#define GL_DEBUG_TYPE_OTHER					0x8251

#define GL_DEBUG_SOURCE_API					0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM		0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER		0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY			0x8249
#define GL_DEBUG_SOURCE_APPLICATION			0x824A
#define GL_DEBUG_SOURCE_OTHER				0x824B

#define GL_DEBUG_OUTPUT						0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS			0x8242

#define GL_DEBUG_SEVERITY_HIGH				0x9146
#define GL_DEBUG_SEVERITY_MEDIUM			0x9147
#define GL_DEBUG_SEVERITY_LOW				0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION		0x826B

#endif // PLATFORM_MACOS

// Debug stuff, all optional.
typedef void (APIENTRY *glDebugMessageCallbackFunc)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

GL_EXTENSION(void,				glDebugMessageCallback,		(glDebugMessageCallbackFunc func, void* userParam),														glDebugMessageCallback,		false)
GL_EXTENSION(void,				glDebugMessageControl,		(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled),		glDebugMessageControl,		false)
