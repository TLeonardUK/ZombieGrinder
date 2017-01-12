// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDER_PIPELINE_TYPES_
#define _ENGINE_RENDER_PIPELINE_TYPES_

#include "Engine/Renderer/RendererTypes.h"

#include "Generic/Types/Color.h"

#include <vector>

class GfxDisplay;
class Camera;
class RenderTarget;
class ShaderProgram;
class Light;
class IDrawable;
class Texture;
class TextureHandle;
class RenderBatch;
class ShaderProgramHandle;

#define MAX_RENDER_BATCHES 1

struct DrawInstance
{
	IDrawable* drawable;
	bool	  transparent;
	float	  draw_depth;
	int		  draw_index;
};

struct RenderPipeline_StateSettingType
{
	enum Type
	{
#define STATE_SETTING(setting_name, setting_enum, setting_type) \
	setting_enum, 
#include "Engine/Renderer/RenderPipeline_StateSettingType.inc"
#undef STATE_SETTING
	};
};

struct RenderPipeline_StateSettingValueType
{
	enum Type
	{
		// Standard types.
		Color,
		Bool,
		Float,
		String,
		Int,
		Enum
	};
};

struct RenderPipeline_StateSetting
{
	MEMORY_ALLOCATOR(RenderPipeline_StateSetting, "Rendering");

public:
	RenderPipeline_StateSettingType::Type	     Type;
	RenderPipeline_StateSettingValueType::Type	 ValueType;

	std::string				StringValue;
	Color					ColorValue;
	float					FloatValue;
	int						IntValue;
	bool					BoolValue;
	RendererOption::Type	EnumValue;
};

struct RenderPipeline_State
{
	MEMORY_ALLOCATOR(RenderPipeline_State, "Rendering");

public:
	RenderPipeline_State();
	~RenderPipeline_State();

	std::vector<RenderPipeline_StateSetting*>	Settings;
};

struct RenderPipeline_MeshPrimitive
{
	enum Type
	{
		Scene,
		Quads
	};
};

struct RenderPipeline_Texture
{
	MEMORY_ALLOCATOR(RenderPipeline_Texture, "Rendering");

public:
	RenderPipeline_Texture();
	~RenderPipeline_Texture();

	std::string							Name;
	Texture*							RawTexture;
	TextureHandle*						TexturePtr;
};

struct RenderPipeline_Target
{
	MEMORY_ALLOCATOR(RenderPipeline_Target, "Rendering");

public:
	RenderPipeline_Target();
	~RenderPipeline_Target();

	RenderTarget*							Target;
	std::string								Name;
	std::vector<RenderPipeline_Texture*>	Attached_Textures;
};

struct RenderPipeline_ShaderUniformType
{
	enum Type
	{
		Texture,

#define SHADER_UNIFORM(uniform_type, uniform_name, uniform_constant) \
	uniform_constant, 
#include "Engine/Renderer/RenderPipeline_ShaderUniformType.inc"
#undef SHADER_UNIFORM

	};
};

struct RenderPipeline_ShaderUniform
{
	MEMORY_ALLOCATOR(RenderPipeline_ShaderUniform, "Rendering");

public:
	RenderPipeline_ShaderUniform();

	std::string								Name;
	RenderPipeline_ShaderUniformType::Type	Type;
	std::string								Value;
	RenderPipeline_Texture*					TexturePtr;
};

struct RenderPipeline_Shader
{
	MEMORY_ALLOCATOR(RenderPipeline_Shader, "Rendering");

public:
	RenderPipeline_Shader();
	~RenderPipeline_Shader();

	std::string									Name;
	ShaderProgramHandle*						Shader_Program;
	std::vector<RenderPipeline_ShaderUniform*>	Uniforms;
};

struct RenderPipeline_PassType
{
	enum Type
	{
		Scene,
		FullscreenQuad,
		Container,
		PostProcess
	};
};

struct RenderPipeline_PassForEachType
{
	enum Type
	{
		NONE,
		Light,
		Shadow_Casting_Light
	};
};

struct RenderPipeline_PassOutputType
{
	enum Type
	{
		Texture,

		// Special "constant" outputs.
		BackBuffer
	};
};

struct RenderPipeline_PassOutput
{
	MEMORY_ALLOCATOR(RenderPipeline_PassOutput, "Rendering");

public:
	RenderPipeline_PassOutput();

	RenderPipeline_PassOutputType::Type Type;
	RenderPipeline_Texture*				TexturePtr;
};

struct RenderPipeline_Pass
{
	MEMORY_ALLOCATOR(RenderPipeline_Pass, "Rendering");

public:
	RenderPipeline_Pass();
	~RenderPipeline_Pass();

	std::string									Name;
	RenderPipeline_PassType::Type				Type;
	bool										Enabled;
	RenderPipeline_Shader*						Shader;
	RenderPipeline_Target*						Target;
	RenderPipeline_State*						State;
	std::vector<RenderPipeline_PassOutput*>		Outputs;

	RenderPipeline_PassForEachType::Type		Foreach;
	std::vector<RenderPipeline_Pass*>			SubPasses;
};

struct RenderPipeline_SlotSortType
{
	enum Type
	{
		NONE,
		Front_To_Back
	};
};

struct RenderPipeline_Slot
{
	MEMORY_ALLOCATOR(RenderPipeline_Slot, "Rendering");

public:
	RenderPipeline_Slot();
	~RenderPipeline_Slot();

	std::string									Name;
	int											NameHash;
	RenderPipeline_Shader*						Shader;
	RenderPipeline_SlotSortType::Type			SortType;
	std::vector<RenderPipeline_Pass*>			Passes;

	static bool Sort_Front_To_Back(const DrawInstance& a, const DrawInstance& b);
};

#endif
