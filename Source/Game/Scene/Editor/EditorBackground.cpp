// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Editor/EditorBackground.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Engine/Resources/ResourceFactory.h"

EditorBackground::EditorBackground(UIScene_Editor* editor)
	: m_depth(0.0f)
	, m_editor(editor)
{
	Set_Render_Slot("geometry");

	m_background_frame = ResourceFactory::Get()->Get_Atlas_Frame("editor_background");
}

void EditorBackground::Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance)
{
	Rect2D bounds = GameEngine::Get()->Get_Scene()->Get_Boundries();
	
	Renderer::Get()->Set_Depth_Write(false);

	AtlasRenderer renderer;
	renderer.Tile_Frame(
		m_background_frame, 
		bounds, 
		m_depth, 
		Color::White,
		false, 
		false, 
		RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, 
		Vector2(1.0f, 1.0f), 
		0.0f
	);

	pipeline->Flush_Batches();
	
	Renderer::Get()->Set_Depth_Write(true);
}

float EditorBackground::Get_Draw_Depth()
{
	return m_depth - 1000.0f; // Bias to always go lower than bottom map layer.
}

bool EditorBackground::Is_Visible_From(Rect2D viewport)
{
	return true;
}


