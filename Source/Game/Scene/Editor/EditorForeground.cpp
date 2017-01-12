// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Editor/EditorForeground.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Game/Scene/Map/RandGen/MapRandGenManager.h"

#include "Game/Runner/Game.h"

#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"

#include "Engine/Resources/ResourceFactory.h"

EditorForeground::EditorForeground(UIScene_Editor* editor)
	: m_depth(9999999.0f)
	, m_editor(editor)
	, m_draw_randgen(false)
	, m_draw_grid(false)
{
	Set_Render_Slot("geometry");
}

void EditorForeground::Draw(const FrameTime& time, RenderPipeline* pipeline, const DrawInstance& instance)
{
	Rect2D bounds = GameEngine::Get()->Get_Scene()->Get_Boundries();

	Game* game = Game::Get();
	Map* map = game->Get_Map();

	pipeline->Flush_Batches();

	Renderer::Get()->Set_Depth_Test(false);
	Renderer::Get()->Set_Depth_Write(true);

	PrimitiveRenderer renderer;

	// Draw map collision.
	for (int i = 0; i < map->Get_Depth(); i++)
	{
		map->Get_Layer(i)->Draw_Collision(time, pipeline);
	}

	// Outline of map.
	renderer.Draw_Wireframe_Quad(bounds, 0.0f, Color::Gray, 2.0f);

	// Horizontal expanse lines.
	float span = 999999999.0f;
	renderer.Draw_Line(Vector3(0.0f, -span, 0.0f), Vector3(0.0f, span, 0.0f), 2.0f, Color::Gray);
	renderer.Draw_Line(Vector3(bounds.Width, -span, 0.0f), Vector3(bounds.Width, span, 0.0f), 2.0f, Color::Gray);

	// Vertical expanse lines.
	renderer.Draw_Line(Vector3(-span, 0.0f, 0.0f), Vector3(span, 0.0f, 0.0f), 2.0f, Color::Gray);
	renderer.Draw_Line(Vector3(-span, bounds.Height, 0.0f), Vector3(span, bounds.Height, 0.0f), 2.0f, Color::Gray);

	// Draw rand-gen template?
	if (m_draw_randgen)
	{
		int room_width = MapRandGenManager::template_room_tile_width * map->Get_Tile_Width();
		int room_height = MapRandGenManager::template_room_tile_height * map->Get_Tile_Height();
		for (float x = 0; x < bounds.Width; x += room_width)
		{
			renderer.Draw_Line(Vector3(x, 0.0f, 0.0f), Vector3(x, bounds.Height, 0.0f), 2.0f, Color::Gray);
		}
		for (float y = 0; y < bounds.Height; y += room_height)
		{
			renderer.Draw_Line(Vector3(0.0f, y, 0.0f), Vector3(bounds.Width, y, 0.0f), 2.0f, Color::Gray);
		}
	}

	// Draw grid emplate?
	if (m_draw_grid)
	{
		int room_width = 10 * map->Get_Tile_Width();
		int room_height = 10 * map->Get_Tile_Height();
		for (float x = 0; x < bounds.Width; x += room_width)
		{
			renderer.Draw_Line(Vector3(x, 0.0f, 0.0f), Vector3(x, bounds.Height, 0.0f), 2.0f, Color::Gray);
		}
		for (float y = 0; y < bounds.Height; y += room_height)
		{
			renderer.Draw_Line(Vector3(0.0f, y, 0.0f), Vector3(bounds.Width, y, 0.0f), 2.0f, Color::Gray);
		}
	}

	// Draw union box for selection.
	m_editor->Draw_Selection(time, pipeline);

	pipeline->Flush_Batches();

	Renderer::Get()->Set_Depth_Test(false);
	Renderer::Get()->Set_Depth_Write(true);
}

float EditorForeground::Get_Draw_Depth()
{
	return m_depth;
}

bool EditorForeground::Is_Visible_From(Rect2D viewport)
{
	return true;
}


