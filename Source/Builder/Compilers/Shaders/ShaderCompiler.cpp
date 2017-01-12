// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Shaders/ShaderCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Resources/Compiled/Shaders/CompiledShader.h"

#include "Engine/IO/StreamFactory.h"

#include "Engine/IO/PatchedBinaryStream.h"

int ShaderCompiler::Get_Version()
{
	return 4;
}

std::vector<AssetToCompile> ShaderCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("shaders"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("shaders/shader");

	// Load atlases.
	for (std::vector<ConfigFileNode>::iterator atlas_iter = atlases.begin(); atlas_iter != atlases.end(); atlas_iter++)
	{
		ConfigFileNode& node = *atlas_iter;

		AssetToCompile asset;
		asset.Source_Path = file->Get<const char*>("file", node, true);
		asset.Priority = "";

		if (file->Contains("priority", node, true))
		{
			asset.Priority = file->Get<const char*>("priority", node, true);
		}

		// Calculate all dependencies.
		std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);

		ConfigFile asset_file;
		if (!asset_file.Load(resolved_asset_path.c_str()))
		{
			DBG_LOG("[WARNING] Failed to check dependencies for atlas. Bank file does not exist! '%s'.", asset.Source_Path.c_str());
			continue;
		}

		if (asset_file.Contains("fragment", NULL, false))
		{
			std::string image_path = asset_file.Get<const char*>("fragment", NULL, false);
			asset.Dependent_Files.push_back(image_path);
		}

		if (asset_file.Contains("vertex", NULL, false))
		{
			std::string image_path = asset_file.Get<const char*>("vertex", NULL, false);
			asset.Dependent_Files.push_back(image_path);
		}

		result.push_back(asset);
	}

	return result;
}

struct ShaderUniform
{
	const char* name;
	const char* type;
	const char* value;
	RenderPipeline_ShaderUniformType::Type pipeline_type;

	int name_ptr_index;
	int value_ptr_index;
	int type_ptr_index;
};

