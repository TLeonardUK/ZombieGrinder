// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/Renderer.h"

#include <vector>

ShaderProgram::ShaderProgram(CompiledShaderHeader* data)
	: m_resource(data)
{
}

ShaderProgram::~ShaderProgram()
{
	SAFE_DELETE(m_resource);
}

CompiledShaderHeader* ShaderProgram::Get_Resource()
{
	return m_resource;
}