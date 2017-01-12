// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_MENUBACKGROUND_
#define _GAME_UI_SCENES_UISCENE_MENUBACKGROUND_

#include "Engine/UI/UIScene.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"

#include "Game/UI/Scenes/Backgrounds/MenuTitleRenderer.h"

#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include <vector>

class UIScene_MenuBackground : public UIScene
{
	MEMORY_ALLOCATOR(UIScene_MenuBackground, "UI");

private:
	MenuTitleRenderer m_title_renderer;

protected:

public:
	UIScene_MenuBackground();

	const char* Get_Name();
	bool Should_Tick_When_Not_Top();
	bool Should_Render_Lower_Scenes();
	
	void Refresh(UIManager* manager);

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

