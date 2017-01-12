// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_PROFILER_
#define _GAME_UI_SCENES_UISCENE_PROFILER_

#include "Engine/UI/UIScene.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Renderer/Text/MarkupFontRenderer.h"
#include "Engine/Renderer/PrimitiveRenderer.h"

#include "Engine/Scene/Map/Map.h"

#include "Generic/Types/IntVector3.h"
#include "Generic/Types/AABB.h"
#include "Generic/Types/Color.h"

class UIToolbarItem;
class EditorCamera;
class EditorBackground;
class EditorForeground;
struct AtlasFrame;
class UIPropertyGrid;
struct UIPropertyGridItem;
struct ProfileFrameCaptureNode;

class UIScene_Profiler : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_Profiler, "UI");

private:	
	AtlasFrame*				m_background_frame;

	UIManager*				m_manager;
	FontHandle*				m_font;
	FontRenderer			m_font_renderer;

	int						m_page;

	float						m_persist_nodes_time;
	ProfileFrameCaptureNode*	m_persist_nodes_data;
	int							m_persist_nodes_count;

	enum
	{
		line_height = 16,
		//line_width	= 450,
		line_width	= 275,
		name_width	= 200,
		value_width = 75,
		profile_graph_height = 350,
		profile_layer_height = 20,
		max_profile_depth = 4,
		line_spacing = 2,
		max_frame_time = 17,
		persist_over_time_frames_for = 2
	};

protected:
	void Add_Stats(UIPropertyGrid* grid, UIPropertyGridItem* parent, std::string path, int depth);
	static void Get_Stat_String(UIPropertyGridItem* item, void* value);
	static void Set_Stat_String(UIPropertyGridItem* item, void* value);

	void Draw_Stats(PrimitiveRenderer& pr, MarkupFontRenderer& tr, std::string path, int depth, Rect2D& bounds);

public:
	UIScene_Profiler();
	~UIScene_Profiler();

	UIScene* Get_Background(UIManager* manager);
	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Display_Cursor();
	bool Should_Display_Focus_Cursor();
	bool Is_Focusable();
	bool Should_Fade_Cursor();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);

	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

	void Refresh_State();
	void Refresh_Stats();
	void Refresh(UIManager* manager);
	void Recieve_Event(UIManager* manager, UIEvent e);

};

#endif

