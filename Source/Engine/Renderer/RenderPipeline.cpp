// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/RenderBatch.h"
#include "Engine/IO/StreamFactory.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"
#include "Engine/Renderer/RenderQuery.h"
#include "Engine/Scene/Light.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Renderer/Drawable.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Engine/EngineOptions.h"

#include "Engine/Scene/Collision/CollisionManager.h"
#include "Engine/Scene/Pathing/PathManager.h"
#include "Engine/Scene/PostProcess/PostProcessManager.h"
#include "Engine/Particles/ParticleManager.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Helper/StringHelper.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"

#include "Engine/Profiling/ProfilingManager.h"

#include <algorithm>


// TODO: all into the bin plz. Fuck config based rendering, what was I smoking.


RenderPipeline_State::RenderPipeline_State()
{
}

RenderPipeline_State::~RenderPipeline_State()
{
	for (std::vector<RenderPipeline_StateSetting*>::iterator iter = Settings.begin(); iter != Settings.end(); iter++)
	{
		delete *iter;
	}	
}

RenderPipeline_Texture::RenderPipeline_Texture()
	: TexturePtr(NULL)
	, RawTexture(NULL)
{
}

RenderPipeline_Texture::~RenderPipeline_Texture()
{
	//SAFE_DELETE(Texture);
	SAFE_DELETE(RawTexture);
}

RenderPipeline_Target::RenderPipeline_Target()
	: Target(NULL)
{
}

RenderPipeline_Target::~RenderPipeline_Target()
{
	SAFE_DELETE(Target);
	Attached_Textures.clear();
}

RenderPipeline_Shader::RenderPipeline_Shader()
	: Shader_Program(NULL)
{
}

RenderPipeline_ShaderUniform::RenderPipeline_ShaderUniform()
	: TexturePtr(NULL)
{
}

RenderPipeline_Shader::~RenderPipeline_Shader()
{
	for (std::vector<RenderPipeline_ShaderUniform*>::iterator iter = Uniforms.begin(); iter != Uniforms.end(); iter++)
	{
		delete *iter;
	}	

//	SAFE_DELETE(Shader_Program);
}

RenderPipeline_Pass::RenderPipeline_Pass()
	: Shader(NULL)
	, Target(NULL)
	, State(NULL)
	, Foreach(RenderPipeline_PassForEachType::NONE)
{
}

RenderPipeline_Pass::~RenderPipeline_Pass()
{
	for (std::vector<RenderPipeline_PassOutput*>::iterator iter = Outputs.begin(); iter != Outputs.end(); iter++)
	{
		delete *iter;
	}	
	for (std::vector<RenderPipeline_Pass*>::iterator iter = SubPasses.begin(); iter != SubPasses.end(); iter++)
	{
		delete *iter;
	}

	SAFE_DELETE(State);
}

RenderPipeline_Slot::RenderPipeline_Slot()
	: Shader(NULL)
	, Name("")
	, NameHash(0)
	, SortType(RenderPipeline_SlotSortType::NONE)
{
}

RenderPipeline_Slot::~RenderPipeline_Slot()
{
}

bool RenderPipeline_Slot::Sort_Front_To_Back(const DrawInstance& a, const DrawInstance& b)
{
	return (a.draw_depth < b.draw_depth);
}

RenderPipeline_PassOutput::RenderPipeline_PassOutput()
	: TexturePtr(NULL)
{
}

RenderPipeline::RenderPipeline(Renderer* renderer)
	: m_renderer(renderer)
	, m_default_state(NULL)
	, m_active_camera(NULL)
	, m_active_light(NULL)
	, m_binded_shader_program(NULL)
	, m_default_shader(NULL)
//	, m_render_batch_index(0)
//	, m_previous_batch_hash(0xDEDEDEDE)
	, m_batch_global_color(Color::White)
	, m_batch_global_scissor_test(false)
	, m_draw_entity_debug(false)
	, m_debug_layer(0)
	, m_disable_drawables(false)
	, m_ambient_lighting(0.5f, 0.5f, 0.5f, 1.0f)
	, m_render_query_index(0)
	, m_game_clear_color(Color::Black)
	, m_object_mask(Color::Black.To_Vector4())
	, m_created_queries(false)
{
	m_render_buffer = new RenderBatch();
//	for (int i = 0; i < MAX_RENDER_BATCHES; i++)
//	{
//		m_render_batches[i] = NULL;
//	}
//	memset(m_render_batches, 0, sizeof(RenderBatch*) * MAX_RENDER_BATCHES);
}

RenderPipeline::~RenderPipeline()
{
	Reset();

	for (int i = 0; i < render_query_pool_buffers; i++)
	{
		for (int j = 0; j < render_query_pool_size; j++)
		{
			if (m_render_query_pool[i][j] != NULL) 
			{
				SAFE_DELETE(m_render_query_pool[i][j]);
			}
		}
	}

	SAFE_DELETE(m_render_buffer);
}

void RenderPipeline::Set_Game_Clear_Color(Color clr)
{
	m_game_clear_color = clr;
}

Color RenderPipeline::Get_Game_Clear_Color()
{
	return m_game_clear_color;
}

void RenderPipeline::Set_Object_Mask(Vector4 clr)
{
	m_object_mask = clr;
}

Vector4 RenderPipeline::Get_Object_Mask()
{
	return m_object_mask;
}

void RenderPipeline::Set_Disable_Drawables(bool disabled)
{
	m_disable_drawables = disabled;
}

// Unbinds all resources in preperation for transfering to legacy rendering mode.
void RenderPipeline::Prepare_For_Legacy()
{
	GfxDisplay* display = GfxDisplay::Get();
	Rect2D full_viewport = Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height());	

	std::vector<OutputBufferType::Type> output;
	output.push_back(OutputBufferType::BackBuffer);

	m_renderer->Set_Depth_Test(false);
	m_renderer->Set_Depth_Write(true);
	m_renderer->Set_Viewport(full_viewport); 
	m_renderer->Set_Scissor_Test(false); 
	m_renderer->Bind_Shader_Program(NULL);
	m_renderer->Bind_Render_Target(NULL);
	m_renderer->Set_Output_Buffers(output);
	m_renderer->Clear_Buffer();
}

void RenderPipeline::Reset()
{
	SAFE_DELETE(m_default_state);

	Reset_Reload_Trigger_Files();

	for (std::vector<RenderPipeline_Texture*>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		delete *iter;
	}	
	for (std::vector<RenderPipeline_Target*>::iterator iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		delete *iter;
	}
	for (std::vector<RenderPipeline_Shader*>::iterator iter = m_shaders.begin(); iter != m_shaders.end(); iter++)
	{
		delete *iter;
	}
	for (std::vector<RenderPipeline_Pass*>::iterator iter = m_passes.begin(); iter != m_passes.end(); iter++)
	{
		delete *iter;
	}
	for (std::vector<RenderPipeline_Slot*>::iterator iter = m_slots.begin(); iter != m_slots.end(); iter++)
	{
		delete *iter;
	}
/*	for (int i = 0; i < MAX_RENDER_BATCHES; i++)
	{
		if (m_render_batches[i] != NULL)
		{
			SAFE_DELETE(m_render_batches[i]);
		}
	}
*/
	m_render_buffer->Clear();

	m_slots.clear();
	m_textures.clear();
	m_targets.clear();
	m_shaders.clear();
	m_shaders_hashtable.Clear();
	m_passes.clear();

	m_binded_shader_program = NULL;
	m_default_shader = NULL;
	m_current_pass = NULL;
	m_post_process_output_texture = NULL;
	m_post_process_input_texture = NULL;
	m_default_state = NULL;
}

int RenderPipeline::Get_Debug_Layer()
{
	return m_debug_layer;
}

void RenderPipeline::Set_Debug_Layer(int layer)
{
	m_debug_layer = layer;
}

void RenderPipeline::Set_Draw_Entity_Debug(bool value)
{
	m_draw_entity_debug = value;
}

void  RenderPipeline::Set_Ambient_Lighting(Vector4 lighting)
{
	m_ambient_lighting = lighting;
}

Vector4 RenderPipeline::Get_Ambient_Lighting()
{
	return m_ambient_lighting;
}