bool ShaderCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{
	DBG_LOG("Compiling shader '%s'.", asset.Source_Path.c_str());

	// Calculate all dependencies.
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);
	
	// Load asset file.
	ConfigFile asset_file;
	if (!asset_file.Load(resolved_asset_path.c_str()))
	{
		return false;
	}

	const char* fragment_file = NULL;
	const char* vertex_file = NULL;
	const char* name = NULL;

	std::vector<ShaderUniform> uniforms;

	if (asset_file.Contains("fragment", NULL, false))
	{
		fragment_file = asset_file.Get<const char*>("fragment", NULL, false);
	}
	if (asset_file.Contains("vertex", NULL, false))
	{
		vertex_file = asset_file.Get<const char*>("vertex", NULL, false);
	}

	name = asset_file.Get<const char*>("settings/name");

	// Add each of the source-images.
	std::vector<ConfigFileNode> images = asset_file.Get<std::vector<ConfigFileNode> >("uniforms/uniform");
	for (std::vector<ConfigFileNode>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		ConfigFileNode node = *iter;		

		const char* name = asset_file.Get<const char*>("name", node, true);
		const char* type = asset_file.Get<const char*>("type", node, true);
		const char* value = asset_file.Get<const char*>("value", node, true);

		ShaderUniform uniform;
		uniform.name = name;
		uniform.type = type;
		uniform.value = value;

		bool found = false;
#define SHADER_UNIFORM(uniform_type, uniform_name, uniform_constant)								\
		if (stricmp(type, #uniform_type) == 0 && stricmp(value, uniform_name) == 0)	\
		{																							\
			uniform.pipeline_type = RenderPipeline_ShaderUniformType::uniform_constant;				\
			found = true;																			\
		} 
#include "Engine/Renderer/RenderPipeline_ShaderUniformType.inc"
#undef SHADER_UNIFORM

		if (found == false)
		{	
			if (stricmp(type, "texture") == 0)
			{					
				uniform.pipeline_type = RenderPipeline_ShaderUniformType::Texture;		
			}
			else
			{
				DBG_ASSERT_STR(false, "Failed to load uniform '%s', invalid type '%s'.", name, type);
			}
		}

		uniforms.push_back(uniform);
	}

	// Save compiled binary output.
	PackageFile* file = builder->Get_Package_File();
	PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Write);

	// CompiledShaderHeader
	int name_ptr_index		= stream->Create_Pointer();
	stream->Write<u32>(StringHelper::Hash(name));
	stream->Write<u32>(uniforms.size());
	int uniforms_ptr_index	= stream->Create_Pointer();
	int vertex_ptr_index	= stream->Create_Pointer();
	int fragment_ptr_index	= stream->Create_Pointer();
	stream->Align_To_Pointer();

	// Write uniforms.
	stream->Patch_Pointer(uniforms_ptr_index);
	for (std::vector<ShaderUniform>::iterator iter = uniforms.begin(); iter != uniforms.end(); iter++)
	{
		ShaderUniform& uniform = *iter;

		// CompiledShaderUniform
		stream->Write<u32>(uniform.pipeline_type);
		stream->Write<u32>(StringHelper::Hash(uniform.type));
		stream->Write<u32>(StringHelper::Hash(uniform.name));
		stream->Write<u32>(StringHelper::Hash(uniform.value));
		uniform.type_ptr_index = stream->Create_Pointer();
		uniform.name_ptr_index = stream->Create_Pointer();
		uniform.value_ptr_index = stream->Create_Pointer();
		stream->Align_To_Pointer();
	}

	// Write vertex data.
	if (vertex_file != NULL)
	{
		stream->Patch_Pointer(vertex_ptr_index);

		std::string output;
		std::string resolved = builder->Resolve_Asset_Path(vertex_file);
		DBG_ASSERT_STR(StreamFactory::Load_Text(resolved.c_str(), &output), "Failed to load vertex shader file '%s'.", vertex_file);

		stream->WriteNullTerminatedString(output.c_str());
	}

	// Write fragment data.
	if (vertex_file != NULL)
	{
		stream->Patch_Pointer(fragment_ptr_index);

		std::string output;
		std::string resolved = builder->Resolve_Asset_Path(fragment_file);
		DBG_ASSERT_STR(StreamFactory::Load_Text(resolved.c_str(), &output), "Failed to load fragment shader file '%s'.", vertex_file);

		stream->WriteNullTerminatedString(output.c_str());
	}

	// Write uniform name strings.
	for (std::vector<ShaderUniform>::iterator iter = uniforms.begin(); iter != uniforms.end(); iter++)
	{
		ShaderUniform& uniform = *iter;

		// CompiledShaderUniform
		stream->Patch_Pointer(uniform.type_ptr_index);
		stream->WriteNullTerminatedString(uniform.type);
		stream->Patch_Pointer(uniform.name_ptr_index);
		stream->WriteNullTerminatedString(uniform.name);
		stream->Patch_Pointer(uniform.value_ptr_index);
		stream->WriteNullTerminatedString(uniform.value);
	}

	// Write name string.
	stream->Patch_Pointer(name_ptr_index);
	stream->WriteNullTerminatedString(name);

	stream->Close();
	SAFE_DELETE(stream);

//	stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Read);
//	CompiledShaderHeader* header = reinterpret_cast<CompiledShaderHeader*>(stream->Get_Data());
//	stream->Close();
//	SAFE_DELETE(stream);

	return true;
}

void ShaderCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	// Calculate all dependencies.
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	// Add compiled file to package.
	builder->Get_Package_File()->Add_Chunk(compiled_asset_path.c_str(), asset.Source_Path.c_str(), PackageFileChunkType::Shader, false, asset.Priority.c_str(), asset.Out_Of_Date);
}