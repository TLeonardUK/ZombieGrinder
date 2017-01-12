// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#ifndef _ENGINE_RENDERER_OPENGL_SHADERPROGRAM_
#define _ENGINE_RENDERER_OPENGL_SHADERPROGRAM_

#include "Generic/Types/HashTable.h"

#include "Engine/Renderer/Shaders/ShaderProgram.h"

#include "Engine/Renderer/OpenGL/OpenGL_Renderer.h"

class OpenGL_ShaderProgram : public ShaderProgram
{
	MEMORY_ALLOCATOR(OpenGL_ShaderProgram, "Rendering");

private:
	friend class OpenGL_Renderer;

	GLuint m_id;
	std::vector<GLuint> m_shader_ids;

	bool m_supported;

	CompiledShaderHeader* m_resource;

	HashTable<GLint, unsigned int> m_uniform_locations;

private:

	GLint Get_Uniform_Location(const char* name);

	// Constructor!
	OpenGL_ShaderProgram(GLuint program, CompiledShaderHeader* resource, std::vector<GLuint> shaders);

public:

	// Destructor!
	~OpenGL_ShaderProgram();

	// Variable binding.
	void Bind_Texture	(const char* name, int texture_index);
	void Bind_Matrix	(const char* name, Matrix4 matrix);
	void Bind_Vector	(const char* name, Vector3 vector);
	void Bind_Vector	(const char* name, Vector4 vector);
	void Bind_Float		(const char* name, float value);
	void Bind_Int		(const char* name, int value);
	void Bind_Bool		(const char* name, bool value);

	GLuint Get_ID();

};

#endif

#endif

