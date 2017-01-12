// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERS_SHADERPROGRAM_
#define _ENGINE_RENDERER_SHADERS_SHADERPROGRAM_

#include "Generic/Types/Matrix4.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Vector4.h"

struct CompiledShaderHeader;
class Texture;

class ShaderProgram
{
	MEMORY_ALLOCATOR(ShaderProgram, "Rendering");

private:
	CompiledShaderHeader* m_resource;

protected:

	// Constructor!
	ShaderProgram(CompiledShaderHeader* header);

public:

	CompiledShaderHeader* Get_Resource();

	// Destructor!
	virtual ~ShaderProgram();
	
	// Variable binding.
	virtual void Bind_Texture	(const char* name, int texture_index) = 0;
	virtual void Bind_Matrix	(const char* name, Matrix4 matrix) = 0;
	virtual void Bind_Vector	(const char* name, Vector3 vector) = 0;
	virtual void Bind_Vector	(const char* name, Vector4 vector) = 0;
	virtual void Bind_Float		(const char* name, float value) = 0;
	virtual void Bind_Int		(const char* name, int value) = 0;
	virtual void Bind_Bool		(const char* name, bool value) = 0;

};

#endif

