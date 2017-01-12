// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Shaders/Shader.h"

Shader::Shader(char* source, ShaderType::Type type)
	: m_source(source)
	, m_type(type)
{
}

Shader::~Shader()
{
	SAFE_DELETE_ARRAY(m_source);
}