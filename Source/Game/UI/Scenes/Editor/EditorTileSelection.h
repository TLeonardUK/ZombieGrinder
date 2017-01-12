// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_EDITORTILESELECTION_
#define _GAME_UI_SCENES_UISCENE_EDITORTILESELECTION_

#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"

#include "Engine/Engine/FrameTime.h"

class EditorCamera;
class RenderPipeline;

class EditorTileSelection 
{
	MEMORY_ALLOCATOR(EditorTileSelection, "UI");

private:
	EditorCamera* m_camera;
	Vector2 m_boundry_size;

	bool m_selection_in_progress;
	bool m_is_selecting;
	Vector2 m_selection_start;
	Vector2 m_selection_end;
	Rect2D m_selection_rect;

	Vector2 m_drag_start;
	bool m_dragging;

	bool m_pixel_based;

	double m_trigger_timer;

protected:	
	float Get_Zoom();
	Vector2 Screen_Position_To_Tile_Position(Vector2 position);
	Vector2 Get_Tile_Size();
	bool Tile_Position_In_Boundry(Vector2 position, bool pixel_based);

public:
	EditorTileSelection();
	EditorTileSelection(EditorCamera* camera, Vector2 boundry_size);
	
	Vector2 Get_Cursor_Tile_Position(bool pixel_based);

	bool Is_Selecting();
	Rect2D Get_Selection();

	void Clear();
	bool Update(bool persist,  bool expandable = true, bool pixel_based = false, bool continual = false);

	void Draw(const FrameTime& time, RenderPipeline* pipeline);
	void Draw_Radius(const FrameTime& time, RenderPipeline* pipeline, float radius);

};

#endif