bool RenderPipeline::Load_Config(const char* path)
{
	Stream* stream = ResourceFactory::Get()->Open(path, StreamMode::Read);
	if (stream == NULL)
	{
		return false;
	}

	// Load source in a single string.
	int source_len = stream->Length();

	char* buffer = new char[source_len + 1];
	buffer[source_len] = '\0';
	DBG_ASSERT(buffer != NULL);

	stream->ReadBuffer(buffer, 0, source_len);

	// Try and parse XML.
	rapidxml::xml_document<>* document = new rapidxml::xml_document<>();
	try
	{
		document->parse<0>(buffer);
	}
	catch (rapidxml::parse_error error)
	{
		int offset = error.where<char>() - buffer;
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(buffer, offset, line, column);

		DBG_LOG("Failed to parse render pipeline XML with error @ %i:%i: %s", line, column, error.what());
		delete stream;
		delete[] buffer;
		return false;
	}
	
	// Unload old state.
	Reset();
	
	// Store config path for reloading later on.
	m_config_path = path;
	Add_Reload_Trigger_File(path);

	// Find root node.	
	rapidxml::xml_node<>* root = document->first_node("xml", 0, false);
	DBG_ASSERT(root != NULL);

	// Load in defaults first.
	rapidxml::xml_node<>* defaults = root->first_node("defaults", 0, false);
	if (defaults != NULL)
	{
		Load_Defaults(defaults);
	}

	// Load in textures.
	rapidxml::xml_node<>* textures = root->first_node("textures", 0, false);
	if (textures != NULL)
	{
		Load_Textures(textures);
	}
	
	// Load in render targets.
	rapidxml::xml_node<>* targets = root->first_node("targets", 0, false);
	if (targets != NULL)
	{
		Load_Targets(targets);
	}
	
	// Load in shaders.
	rapidxml::xml_node<>* shaders = root->first_node("shaders", 0, false);
	if (shaders != NULL)
	{
		Load_Shaders(shaders);
		DBG_ASSERT_STR(m_default_shader != NULL, "No default shader specified.");
	}
	
	// Load in passes.
	rapidxml::xml_node<>* passes = root->first_node("passes", 0, false);
	if (passes != NULL)
	{
		Load_Passes(passes);
	}

	// Load in slots.
	rapidxml::xml_node<>* slots = root->first_node("slots", 0, false);
	if (passes != NULL)
	{
		Load_Slots(slots);
	}

	// Clean up and return.
	delete stream;
	delete[] buffer;
	return true;
}

Camera* RenderPipeline::Get_Active_Camera()
{
	return m_active_camera;
}

RenderPipeline_Shader* RenderPipeline::Get_Active_Shader()
{
	return m_binded_shader_program;
}

void RenderPipeline::Set_Active_Camera(Camera* camera)
{	
	m_active_camera = camera;

	m_renderer->Set_Projection_Matrix(camera->Get_Projection_Matrix());
	m_renderer->Set_View_Matrix(camera->Get_View_Matrix());
	m_renderer->Set_World_Matrix(Matrix4::Identity());
	
	// Update shader uniforms.
	Update_Shader_Uniforms();

	//m_renderer->Set_Viewport(camera->Get_Viewport());
}

Light* RenderPipeline::Get_Active_Light()
{
	return m_active_light;
}

RenderPipeline_Texture* RenderPipeline::Get_Texture_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Texture*>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Target* RenderPipeline::Get_Target_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Target*>::iterator iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Shader* RenderPipeline::Get_Shader_From_Name(const char* name)
{
	return m_shaders_hashtable.Get(StringHelper::Hash(name));
}

RenderPipeline_Pass* RenderPipeline::Get_Pass_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Pass*>::iterator iter = m_passes.begin(); iter != m_passes.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Slot* RenderPipeline::Get_Slot_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Slot*>::iterator iter = m_slots.begin(); iter != m_slots.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

void RenderPipeline::Apply_Default_Shader(const FrameTime& time)
{
	Apply_Shader(time, m_default_shader);
	Update_Shader_Uniforms();
}

void RenderPipeline::Draw(const FrameTime& time)
{
	GfxDisplay* display = GfxDisplay::Get();
	Rect2D full_viewport = Rect2D(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height());	
	UIManager* ui = GameEngine::Get()->Get_UIManager();

	// Create queries.
	if (!m_created_queries)
	{
		// Create queries.
		for (int i = 0; i < render_query_pool_buffers; i++)
		{
			for (int j = 0; j < render_query_pool_size; j++)
			{
				m_render_query_pool[i][j] = Renderer::Get()->Create_Query(RenderQueryType::Time);
				m_render_query_pool_names[i][j] = "";
			}
			m_render_query_pool_allocated[i] = 0;
		}

		m_created_queries = true;
	}

	// Cancel query indexes.
	m_render_query_read_index = (m_render_query_index + 1) % render_query_pool_buffers;
	m_render_query_write_index = m_render_query_index;
	m_render_query_index = (++m_render_query_index) % render_query_pool_buffers;

	// Get results.0
	//DBG_LOG("===================[FRAME]===================");
	for (int i = 0; i < m_render_query_pool_allocated[m_render_query_read_index]; i++)
	{
		float ret = m_render_query_pool[m_render_query_read_index][i]->Get_Result();
		std::string name = m_render_query_pool_names[m_render_query_read_index][i];

	//	DBG_LOG("Result[%s] = %f", name.c_str(), ret);

		// Root result = full render time.
		if (i == 0)
		{
			GameEngine::Get()->Get_Time()->Set_GPU_Time(ret);
		}
	}

	// Reset ready for this run!
	m_render_query_pool_allocated[m_render_query_write_index] = 0;

	{
		RENDER_PROFILE_BLOCK("Render");

		GameEngine::Get()->Get_Time()->Begin_Draw();

		{
			PROFILE_SCOPE_BUDGETED("Render Setup", 1.0f);
			RENDER_PROFILE_BLOCK("Render Setup");

			// Set UI camera.
			Set_Active_Camera(ui->Get_Camera());

			// Clear back buffer.
			std::vector<OutputBufferType::Type> outputs;
			outputs.push_back(OutputBufferType::BackBuffer);

			m_renderer->Set_Depth_Test(false);
			m_renderer->Set_Depth_Write(true);
			m_renderer->Set_Viewport(full_viewport); 
			m_renderer->Set_Scissor_Test(false); 
			m_renderer->Bind_Render_Target(NULL);
			m_renderer->Set_Output_Buffers(outputs);
			m_renderer->Clear_Buffer();
	
			// Apply default shader.
			Apply_Default_Shader(time);
		}
	
		// Draw the scene/UI.
		{
			PROFILE_SCOPE_BUDGETED("Render UI", 5.0f);
			RENDER_PROFILE_BLOCK("Render UI");

			ui->Draw(time);
		}

		// Flush batches.
		{
			PROFILE_SCOPE_BUDGETED("Flush Batches", 5.0f);
			RENDER_PROFILE_BLOCK("Flush Batches");

			Flush_Batches();
		}

		GameEngine::Get()->Get_Time()->Finish_Draw();
	}

	// Flip buffers.
	{	
		PROFILE_SCOPE_BUDGETED("Flip Buffers", 8.0f);

		GameEngine::Get()->Get_Time()->Begin_VSync();		
		m_renderer->Flip(time);		
		GameEngine::Get()->Get_Time()->Finish_VSync();
	}
}

int RenderPipeline::Start_Profile_Scope(const char* name)
{
	int index = (m_render_query_pool_allocated[m_render_query_write_index]++);
	DBG_ASSERT_STR(index < render_query_pool_size, "Ran out of space: %i/%i", index, render_query_pool_size);

	m_render_query_pool_names[m_render_query_write_index][index] = name;

	RenderQuery* query = m_render_query_pool[m_render_query_write_index][index];
	if (query != NULL)
	{
		query->Begin();
	}

	return index;
}

void RenderPipeline::End_Profile_Scope(int index)
{
	RenderQuery* query = m_render_query_pool[m_render_query_write_index][index];
	if (query != NULL)
	{
		query->Finish();
	}
}

