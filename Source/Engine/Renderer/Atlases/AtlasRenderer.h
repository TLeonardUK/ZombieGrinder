// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLASRENDERER_
#define _ENGINE_RENDERER_ATLASES_ATLASRENDERER_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/HashTable.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Types/Color.h"
#include "Engine/IO/Stream.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasFactory.h"

#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"

struct RenderPipeline_Shader;
class Texture;
class RenderBatch;

class AtlasRenderer 
{
	MEMORY_ALLOCATOR(AtlasRenderer, "Rendering");

private:
	AtlasHandle*	m_atlas;
	RenderBatch*	m_batch;

	bool			m_batched;
	bool			m_in_batch;
	Texture*		m_batch_texture;

	// Begin/End rendering stubs.
	void Begin(Texture* texture, PrimitiveType::Type type);
	void End();

public:

	// Constructors
	AtlasRenderer();
	AtlasRenderer(AtlasHandle* atlas);	
	~AtlasRenderer();	

	// Batched settings.
	void Begin_Batch();
	void End_Batch();

	// Rendering methods.
	void Draw		(Texture* texture, Rect2D uv, Vector2 origin, Rect2D  point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f), bool tiled = false, Vector2 tile_repeats = Vector2(0, 0));
	void Draw_Frame	(Texture* frame, Vector2 point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	void Draw_Frame	(AtlasFrame* frame, Vector2 point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	void Draw_Frame	(AtlasFrame* frame, Rect2D  point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	void Draw_Frame	(const char* frame, Vector2 point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	void Draw_Frame	(const char* frame, Rect2D  point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	
	void Tile_Frame	(AtlasFrame* frame, Rect2D  point, float depth = 0.0f, Color color = Color::White, bool flip_horizontal = false, bool flip_vertical = false, RendererOption::Type blend_mode = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2 scale = Vector2(1.0f, 1.0f), float rotation = 0.0f, Vector4 object_mask = Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	

};

#endif

