// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_LOADINGBACKGROUND_
#define _GAME_UI_SCENES_UISCENE_LOADINGBACKGROUND_

#include "Engine/UI/UIScene.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Game/UI/Scenes/Backgrounds/MenuTitleRenderer.h"

#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include <vector>

class UIScene_LoadingBackground : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_LoadingBackground, "UI");

public:
	enum
	{
		MAX_STYLES = 4,
		MAX_BACKROUND_OVALS = 20
	};

private:
	MenuTitleRenderer m_title_renderer;

	Vector2 m_character_position;
	Vector2 m_logo_position;

	int m_style_index;

	float m_circle_radius_offset;

	static Color m_foreground_colors[MAX_STYLES];
	static Color m_background_colors[MAX_STYLES];

	AtlasFrame* m_character_frames[MAX_STYLES];
	
	AtlasRenderer m_atlas_renderer;

protected:

public:
	UIScene_LoadingBackground();

	const char* Get_Name();
	bool Should_Tick_When_Not_Top();
	bool Should_Render_Lower_Scenes();
	
	void Set_Character_Position(float x, float y);
	void Set_Logo_Position(float x, float y);
	void Set_Style_Index(int index);

	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