void RenderPipeline::Draw_Game(const FrameTime& time)
{
	GfxDisplay* display = GfxDisplay::Get();
	Rect2D full_viewport = Rect2D(0, 0, display->Get_Width(), display->Get_Height());

	// Apply the default state.
	if (m_default_state != NULL)
	{
		PROFILE_SCOPE("Apply Default State");
		Apply_State(time, m_default_state);
	}

	// Predraw step for all drawables.
	{
		PROFILE_SCOPE("Pre Draw");

		std::vector<IDrawable*>& drawable = GameEngine::Get()->Get_Scene()->Get_Drawables();
		for (std::vector<IDrawable*>::iterator iter = drawable.begin(); iter != drawable.end(); iter++)
		{
 			(*iter)->Pre_Draw();
		}
	}

	// Draw every pass!
	for (std::vector<RenderPipeline_Pass*>::iterator iter = m_passes.begin(); iter != m_passes.end(); iter++)
	{
		RenderPipeline_Pass* pass = *iter;
		RENDER_PROFILE_BLOCK(StringHelper::Format("Pass %s", pass->Name.c_str()).c_str());
		Draw_Pass(time, pass);
	}

	// Remove some temp state variables we no longer need.
	m_active_light = NULL;

	// Reset some stuff.
	m_renderer->Set_Viewport(full_viewport); 	
	Apply_Default_Shader(time);
}

void RenderPipeline::Apply_State(const FrameTime& time, RenderPipeline_State* state)
{
	GfxDisplay* display = GfxDisplay::Get();
	Rect2D full_viewport = Rect2D(0, 0, display->Get_Width(), display->Get_Height());
	Rect2D camera_viewport = m_active_camera->Get_Viewport();
	Rect2D camera_screen_viewport = m_active_camera->Get_Screen_Viewport();

	for (std::vector<RenderPipeline_StateSetting*>::iterator iter = state->Settings.begin(); iter != state->Settings.end(); iter++)
	{
		RenderPipeline_StateSetting* setting = *iter;
		switch (setting->Type)
		{
		case RenderPipeline_StateSettingType::ClearColorValue:	m_renderer->Set_Clear_Color(setting->ColorValue);		break;
		case RenderPipeline_StateSettingType::ClearDepthValue:	m_renderer->Set_Clear_Depth(setting->FloatValue);		break;
		case RenderPipeline_StateSettingType::CullFace:			m_renderer->Set_Cull_Face(setting->EnumValue);			break;
		case RenderPipeline_StateSettingType::DepthFunction:	m_renderer->Set_Depth_Function(setting->EnumValue);		break;
		case RenderPipeline_StateSettingType::DepthTest:		m_renderer->Set_Depth_Test(setting->BoolValue);			break;
		case RenderPipeline_StateSettingType::DepthWrite:		m_renderer->Set_Depth_Write(setting->BoolValue);			break;
		case RenderPipeline_StateSettingType::AlphaTest:		m_renderer->Set_Alpha_Test(setting->BoolValue);			break;
		case RenderPipeline_StateSettingType::Clear:			
			{
				if (setting->BoolValue == true)
				{
					m_renderer->Clear_Buffer(true, false);							
				}
				break;	
			}
		case RenderPipeline_StateSettingType::ClearDepth:			
			{
				if (setting->BoolValue == true)
				{
					m_renderer->Clear_Buffer(false, true);							
				}
				break;	
			}

		case RenderPipeline_StateSettingType::Blend:			m_renderer->Set_Blend(setting->BoolValue);				break;
		case RenderPipeline_StateSettingType::BlendFunction:	m_renderer->Set_Blend_Function(setting->EnumValue);		break;
		case RenderPipeline_StateSettingType::Viewport:
			{
				switch (setting->EnumValue)
				{
				case RendererOption::E_Output:					m_renderer->Set_Viewport(camera_screen_viewport);				break;
				case RendererOption::E_Full:					m_renderer->Set_Viewport(full_viewport);				break;
				default:										DBG_ASSERT(false); 
				}
				break;
			}
		default:												DBG_ASSERT(false); 
		}
	}
}

void RenderPipeline::Apply_Shader(const FrameTime& time, RenderPipeline_Shader* shader)
{
	if (shader != m_binded_shader_program)
	{
		ShaderProgramHandle* prog = shader == NULL ? NULL : shader->Shader_Program;
		if (prog == NULL)
		{
			return;
		}

		m_renderer->Bind_Shader_Program(prog->Get());
		m_binded_shader_program = shader;

		Update_Shader_Uniforms();
	}
}

void RenderPipeline::Update_Shader_Uniforms()
{
	Matrix4 world_matrix					= m_renderer->Get_World_Matrix();
	Matrix4 view_matrix						= m_renderer->Get_View_Matrix();
	Matrix4 projection_matrix				= m_renderer->Get_Projection_Matrix();

	Material* material						= m_renderer->Get_Material();
	int binded_texture_index				= 0 ;

	if (m_binded_shader_program != NULL && m_binded_shader_program->Shader_Program != NULL)
	{
		ShaderProgram* prog = m_binded_shader_program->Shader_Program->Get();

		for (std::vector<RenderPipeline_ShaderUniform*>::iterator iter = m_binded_shader_program->Uniforms.begin(); iter != m_binded_shader_program->Uniforms.end(); iter++)
		{
			RenderPipeline_ShaderUniform* uniform = *iter;
			switch (uniform->Type)
			{
			// Samplers
			case RenderPipeline_ShaderUniformType::Texture:								
				{
					const Texture* tex = uniform->TexturePtr->TexturePtr != NULL ? uniform->TexturePtr->TexturePtr->Get() : uniform->TexturePtr->RawTexture;

					m_renderer->Bind_Texture(tex, binded_texture_index);
					prog->Bind_Texture(uniform->Name.c_str(), binded_texture_index);
					binded_texture_index++;
					break;
				}
			case RenderPipeline_ShaderUniformType::PostProcessResultTexture:								
				{
					const Texture* tex = m_post_process_output_texture;

					m_renderer->Bind_Texture(tex, binded_texture_index);
					prog->Bind_Texture(uniform->Name.c_str(), binded_texture_index);
					binded_texture_index++;
					break;
				}
			case RenderPipeline_ShaderUniformType::PostProcessInputTexture:			
				{
					const Texture* tex = m_post_process_input_texture;

					m_renderer->Bind_Texture(tex, binded_texture_index);
					prog->Bind_Texture(uniform->Name.c_str(), binded_texture_index);
					binded_texture_index++;
					break;
				}
			case RenderPipeline_ShaderUniformType::MaterialTexture:			
				{
					if (material != NULL)
					{
						//DBG_ASSERT(material != NULL && material->Get_Texture() != NULL);
						m_renderer->Bind_Texture(material->Get_Texture()->Get(), binded_texture_index);
						prog->Bind_Texture(uniform->Name.c_str(), binded_texture_index);
						binded_texture_index++;
					}
					break;
				}

			// Floats
			case RenderPipeline_ShaderUniformType::CameraNearClip:						prog->Bind_Float(uniform->Name.c_str(), m_active_camera->Get_Near_Clip());							break;
			case RenderPipeline_ShaderUniformType::CameraFarClip:						prog->Bind_Float(uniform->Name.c_str(), m_active_camera->Get_Far_Clip());							break;
			case RenderPipeline_ShaderUniformType::CameraFOV:							prog->Bind_Float(uniform->Name.c_str(), m_active_camera->Get_FOV());								break;
			case RenderPipeline_ShaderUniformType::MaterialShininess:					prog->Bind_Float(uniform->Name.c_str(), material == NULL ? 0.0f : material->Get_Shininess());		break;
			case RenderPipeline_ShaderUniformType::LightRadius:							prog->Bind_Float(uniform->Name.c_str(), m_active_light->Get_Radius());								break;
			case RenderPipeline_ShaderUniformType::LightOuterRadius:					prog->Bind_Float(uniform->Name.c_str(), m_active_light->Get_Outer_Radius());						break;

			// Ints
			case RenderPipeline_ShaderUniformType::LightType:							prog->Bind_Int(uniform->Name.c_str(), m_active_light->Get_Type());									break;

			// Vector3's
			case RenderPipeline_ShaderUniformType::MaterialSpecular:					prog->Bind_Vector(uniform->Name.c_str(), material == NULL ? Vector3(0.f, 0.f, 0.f) : material->Get_Specular());					break;
			case RenderPipeline_ShaderUniformType::CameraPosition:						prog->Bind_Vector(uniform->Name.c_str(), m_active_camera->Get_Position());										break;
			case RenderPipeline_ShaderUniformType::Resolution:							prog->Bind_Vector(uniform->Name.c_str(), Vector3(m_active_camera->Get_Viewport().Width, m_active_camera->Get_Viewport().Height, 1.0f));			break;
			case RenderPipeline_ShaderUniformType::ScreenResolution:
			{
				float scale_x = m_active_camera->Get_Screen_Viewport().Width / m_active_camera->Get_Viewport().Width;
				float scale_y = m_active_camera->Get_Screen_Viewport().Height / m_active_camera->Get_Viewport().Height;
				prog->Bind_Vector(uniform->Name.c_str(), Vector3(m_active_camera->Get_Screen_Viewport().Width, m_active_camera->Get_Screen_Viewport().Height, 1.0f));			
				break;
			}
			case RenderPipeline_ShaderUniformType::ScreenSize:							prog->Bind_Vector(uniform->Name.c_str(), Vector3((float)GfxDisplay::Get()->Get_Width(), (float)GfxDisplay::Get()->Get_Height(), 1.0f));			break;

			// Vector4's
			case RenderPipeline_ShaderUniformType::DrawColor:							prog->Bind_Vector(uniform->Name.c_str(), Vector4(1.0f, 1.0f, 1.0f, 1.0f));			break;
			case RenderPipeline_ShaderUniformType::ObjectMask:							prog->Bind_Vector(uniform->Name.c_str(), m_object_mask);			break;
			case RenderPipeline_ShaderUniformType::GameClearColor:						prog->Bind_Vector(uniform->Name.c_str(), m_game_clear_color.To_Vector4());			break;

			case RenderPipeline_ShaderUniformType::AmbientLighting:						prog->Bind_Vector(uniform->Name.c_str(), m_ambient_lighting);			break;

			// Matrix4's
			case RenderPipeline_ShaderUniformType::WorldMatrix:							prog->Bind_Matrix(uniform->Name.c_str(), world_matrix);								break;
			case RenderPipeline_ShaderUniformType::ViewMatrix:							prog->Bind_Matrix(uniform->Name.c_str(), view_matrix);								break;
			case RenderPipeline_ShaderUniformType::ProjectionMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), projection_matrix);						break;
			case RenderPipeline_ShaderUniformType::WorldViewMatrix:						prog->Bind_Matrix(uniform->Name.c_str(), (view_matrix * world_matrix));						break;
			case RenderPipeline_ShaderUniformType::WorldViewProjectionMatrix:			prog->Bind_Matrix(uniform->Name.c_str(), (projection_matrix * (view_matrix * world_matrix)));				break;
			case RenderPipeline_ShaderUniformType::InverseWorldMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), world_matrix.Inverse());					break;
			case RenderPipeline_ShaderUniformType::InverseViewMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), view_matrix.Inverse());					break;
			case RenderPipeline_ShaderUniformType::InverseProjectionMatrix:				prog->Bind_Matrix(uniform->Name.c_str(), projection_matrix.Inverse());				break;
			case RenderPipeline_ShaderUniformType::InverseWorldViewMatrix:				prog->Bind_Matrix(uniform->Name.c_str(), (view_matrix * world_matrix).Inverse());										break;
			case RenderPipeline_ShaderUniformType::InverseWorldViewProjectionMatrix:	prog->Bind_Matrix(uniform->Name.c_str(), (projection_matrix * (view_matrix * world_matrix)).Inverse());							break;
			case RenderPipeline_ShaderUniformType::InverseNormalMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), (view_matrix * world_matrix).Inverse().Transpose().Inverse());					break;
			case RenderPipeline_ShaderUniformType::NormalMatrix:						prog->Bind_Matrix(uniform->Name.c_str(), (view_matrix * world_matrix).Inverse().Transpose());							break;
			
			// Special ones!
			case RenderPipeline_ShaderUniformType::TextureSize:
				{
					prog->Bind_Vector(uniform->Name.c_str(), Vector3((float)uniform->TexturePtr->TexturePtr->Get()->Get_Width(), (float)uniform->TexturePtr->TexturePtr->Get()->Get_Height(), 0.0f));	break;
					break;
				}

			default: DBG_ASSERT(false); 
			}
		}
	}
}

