// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_COMPILED_SHADER_
#define _ENGINE_RESOURCES_COMPILED_SHADER_

#include "Engine/Renderer/RenderPipelineTypes.h"

struct CompiledShaderUniform
{
	RenderPipeline_ShaderUniformType::Type pipeline_type;
	u32 type_hash;
	u32	name_hash;
	u32 value_hash;
	char* type;
	char* name;
	char* value;
};

struct CompiledShaderHeader
{
	char*					name;
	u32						name_hash;
	u32						uniform_count;
	CompiledShaderUniform*	uniforms;
	char*					vertex_source;
	char*					fragment_source;
};

#endif