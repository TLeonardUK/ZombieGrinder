// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDER_PIPELINE_
#define _ENGINE_RENDER_PIPELINE_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Vector3.h"
#include "Generic/Types/Matrix4.h"
#include "Generic/Types/AABB.h"
#include "Generic/Types/Frustum.h"
#include "Generic/Types/Color.h"
#include "Generic/Types/HashTable.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipelineTypes.h"
#include "Engine/Renderer/RenderBatch.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/RenderTarget.h"
#include "Engine/Renderer/Material.h"

#include "Generic/ThirdParty/RapidXML/rapidxml.hpp"
#include "Generic/ThirdParty/RapidXML/rapidxml_iterators.hpp"

#include "Engine/Resources/Reloadable.h"

#include <vector>

class GfxDisplay;
class Camera;
class RenderTarget;
class ShaderProgram;
class Light;
class IDrawable;
class Texture;
class RenderBatch;
class Renderer;

class RenderPipeline : public Singleton<RenderPipeline>, public Reloadable
{
	MEMORY_ALLOCATOR(RenderPipeline, "Rendering");

private:
	RenderPipeline_State*					m_default_state;
	std::vector<RenderPipeline_Texture*>	m_textures;
	std::vector<RenderPipeline_Target*>		m_targets;
	std::vector<RenderPipeline_Shader*>		m_shaders;
	std::vector<RenderPipeline_Pass*>		m_passes;
	std::vector<RenderPipeline_Slot*>		m_slots;

	HashTable<RenderPipeline_Shader*, unsigned int> m_shaders_hashtable;

	//HashTable<RenderBatch*, unsigned int> m_render_batches;
//	RenderBatch* m_render_batches[MAX_RENDER_BATCHES];
//	int m_render_batch_index;
//	int m_previous_batch_hash;
	RenderBatch* m_render_buffer;

	enum 
	{
		render_query_pool_size = 32,
		render_query_pool_buffers = 3,
	};

	bool									m_created_queries;

	RenderQuery*							m_render_query_pool[render_query_pool_buffers][render_query_pool_size];
	std::string								m_render_query_pool_names[render_query_pool_buffers][render_query_pool_size];
	int										m_render_query_pool_allocated[render_query_pool_size];
	int										m_render_query_read_index;
	int										m_render_query_write_index;
	int										m_render_query_index;

	Renderer*								m_renderer;
//	RenderQuery*							m_render_queries[max_render_queries];

	RenderPipeline_Shader*					m_binded_shader_program;
	RenderPipeline_Shader*					m_default_shader;

	Camera*									m_active_camera;
	Light*									m_active_light;

	Vector4									m_ambient_lighting;

	Vector4									m_object_mask;

	std::string								m_config_path;

	RenderPipeline_Pass*					m_current_pass;

	Color									m_batch_global_color;
	bool									m_batch_global_scissor_test;
	Rect2D									m_batch_global_scissor_rect;

	bool									m_draw_entity_debug;
	bool									m_disable_drawables;
	int										m_debug_layer;

	Texture*								m_post_process_output_texture;
	Texture*								m_post_process_input_texture;

	Color									m_game_clear_color;

protected:
	std::string Get_Attribute_Value(rapidxml::xml_node<>* node, const char* name, const char* def);
	std::string Get_Node_Value	   (rapidxml::xml_node<>* node, const char* name, const char* def);

	// Drawing functions.
	void Draw_Pass(const FrameTime& time, RenderPipeline_Pass* pass);
	void Draw_Scene(const FrameTime& time);
	void Draw_Scene_With_Matrices(const FrameTime& time, Matrix4& projection_matrix, Matrix4& view_matrix, Matrix4& world_matrix);

	// Loading functions.
	void Load_Defaults	(rapidxml::xml_node<>* node);
	void Load_Textures	(rapidxml::xml_node<>* node);
	void Load_Targets	(rapidxml::xml_node<>* node);
	void Load_Shaders	(rapidxml::xml_node<>* node);
	void Load_Passes	(rapidxml::xml_node<>* node);
	void Load_Slots		(rapidxml::xml_node<>* node);
	
	RenderPipeline_State*	Load_State	(rapidxml::xml_node<>* node);
	RenderPipeline_Texture*	Load_Texture(rapidxml::xml_node<>* node);
	RenderPipeline_Target*	Load_Target	(rapidxml::xml_node<>* node);
	RenderPipeline_Shader*	Load_Shader	(rapidxml::xml_node<>* node);
	RenderPipeline_Pass*	Load_Pass	(rapidxml::xml_node<>* node);
	RenderPipeline_Slot*	Load_Slot	(rapidxml::xml_node<>* node);

	RenderPipeline_Target*  Get_Target_From_Name	(const char* name);
	RenderPipeline_Texture* Get_Texture_From_Name	(const char* name);
	RenderPipeline_Slot*	Get_Slot_From_Name		(const char* name);

	void Apply_Outputs(const FrameTime& time, RenderPipeline_Pass* pass, int only_index = -1);
	void Apply_State(const FrameTime& time, RenderPipeline_State* state);
	void Apply_Shader(const FrameTime& time, RenderPipeline_Shader* shader);

	Light* Get_Active_Light();

	void Update_Shader_Uniforms();
	void Apply_Default_Shader(const FrameTime& time);

public:

	// Construction functions.
	RenderPipeline(Renderer* renderer);
	~RenderPipeline();

	// Configuration functions.
	bool Load_Config(const char* path);
	void Reset();
	void Reload();
	void Display_Mode_Changed();

	void Set_Disable_Drawables(bool disabled);
	void Set_Draw_Entity_Debug(bool value);

	int Get_Debug_Layer();
	void Set_Debug_Layer(int layer);

	// Base functions.	
	void Draw(const FrameTime& time);
	void Draw_Game(const FrameTime& time);
	void Flush_Batches();

	void Prepare_For_Legacy(); 

	// Get/Set method.
	Camera*					Get_Active_Camera		();
	RenderPipeline_Shader*	Get_Active_Shader		();

	void					Set_Active_Camera		(Camera* camera);

	RenderPipeline_Shader*  Get_Shader_From_Name	(const char* name);
	RenderPipeline_Pass*	Get_Pass_From_Name		(const char* name);

	// Batch rendering.
	RenderBatch* Get_Render_Batch(Texture* texture, PrimitiveType::Type type);

	// Global render state.
	void Set_Render_Batch_Global_Color(Color color);
	Color Get_Render_Batch_Global_Color();
	void Set_Render_Batch_Scissor_Test(bool test);
	void Set_Render_Batch_Scissor_Rectangle(Rect2D rect);

	// Lighting.
	void Set_Ambient_Lighting(Vector4 lighting);
	Vector4 Get_Ambient_Lighting();

	void Set_Game_Clear_Color(Color clr);
	Color Get_Game_Clear_Color();

	void Set_Object_Mask(Vector4 clr);
	Vector4 Get_Object_Mask();

	// Starts and ends profiling scopes. Use with caution, finite amount available!
	int Start_Profile_Scope(const char* name);
	void End_Profile_Scope(int index);

};

struct RenderProfileBlock
{
public:
	RenderProfileBlock(const char* name)
	{
		m_index = RenderPipeline::Get()->Start_Profile_Scope(name);
	}
	~RenderProfileBlock()
	{
		RenderPipeline::Get()->End_Profile_Scope(m_index);
	}

private:
	int m_index;

};

#define RENDER_PROFILE_BLOCK(name) RenderProfileBlock _s_gpu_##__LINE__##_profiler(name);

#endif