void RenderPipeline::Apply_Outputs(const FrameTime& time, RenderPipeline_Pass* pass, int only_index)
{
	std::vector<OutputBufferType::Type> outputs;

	int index = 0;
	for (std::vector<RenderPipeline_PassOutput*>::iterator iter = pass->Outputs.begin(); iter != pass->Outputs.end(); iter++, index++)
	{
		if (only_index >= 0)
		{
			if (index != only_index)
			{
				continue;
			}
		}

		RenderPipeline_PassOutput* output = *iter;
		switch (output->Type)
		{
		case RenderPipeline_PassOutputType::BackBuffer:
			{
				outputs.push_back(OutputBufferType::BackBuffer);
				break;
			}
		case RenderPipeline_PassOutputType::Texture:
			{
				int texture_index = -1;
				DBG_ASSERT(pass->Target != NULL);

				int texindex = 0;
				for (std::vector<RenderPipeline_Texture*>::iterator iter2 = pass->Target->Attached_Textures.begin(); iter2 != pass->Target->Attached_Textures.end(); iter2++, texindex++)
				{
					RenderPipeline_Texture* tex = *iter2;
					if (tex == output->TexturePtr)
					{
						texture_index = texindex;
						break;
					}
				}

				DBG_ASSERT(texture_index >= 0);
				outputs.push_back((OutputBufferType::Type)(OutputBufferType::RenderTargetTexture0 + texture_index));

				break;
			}
		default:
			{
				DBG_ASSERT(false);
				break;
			}
		}
	}
	
	m_renderer->Set_Output_Buffers(outputs);
	//m_renderer->Set_Viewport(m_active_camera->Get_Viewport());
}

void RenderPipeline::Draw_Scene(const FrameTime& time)
{
	// Calculate matricies.
	Matrix4 projection_matrix = m_active_camera->Get_Projection_Matrix();
	Matrix4 view_matrix       = m_active_camera->Get_View_Matrix();
	Matrix4 world_matrix	  = Matrix4::Identity();

	Draw_Scene_With_Matrices(time, projection_matrix, view_matrix, world_matrix);
}

