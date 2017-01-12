// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_NULL_SHADERPROGRAM_
#define _ENGINE_RENDERER_NULL_SHADERPROGRAM_

#include "Generic/Types/HashTable.h"

#include "Engine/Renderer/Shaders/ShaderProgram.h"

#include "Engine/Renderer/Null/Null_Renderer.h"

class Null_ShaderProgram : public ShaderProgram
{
	MEMORY_ALLOCATOR(Null_ShaderProgram, "Rendering");

private:
	friend class Null_Renderer;

	CompiledShaderHeader* m_resource;

private:

	// Constructor!
	Null_ShaderProgram(CompiledShaderHeader* data);

public:

	// Destructor!
	~Null_ShaderProgram();

	// Variable binding.
	void Bind_Texture	(const char* name, int texture_index);
	void Bind_Matrix	(const char* name, Matrix4 matrix);
	void Bind_Vector	(const char* name, Vector3 vector);
	void Bind_Vector	(const char* name, Vector4 vector);
	void Bind_Float		(const char* name, float value);
	void Bind_Int		(const char* name, int value);
	void Bind_Bool		(const char* name, bool value);

};

#endif

