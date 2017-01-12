// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERS_SHADER_
#define _ENGINE_RENDERER_SHADERS_SHADER_

struct ShaderType
{
	enum Type
	{
		Vertex,
		Fragment
	};
};

class Shader
{
	MEMORY_ALLOCATOR(Shader, "Rendering");

private:
	char*				m_source;
	ShaderType::Type	m_type;

protected:

	// Constructor!
	Shader(char* source, ShaderType::Type type);

public:

	// Destructor!
	virtual ~Shader();

};

#endif