void RenderPipeline::Draw_Scene_With_Matrices(const FrameTime& time, Matrix4& projection_matrix, Matrix4& view_matrix, Matrix4& world_matrix)
{
	// Saves reallocating these vectors each frame, they can get pretty chunky.
	static std::vector<IDrawable*> drawn;
	static std::vector<DrawInstance> slot_contents;

	drawn.clear();
	slot_contents.clear();

	Rect2D camera_bounding_volume = m_active_camera->Get_Bounding_Viewport().Inflate(0.25f, 0.25f); // Inflated slightly to deal with rotated objects.

	// Shader uniforms will need updating!
	m_renderer->Set_Projection_Matrix(projection_matrix);
	m_renderer->Set_View_Matrix(view_matrix);
	m_renderer->Set_World_Matrix(world_matrix);

	// Render all drawables.
	std::vector<IDrawable*>& drawables = GameEngine::Get()->Get_Scene()->Get_Drawables();
	int counter = 0;
	for (std::vector<RenderPipeline_Slot*>::iterator iter = m_slots.begin(); iter != m_slots.end(); iter++)
	{
		RenderPipeline_Slot* slot = *iter;

		// Do we render this slot on this pass?
		bool on_pass = false;
		for (std::vector<RenderPipeline_Pass*>::iterator passiter = slot->Passes.begin(); passiter != slot->Passes.end(); passiter++)
		{
			if (*passiter == m_current_pass)
			{
				on_pass = true;
				break;
			}
		}
		if (on_pass == false)
			continue;
	
		// Gather all drawables in this slot (we should really do this as and when things are added to slots >_>).

		{
			PROFILE_SCOPE("Drawable Collection");

			for (std::vector<IDrawable*>::iterator iter2 = drawables.begin(); iter2 != drawables.end(); iter2++)
			{
				IDrawable*	drawable				= *iter2;
				Camera*		draw_camera				= drawable->Get_Draw_Camera();

				if (drawable->Get_Render_Slot_Hash() == slot->NameHash &&
					(draw_camera == NULL || draw_camera == m_active_camera) &&
					drawable->Is_Visible_From(camera_bounding_volume))
				{
					drawable->Add_Draw_Instances(time, slot_contents);

					//slot_contents.push_back(drawable);
					drawn.push_back(drawable);
				}
			}
		}

		{
			PROFILE_SCOPE("Drawable Sort");

			// Sort slot contents.
			switch (slot->SortType)
			{
			case RenderPipeline_SlotSortType::Front_To_Back:
				std::sort(slot_contents.begin(), slot_contents.end(), RenderPipeline_Slot::Sort_Front_To_Back);
				break;
			}
		}

		// Render all batched drawing.
		Flush_Batches();

		// Apply shader.
		Apply_Shader(time, slot->Shader);
		Update_Shader_Uniforms();
		Renderer::Get()->Set_Depth_Test(false);
		Renderer::Get()->Set_Depth_Write(true);

		int transparent = 0;
		int opaque = 0;

		// Render contents.
		if (m_disable_drawables == false)
		{
			PROFILE_SCOPE("Draw Opaque");

			for (std::vector<DrawInstance>::iterator iter2 = slot_contents.begin(); iter2 != slot_contents.end(); iter2++)
			{
				DrawInstance& instance = *iter2;
				if (instance.transparent == false)
				{
					instance.drawable->Draw(time, this, instance);
					opaque++;
				}
			}
		}

		// Render all transparent batched drawing.
		Flush_Batches();

		// Apply shader.
		Apply_Shader(time, slot->Shader);
		Update_Shader_Uniforms();
		Renderer::Get()->Set_Depth_Test(true);
		Renderer::Get()->Set_Depth_Write(false);

		// Render contents.
		if (m_disable_drawables == false)
		{
			PROFILE_SCOPE("Draw Transparent");

			for (std::vector<DrawInstance>::iterator iter2 = slot_contents.begin(); iter2 != slot_contents.end(); iter2++)
			{
				DrawInstance& instance = *iter2;
				if (instance.transparent == true)
				{
					instance.drawable->Draw(time, this, instance);
					transparent++;
				}
			}
		}

		// Apply shader.
		Apply_Shader(time, slot->Shader);
		Update_Shader_Uniforms();

		Renderer::Get()->Set_Depth_Test(true);
		Renderer::Get()->Set_Depth_Write(false);

		// Render all batched drawing.
		Flush_Batches();

		// Draw particles on top.
		{
			PROFILE_SCOPE("Draw Particles");
			ParticleManager::Get()->Draw_Pass(time, m_current_pass->Name);
		}

		// Render all transparent batched drawing.
		Flush_Batches();

		Renderer::Get()->Set_Depth_Test(false);
	}
	
	// Draw some debugging information.
	if (m_current_pass->Name == "geometry")
	{
		if (CollisionManager::Get()->Get_Show_Collision() ||
			PathManager::Get()->Get_Show_Paths() ||
			m_draw_entity_debug)
		{
			PROFILE_SCOPE("Draw Debug");
		
			Renderer::Get()->Clear_Buffer(false, true);

			// Reset to default shader.
			Apply_Shader(time, m_default_shader);
			Update_Shader_Uniforms();
	
			// Render collision.
			CollisionManager::Get()->Draw(time);

			// Render pathing.
			PathManager::Get()->Draw(time);

			// Draw bounds of each entity.
			if (m_draw_entity_debug == true)
			{
				for (std::vector<IDrawable*>::iterator iter2 = drawn.begin(); iter2 != drawn.end(); iter2++)
				{
					IDrawable* drawable = *iter2;
					drawable->Draw_Debug(time, this);
				}
			}
		}
	}

	// Render all batched drawing.
	Flush_Batches();
}

RenderBatch* RenderPipeline::Get_Render_Batch(Texture* texture, PrimitiveType::Type type)
{
/*	unsigned int ptr		= reinterpret_cast<unsigned long long>(texture);
	unsigned int type_ptr	= (int)type;
	unsigned int hash		= ptr + type; 
	
	RenderBatch* batch		= NULL;

	if (hash != m_previous_batch_hash || m_render_batch_index == 0)
	{
		DBG_ASSERT_STR(m_render_batch_index < MAX_RENDER_BATCHES, "Ran out of render batch space.");
		if (m_render_batches[m_render_batch_index] == NULL)
		{
			DBG_LOG("New render batch for texture 0x%08x type %i. (render batch index = %i)", texture, type, m_render_batch_index);
			m_render_batches[m_render_batch_index] = new RenderBatch(texture, type);
		}
		else
		{
			m_render_batches[m_render_batch_index]->Reset(texture, type);
		}
		
		m_render_batches[m_render_batch_index]->Set_Global_Color(m_batch_global_color);
		m_render_batches[m_render_batch_index]->Set_Global_Scissor_Test(m_batch_global_scissor_test);
		m_render_batches[m_render_batch_index]->Set_Global_Scissor_Rectangle(m_batch_global_scissor_rect);

		m_render_batch_index++;
	}

	m_previous_batch_hash = hash;
	return m_render_batches[m_render_batch_index - 1];
	*/

	m_render_buffer->Reset(texture, type);

	return m_render_buffer;

}

void RenderPipeline::Set_Render_Batch_Global_Color(Color color)
{
	m_batch_global_color = color;

	m_render_buffer->Set_Global_Color(color);
}

Color RenderPipeline::Get_Render_Batch_Global_Color()
{
	return m_batch_global_color;
}

void RenderPipeline::Set_Render_Batch_Scissor_Test(bool test)
{
	m_batch_global_scissor_test = test;
	m_render_buffer->Set_Global_Scissor_Test(m_batch_global_scissor_test);
}

void RenderPipeline::Set_Render_Batch_Scissor_Rectangle(Rect2D rect)
{
	m_batch_global_scissor_rect = rect;
	m_render_buffer->Set_Global_Scissor_Rectangle(m_batch_global_scissor_rect);
}
/*
bool RenderPipeline::Sort_Batches_Front_To_Back(RenderBatch* a, RenderBatch* b)
{
	float d1 = a->Get_Order();
	float d2 = b->Get_Order();

	return (d1 < d2);
}
*/

void RenderPipeline::Flush_Batches()
{
	//PROFILE_SCOPE("Flush Batches");
	
	{
		//PROFILE_SCOPE("Draw Batches");
		m_render_buffer->Draw(this);
	}

	{
		//PROFILE_SCOPE("Clear Batches");
		m_render_buffer->Clear();
	}

//	m_render_batch_index  = 0;
//	m_previous_batch_hash = 0xDEDEDEDE;

	// Sort the batches.
//	std::vector<RenderBatch*> batches;

//	for (HashTable<RenderBatch*, unsigned int>::Iterator iter = m_render_batches.Begin(); iter != m_render_batches.End(); iter++)
//	{
//		RenderBatch* batch = *iter;
//		batch->Draw(this);
//		//batches.push_back(batch);
//	}

/*
	std::sort(batches.begin(), batches.end(), Sort_Batches_Front_To_Back);

	// Draw batches in order.
	for (std::vector<RenderBatch*>::iterator iter = batches.begin(); iter != batches.end(); iter++)
	{
		RenderBatch* batch = *iter;
		batch->Draw(this);
	}
*/
}

