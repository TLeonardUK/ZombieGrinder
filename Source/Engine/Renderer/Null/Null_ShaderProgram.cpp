// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Null/Null_ShaderProgram.h"

#include "Generic/Helper/StringHelper.h"

Null_ShaderProgram::Null_ShaderProgram(CompiledShaderHeader* data)
	: ShaderProgram(data)
{
}

Null_ShaderProgram::~Null_ShaderProgram()
{
}

void Null_ShaderProgram::Bind_Texture(const char* name, int texture_index)
{
}

void Null_ShaderProgram::Bind_Int(const char* name, int val)
{
}

void Null_ShaderProgram::Bind_Matrix(const char* name, Matrix4 matrix)
{
}

void Null_ShaderProgram::Bind_Vector(const char* name, Vector3 vector)
{
}

void Null_ShaderProgram::Bind_Vector(const char* name, Vector4 vector)
{
}

void Null_ShaderProgram::Bind_Float(const char* name, float value)
{
}

void Null_ShaderProgram::Bind_Bool(const char* name, bool value)
{
}