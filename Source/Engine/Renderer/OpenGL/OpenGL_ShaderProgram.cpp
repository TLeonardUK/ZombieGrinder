// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef DEDICATED_SERVER_BUILD

#include "Engine/Renderer/OpenGL/OpenGL_ShaderProgram.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Engine/EngineOptions.h"

OpenGL_ShaderProgram::OpenGL_ShaderProgram(GLuint program, CompiledShaderHeader* resource, std::vector<GLuint> shaders)
	: ShaderProgram(resource)
	, m_id(program)
	, m_shader_ids(shaders)
{
	m_supported = Renderer::Get()->Supports(RendererFeature::Shaders);
}

OpenGL_ShaderProgram::~OpenGL_ShaderProgram()
{
	if (m_supported)
	{
		glDeleteProgram(m_id);

		for (std::vector<GLuint>::iterator iter = m_shader_ids.begin(); iter != m_shader_ids.end(); iter++)
		{
			glDeleteShader(*iter);
		}
	}
}

GLuint OpenGL_ShaderProgram::Get_ID()
{
	return m_id;
}

GLint OpenGL_ShaderProgram::Get_Uniform_Location(const char* name)
{
	if (m_supported)
	{
		GLint id;
		unsigned int hash = StringHelper::Hash(name);
		if (m_uniform_locations.Get(hash, id))
		{
			return id;
		}
		else
		{
			id = glGetUniformLocation(m_id, name);
			CHECK_GL_ERROR();

			if (id < 0)
			{
				DBG_LOG("[OpenGL Renderer] OpenGL shader does not contain uniform '%s' that was required.", name)
			}

			m_uniform_locations.Set(hash, id);

			return id;
		}
	}

	return 0;
}

void OpenGL_ShaderProgram::Bind_Texture(const char* name, int texture_index)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniform1i(image_id, texture_index); 
		CHECK_GL_ERROR();
	}
}

void OpenGL_ShaderProgram::Bind_Int(const char* name, int val)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniform1i(image_id, val); 
		CHECK_GL_ERROR();
	}
}

void OpenGL_ShaderProgram::Bind_Matrix(const char* name, Matrix4 matrix)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniformMatrix4fv(image_id, 1, GL_FALSE, matrix.Elements);
		CHECK_GL_ERROR();
	}
}

void OpenGL_ShaderProgram::Bind_Vector(const char* name, Vector3 vector)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniform3f(image_id, vector.X, vector.Y, vector.Z);
		CHECK_GL_ERROR();
	}
}

void OpenGL_ShaderProgram::Bind_Vector(const char* name, Vector4 vector)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniform4f(image_id, vector.X, vector.Y, vector.Z, vector.W);
		CHECK_GL_ERROR();
	}
}

void OpenGL_ShaderProgram::Bind_Float(const char* name, float value)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniform1f(image_id, value);
		CHECK_GL_ERROR();
	}
}

void OpenGL_ShaderProgram::Bind_Bool(const char* name, bool value)
{
	if (!m_supported)
	{
		return;
	}

	if (*EngineOptions::render_legacy)
	{
		return;
	}

	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = Get_Uniform_Location(name);

	if (image_id >= 0)
	{
		glUniform1i(image_id, value);
		CHECK_GL_ERROR();
	}
}

#endif