void RenderPipeline::Draw_Pass(const FrameTime& time, RenderPipeline_Pass* pass)
{
	PROFILE_SCOPE(pass->Name.c_str());

	// Not enabled? :(
	if (pass->Enabled == false)
	{
		return;
	}

	// Skip all but geometry pass in legacy mode.
	if (*EngineOptions::render_legacy && pass->Name != "geometry")
	{
		return;
	}

	// Store current pass.
	m_current_pass = pass;

	// Attach target.
	{
		PROFILE_SCOPE("Bind Target");

		if (pass->Target != NULL)
		{
			m_renderer->Bind_Render_Target(pass->Target->Target);		
		}
		else
		{
			m_renderer->Bind_Render_Target(NULL);					
		}
		m_renderer->Bind_Texture(NULL, 0); 
	}

	// Attach shader.
	{
		PROFILE_SCOPE("Bind Shader");

		if (pass->Shader != NULL)
		{
			Apply_Shader(time, pass->Shader);
		}
	}

	// Setup outputs.
	{
		PROFILE_SCOPE("Apply Outputs");
		Apply_Outputs(time, pass);
	}

	// Apply default state.
	{
		PROFILE_SCOPE("Apply Default State");

		if (pass->State != NULL)
		{
			Apply_State(time, pass->State);
		}
	}

	// Reset object mask.
	m_object_mask = Color::Black.To_Vector4();

	// Straight-pass?
	if (pass->Foreach == RenderPipeline_PassForEachType::NONE)
	{
		// Time to do some actual rendering!
		if (pass->Type == RenderPipeline_PassType::Scene)
		{
			PROFILE_SCOPE("Draw Scene");

			Draw_Scene(time);
		}
		else if (pass->Type == RenderPipeline_PassType::FullscreenQuad)
		{	
			PROFILE_SCOPE("Draw Fullscreen Quad");
		
			int display_width	= GfxDisplay::Get()->Get_Width();//(int)m_active_camera->Get_Viewport().Width;
			int display_height	= GfxDisplay::Get()->Get_Height();//(int)m_active_camera->Get_Viewport().Height;

			Matrix4 old_matrix = m_renderer->Get_View_Matrix();
			m_renderer->Set_View_Matrix(Matrix4::Identity());
			m_renderer->Set_Projection_Matrix(Matrix4::Orthographic(0.0f, (float)display_width, (float)display_height, 0.0f, 0.0f, 1.0f));

			Update_Shader_Uniforms();

			m_renderer->Begin_Batch(PrimitiveType::Quad);
			m_renderer->Draw_Quad(Rect2D(0, 0, display_width, display_height), Rect2D(0.0f, 0.0f, 1.0f, 1.0f), 0.0f);
			m_renderer->End_Batch();

			m_renderer->Set_View_Matrix(old_matrix);
		}
		else if (pass->Type == RenderPipeline_PassType::PostProcess)
		{
			PROFILE_SCOPE("Draw Post Quads");

			PostProcessManager* pp_manager = PostProcessManager::Get();

			int display_width	= GfxDisplay::Get()->Get_Width();//(int)m_active_camera->Get_Viewport().Width;
			int display_height	= GfxDisplay::Get()->Get_Height();//(int)m_active_camera->Get_Viewport().Height;
			Matrix4 old_matrix  = m_renderer->Get_View_Matrix();

			int output_index = 0;

			Texture* input_texture;
			Texture* output_texture = pass->Outputs.at(0)->TexturePtr->RawTexture;
			m_post_process_output_texture = output_texture;

			m_renderer->Set_World_Matrix(Matrix4::Identity());
			m_renderer->Set_View_Matrix(Matrix4::Identity());
			m_renderer->Set_Projection_Matrix(Matrix4::Orthographic(0.0f, (float)display_width, (float)display_height, 0.0f, 0.0f, 1.0f));
			
			int post_index = m_active_camera->Get_PostProcess_Index();
			
			// Attach target.
			m_renderer->Bind_Render_Target(pass->Target->Target);		
			m_renderer->Bind_Texture(NULL, 0); 

			// Setup outputs.
			Apply_Outputs(time, pass);

			m_renderer->Set_Depth_Test(false);
			m_renderer->Set_Depth_Write(false);
			m_renderer->Set_Alpha_Test(false);
			m_renderer->Set_Vertex_Color(Color::White);

			for (int i = 0; i < pp_manager->Get_Pass_Count(post_index); i++)
			{
				// Set input texture.
				input_texture = pass->Outputs.at(output_index)->TexturePtr->RawTexture;
				output_texture = pass->Outputs.at(1 - output_index)->TexturePtr->RawTexture;
				
				// Attach shader.
				m_post_process_input_texture = input_texture;
				m_post_process_output_texture = output_texture;
				Apply_Shader(time, pp_manager->Get_Pass_Shader(post_index, i));
				pp_manager->Set_Pass_Uniforms(post_index, i);

				// Set render target.
				Apply_Outputs(time, pass, 1 - output_index);

				// Draw the quad!
				m_renderer->Begin_Batch(PrimitiveType::Quad);
				m_renderer->Draw_Quad(Rect2D(0, 0, display_width, display_height), Rect2D(0.0f, 0.0f, 1.0f, 1.0f), 0.0f);
				m_renderer->End_Batch();
				
				output_index = !output_index;
			}

			m_renderer->Set_View_Matrix(old_matrix);
		}
		else
		{
			DBG_ASSERT(false);
		}

		// Do any defined sub-passes.
		//for (std::vector<RenderPipeline_Pass*>::iterator passiter = pass->SubPasses.begin(); passiter != pass->SubPasses.end(); passiter++)
		//{
		//	Draw_Pass(time, *passiter);
		//}
	}

	// Wut
	else
	{
		DBG_ASSERT(false);
	}
}

void RenderPipeline::Reload()
{
	Load_Config(m_config_path.c_str());
}

void RenderPipeline::Display_Mode_Changed()
{
	Reload();
}

std::string RenderPipeline::Get_Attribute_Value(rapidxml::xml_node<>* node, const char* name, const char* def)
{
	rapidxml::xml_attribute<>* sub = node->first_attribute(name, 0, false);
	if (sub != NULL)
	{
		return sub->value();
	}
	else
	{
		if (def == NULL)
		{
			DBG_ASSERT_STR(false, "Missing attribute '%s' in node '%s'.", name, node->name());
		}
		else
		{
			return def;
		}
	}

	return "";
}

std::string RenderPipeline::Get_Node_Value(rapidxml::xml_node<>* node, const char* name, const char* def)
{
	rapidxml::xml_node<>* sub = node->first_node(name, 0, false);
	if (sub != NULL)
	{
		return sub->value();
	}
	else
	{
		if (def == NULL)
		{
			DBG_ASSERT_STR(false, "Missing sub-node '%s' in node '%s'.", name, node->name());
		}
		else
		{
			return def;
		}
	}

	return "";
}

void RenderPipeline::Load_Defaults(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* state_node = parent_node->first_node("state", 0, false);
	if (state_node != NULL)
	{
		m_default_state = Load_State(state_node);
		DBG_LOG("Loading default state.");
	}
}

void RenderPipeline::Load_Textures(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("texture", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Texture* texture = Load_Texture(node);
		m_textures.push_back(texture);
			
		node = node->next_sibling("texture", 0, false);
		DBG_LOG("Loaded pipeline texture: %s", texture->Name.c_str());
	}
}

void RenderPipeline::Load_Targets(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("target", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Target* target = Load_Target(node);
		m_targets.push_back(target);
			
		node = node->next_sibling("target", 0, false);
		DBG_LOG("Loaded pipeline target: %s", target->Name.c_str());
	}
}

void RenderPipeline::Load_Shaders(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("shader", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Shader* shader = Load_Shader(node);
		m_shaders.push_back(shader);
		m_shaders_hashtable.Set(StringHelper::Hash(shader->Name.c_str()), shader);	

		node = node->next_sibling("shader", 0, false);
		DBG_LOG("Loaded pipeline shader: %s", shader->Name.c_str());
	}
}

void RenderPipeline::Load_Passes(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("pass", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Pass* pass = Load_Pass(node);
		m_passes.push_back(pass);
			
		node = node->next_sibling("pass", 0, false);
		DBG_LOG("Loaded pipeline pass: %s", pass->Name.c_str());
	}
}

void RenderPipeline::Load_Slots(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("slot", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Slot* slot = Load_Slot(node);
		m_slots.push_back(slot);
			
		node = node->next_sibling("slot", 0, false);
		DBG_LOG("Loaded pipeline render slot: %s", slot->Name.c_str());
	}
}

RenderPipeline_State* RenderPipeline::Load_State(rapidxml::xml_node<>* node)
{
	RenderPipeline_State* state = new RenderPipeline_State();

	rapidxml::xml_node<>* value_node = node->first_node(NULL);
	while (value_node != NULL)
	{
		RenderPipeline_StateSetting* state_value = new RenderPipeline_StateSetting();
		std::string name  = value_node->name();	
		std::string value = value_node->value();
		
		// Work out type of state.
		bool found = false;
		const char* value_type = "";

#define STATE_SETTING(setting_name, setting_enum, setting_value_type)					\
			if (stricmp(name.c_str(), setting_name) == 0)								\
			{																			\
				state_value->Type = RenderPipeline_StateSettingType::setting_enum;	\
				value_type = setting_value_type;										\
				found = true;															\
			} 
#include "Engine/Renderer/RenderPipeline_StateSettingType.inc"
#undef STATE_SETTING
		
		if (found == false)
		{
			DBG_ASSERT_STR(false, "Invalid state setting '%s'.", name.c_str());
		}
		
		// Parse value.
		if (stricmp("color", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Color;
			DBG_ASSERT_STR(Color::Parse(value.c_str(), state_value->ColorValue), "Could not parse color value in state setting '%s'.", name.c_str());
		}
		else if (stricmp("float", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Float;
			state_value->FloatValue = (float)atof(value.c_str());
		}
		else if (stricmp("int", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Int;
			state_value->IntValue = atoi(value.c_str());
		}
		else if (stricmp("bool", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Bool;
			state_value->BoolValue = (stricmp(value.c_str(), "false") == 0 || stricmp(value.c_str(), "0") == 0 ? false : true);
		}
		else if (stricmp("enum", value_type) == 0)
		{
			found = false;
			
			state_value->ValueType = RenderPipeline_StateSettingValueType::Enum;

#define STATE_SETTING(setting_name, setting_enum)														\
				if (stricmp(value.c_str(), setting_name) == 0)											\
				{																						\
					state_value->EnumValue = RendererOption::setting_enum;	\
					found = true;																		\
				} 
#include "Engine/Renderer/Renderer_RenderOptions.inc"
#undef STATE_SETTING

			if (found == false)
			{
				DBG_ASSERT_STR(false, "Invalid value '%s' for state setting '%s'.", value.c_str(), name.c_str());
			}
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid internal value type for setting '%s'.", name.c_str());
		}

		state->Settings.push_back(state_value);

		value_node = value_node->next_sibling(NULL);
	}

	return state;
}

RenderPipeline_Texture*	RenderPipeline::Load_Texture(rapidxml::xml_node<>* node)
{
	RenderPipeline_Texture* state = new RenderPipeline_Texture();
	state->Name = Get_Attribute_Value(node, "name", NULL);
	
	DBG_ASSERT_STR(Get_Texture_From_Name(state->Name.c_str()) == NULL, "Duplicate texture name '%s'.", state->Name.c_str());

	std::string format_string = Get_Attribute_Value(node, "format", "");
	std::string width_string  = Get_Attribute_Value(node, "width",	"");
	std::string height_string = Get_Attribute_Value(node, "height", "");
	std::string flags_string  = Get_Attribute_Value(node, "flags",	"");
	std::string file_string   = Get_Attribute_Value(node, "file",	"");

	// Calculate texture flags.
	TextureFlags::Type flags = TextureFlags::NONE;
	std::vector<std::string> semi_flags;
	StringHelper::Split(flags_string.c_str(), ',', semi_flags);

	for (std::vector<std::string>::iterator iter = semi_flags.begin(); iter != semi_flags.end(); iter++)
	{
		std::string& val = *iter;
		if (stricmp(val.c_str(), "AllowRepeat") == 0)
		{
			flags = (TextureFlags::Type)((int)flags | (int)TextureFlags::AllowRepeat);
		}
		else
		{			
			DBG_ASSERT_STR(false, "Invalid texture flag '%s' in node '%s'.", format_string.c_str(), state->Name.c_str());
		}
	}

	// Calculate texture format.
	TextureFormat::Type format = TextureFormat::R8G8B8A8;
	if (stricmp(format_string.c_str(), "R8G8B8A8") == 0)
	{
		format = TextureFormat::R8G8B8A8;
	}
	else if (stricmp(format_string.c_str(), "R8G8B8") == 0)
	{
		format = TextureFormat::R8G8B8;
	}
#ifndef PLATFORM_MACOS
	else if (stricmp(format_string.c_str(), "R32FG32FB32FA32F") == 0)
	{
		format = TextureFormat::R32FG32FB32FA32F;
	}
#endif
	else if (stricmp(format_string.c_str(), "DepthFormat") == 0)
	{
		format = TextureFormat::DepthFormat;
	}
	else if (stricmp(format_string.c_str(), "StencilFormat") == 0)
	{
		format = TextureFormat::StencilFormat;
	}
	else if (format_string != "")
	{
		DBG_ASSERT_STR(false, "Invalid texture format '%s' in node '%s'.", format_string.c_str(), state->Name.c_str());
	}
	else if (file_string == "")
	{
		DBG_ASSERT_STR(false, "No texture format specified for node '%s'.", state->Name.c_str());
	}

	// Create and return texture.
	if (file_string != "")
	{
		state->TexturePtr = TextureFactory::Load(file_string.c_str(), flags);
		Add_Reload_Trigger_File(file_string.c_str());

		DBG_ASSERT_STR(state->TexturePtr != NULL,  "Failed to load texture '%s'.", file_string.c_str())
	}
	else
	{
		// Some constants.
		if (stricmp(width_string.c_str(), "DISPLAY_WIDTH") == 0)
		{
			width_string = StringHelper::To_String(GfxDisplay::Get()->Get_Width());
		}
		if (stricmp(height_string.c_str(), "DISPLAY_HEIGHT") == 0)
		{
			height_string = StringHelper::To_String(GfxDisplay::Get()->Get_Height());
		}

		DBG_ASSERT_STR(width_string != "",  "Invalid width '%s' for texture '%s'.", width_string.c_str(), state->Name.c_str())
		DBG_ASSERT_STR(height_string != "", "Invalid height '%s' for texture '%s'.", height_string.c_str(), state->Name.c_str())

		int width  = atoi(width_string.c_str());
		int height = atoi(height_string.c_str());

		state->RawTexture = m_renderer->Create_Texture(width, height, width, format, flags);
	}

	return state;
}

RenderPipeline_Target* RenderPipeline::Load_Target(rapidxml::xml_node<>* node)
{
	RenderPipeline_Target* state = new RenderPipeline_Target();
	state->Name = Get_Attribute_Value(node, "name", NULL);
	state->Target = m_renderer->Create_Render_Target();
	
	DBG_ASSERT_STR(Get_Target_From_Name(state->Name.c_str()) == NULL, "Duplicate target name '%s'.", state->Name.c_str());

	rapidxml::xml_node<>* value_node = node->first_node("attached-texture");
	while (value_node != NULL)
	{
		std::string name = Get_Attribute_Value(value_node, "name", NULL);
		std::string type = Get_Attribute_Value(value_node, "type", NULL);

		RenderPipeline_Texture* texture = Get_Texture_From_Name(name.c_str());
		DBG_ASSERT_STR(texture != NULL, "Missing texture '%s' in render target '%s'.", name.c_str(), state->Name.c_str());

		if (stricmp(type.c_str(), "color") == 0)
		{
			state->Target->Bind_Texture(RenderTargetBufferType::Color, texture->TexturePtr != NULL ? texture->TexturePtr->Get() : texture->RawTexture);
		}
		else if (stricmp(type.c_str(), "depth") == 0)		
		{
			state->Target->Bind_Texture(RenderTargetBufferType::Depth,  texture->TexturePtr != NULL ? texture->TexturePtr->Get() : texture->RawTexture);
		}
		else if (stricmp(type.c_str(), "stencil") == 0)	
		{
			state->Target->Bind_Texture(RenderTargetBufferType::Depth,  texture->TexturePtr != NULL ? texture->TexturePtr->Get() : texture->RawTexture);
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid type '%s' for attached-texture '%s' in render target '%s'.", type.c_str(), name.c_str(), state->Name.c_str());
		}

		state->Attached_Textures.push_back(texture);
		value_node = value_node->next_sibling("attached-texture");
	}

	state->Target->Validate();

	return state;
}

RenderPipeline_Shader* RenderPipeline::Load_Shader(rapidxml::xml_node<>* node)
{
	RenderPipeline_Shader* state = new RenderPipeline_Shader();
	state->Name = Get_Attribute_Value(node, "name", NULL);
	
	// Default shader?
	if (Get_Attribute_Value(node, "default", "false") != "false")
		m_default_shader = state;

	DBG_ASSERT_STR(Get_Shader_From_Name(state->Name.c_str()) == NULL, "Duplicate shader name '%s'.", state->Name.c_str());

	std::string shader_name   = Get_Attribute_Value(node, "resource_name", "");

	// Load shaders.
	state->Shader_Program = ResourceFactory::Get()->Get_Shader(shader_name.c_str());
	if (state->Shader_Program == NULL)
	{
		DBG_LOG("Failed to load shader program '%s'. Falling back to legacy rendering mode.", shader_name.c_str());

		Prepare_For_Legacy();

		*EngineOptions::render_legacy = true;
		*EngineOptions::render_legacy_was_forced = true;	

		return state;
	}

	CompiledShaderHeader* header = state->Shader_Program->Get()->Get_Resource();

	// Load uniforms.
	for (unsigned int i = 0; i < header->uniform_count; i++)
	{
		CompiledShaderUniform* raw = header->uniforms + i;

		RenderPipeline_ShaderUniform* uniform = new RenderPipeline_ShaderUniform();
		uniform->Name  = raw->name;
		uniform->Value = raw->value;

		//TODO: Remove these string comparisons, not like we need them with the binary format.

		bool found = false;
#define SHADER_UNIFORM(uniform_type, uniform_name, uniform_constant)									\
	if (stricmp(raw->type, #uniform_type) == 0 && stricmp(raw->value, uniform_name) == 0)	\
		{																							\
			uniform->Type = RenderPipeline_ShaderUniformType::uniform_constant;					\
			found = true;																			\
		} 
#include "Engine/Renderer/RenderPipeline_ShaderUniformType.inc"
#undef SHADER_UNIFORM

		if (found == false)
		{	
			if (stricmp(raw->type, "texture") == 0)
			{					
				uniform->Type = RenderPipeline_ShaderUniformType::Texture;
				uniform->TexturePtr = Get_Texture_From_Name(raw->value);
				DBG_ASSERT_STR(uniform->TexturePtr != NULL, "Failed to load uniform '%s' in shader '%s', missing texture with name '%s'.", raw->name, state->Name.c_str(), raw->value);				
			}
			else
			{
				DBG_ASSERT_STR(false, "Failed to load uniform '%s' in shader '%s', invalid type '%s'.", raw->name, state->Name.c_str(), raw->type);
			}
		}
		else
		{
			if (uniform->Type == RenderPipeline_ShaderUniformType::TextureSize)
			{
				uniform->TexturePtr = Get_Texture_From_Name(raw->value);
				DBG_ASSERT_STR(uniform->TexturePtr != NULL, "Failed to load uniform '%s' in shader '%s', missing texture with name '%s'.", raw->name, state->Name.c_str(), raw->value);								
			}
		}

		state->Uniforms.push_back(uniform);
	}

	return state;
}

RenderPipeline_Pass* RenderPipeline::Load_Pass(rapidxml::xml_node<>* node)
{
	std::string enabled_string = Get_Attribute_Value(node, "enabled", "");

	RenderPipeline_Pass* state = new RenderPipeline_Pass();
	state->Name		= Get_Attribute_Value(node, "name", NULL);
	state->Enabled	= (stricmp(enabled_string.c_str(), "0") == 0 || stricmp(enabled_string.c_str(), "false") == 0) ? false : true;

	DBG_ASSERT_STR(Get_Pass_From_Name(state->Name.c_str()) == NULL, "Duplicate pass name '%s'.", state->Name.c_str());
	
	// Parse state.
	rapidxml::xml_node<>* state_node = node->first_node("state");
	if (state_node != NULL)
	{
		state->State = Load_State(state_node);
	}
	
	// Parse pass type.
	std::string type_string = Get_Node_Value(node, "type", NULL);
	if (stricmp(type_string.c_str(), "scene") == 0)
	{		
		state->Type = RenderPipeline_PassType::Scene;
	}
	else if (stricmp(type_string.c_str(), "fullscreen_quad") == 0)
	{		
		state->Type = RenderPipeline_PassType::FullscreenQuad;
	}
	else if (stricmp(type_string.c_str(), "container") == 0)
	{		
		state->Type = RenderPipeline_PassType::Container;
	}
	else if (stricmp(type_string.c_str(), "postprocess") == 0)
	{		
		state->Type = RenderPipeline_PassType::PostProcess;
	}
	else
	{
		DBG_ASSERT_STR(false, "Invalid type '%s' in pass '%s'.", type_string.c_str(), state->Name.c_str());
	}

	// Parse target.
	std::string target_name = Get_Node_Value(node, "target", "");
	if (target_name != "")
	{
		state->Target = Get_Target_From_Name(target_name.c_str());
		DBG_ASSERT_STR(state->Target != NULL, "Unknown target '%s' in pass '%s'.", target_name.c_str(), state->Name.c_str());	
	}
	
	// Parse shader.
	std::string shader_name = Get_Node_Value(node, "shader", "");
	if (shader_name != "")
	{
		state->Shader = Get_Shader_From_Name(shader_name.c_str());
		DBG_ASSERT_STR(state->Shader != NULL, "Unknown shader '%s' in pass '%s'.", shader_name.c_str(), state->Name.c_str());	
	}

	// Parse outputs.
	rapidxml::xml_node<>* outputs_node = node->first_node("outputs");
	if (outputs_node != NULL)
	{		
		rapidxml::xml_node<>* value_node = outputs_node->first_node("output");

		while (value_node != NULL)
		{
			const char* name = value_node->value();

			RenderPipeline_PassOutput* output = new RenderPipeline_PassOutput();

			if (stricmp(name, "BACK_BUFFER") == 0)
			{
				output->Type = RenderPipeline_PassOutputType::BackBuffer;
			}
			else
			{
				output->Type = RenderPipeline_PassOutputType::Texture;	
				output->TexturePtr = Get_Texture_From_Name(name);

				DBG_ASSERT_STR(output->TexturePtr != NULL, "Unknown texture '%s' in outputs for pass '%s'.", name, state->Name.c_str());
			}
			
			state->Outputs.push_back(output);

			value_node = value_node->next_sibling("output");
		}
	}

	// Parse for-each value.
	std::string foreach_name = Get_Node_Value(node, "foreach", "");
	if (foreach_name != "")
	{
		if (stricmp(foreach_name.c_str(), "light") == 0)
		{
			state->Foreach = RenderPipeline_PassForEachType::Light;
		}
		else if (stricmp(foreach_name.c_str(), "shadow_casting_light") == 0)
		{
			state->Foreach = RenderPipeline_PassForEachType::Shadow_Casting_Light;
		}
		else
		{
			DBG_ASSERT_STR(state->Shader != NULL, "Unknown foreach value '%s' in pass '%s'.", foreach_name.c_str(), state->Name.c_str());	
		}
	}

	// Load in passes.
	rapidxml::xml_node<>* passes = node->first_node("sub-passes", 0, false);
	if (passes != NULL)
	{
		rapidxml::xml_node<>* sub_pass = passes->first_node("pass", 0, false);
		while (sub_pass != NULL)
		{
			RenderPipeline_Pass* spass = Load_Pass(sub_pass);
			state->SubPasses.push_back(spass);
			
			sub_pass = sub_pass->next_sibling("pass", 0, false);
			DBG_LOG("Loaded pipeline sub-pass: %s", spass->Name.c_str());
		}
	}

	return state;
}
	
RenderPipeline_Slot* RenderPipeline::Load_Slot(rapidxml::xml_node<>* node)
{
	std::string enabled_string = Get_Attribute_Value(node, "enabled", "");

	RenderPipeline_Slot* state = new RenderPipeline_Slot();
	state->Name		= Get_Attribute_Value(node, "name", NULL);
	state->NameHash	= StringHelper::Hash(state->Name.c_str());
	
	DBG_ASSERT_STR(Get_Slot_From_Name(state->Name.c_str()) == NULL, "Duplicate slot name '%s'.", state->Name.c_str());
	
	// Parse sort type.
	std::string sort_string = Get_Node_Value(node, "sort", NULL);
	if (sort_string != "")
	{
		if (stricmp(sort_string.c_str(), "front_to_back") == 0)
		{
			state->SortType = RenderPipeline_SlotSortType::Front_To_Back;
		}
		else if (stricmp(sort_string.c_str(), "none") == 0)
		{		
			state->SortType = RenderPipeline_SlotSortType::NONE;
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid sort-type '%s' in slot '%s'.", sort_string.c_str(), state->Name.c_str());
		}
	}

	// Parse shader.
	std::string shader_name = Get_Node_Value(node, "shader", "");
	if (shader_name != "")
	{
		state->Shader = Get_Shader_From_Name(shader_name.c_str());
		DBG_ASSERT_STR(state->Shader != NULL, "Unknown shader '%s' in slot '%s'.", shader_name.c_str(), state->Name.c_str());	
	}

	// Parse passes
	std::string passes = Get_Node_Value(node, "passes", "");
	if (passes != "")
	{
		std::vector<std::string> split;
		if (StringHelper::Split(passes.c_str(), ',', split) > 0)
		{
			for (std::vector<std::string>::iterator passiter = split.begin(); passiter != split.end(); passiter++)
			{
				std::string pass_name = StringHelper::Trim((*passiter).c_str());
				RenderPipeline_Pass* pass = Get_Pass_From_Name(pass_name.c_str());
				DBG_ASSERT_STR(pass != NULL, "Unknown pass '%s' in slot '%s'.", pass_name.c_str(), state->Name.c_str());	

				state->Passes.push_back(pass);
			}
		}
	}

	return state;
}
	