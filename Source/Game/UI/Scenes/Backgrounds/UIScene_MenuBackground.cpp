// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Backgrounds/UIScene_MenuBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Math/Math.h"
#include "Generic/Math/Random.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"

UIScene_MenuBackground::UIScene_MenuBackground()
{
	Set_Layout("menu_background");	

	m_title_renderer.Reset();
}

const char* UIScene_MenuBackground::Get_Name()
{
	return "UIScene_MenuBackground";
}

bool UIScene_MenuBackground::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_MenuBackground::Should_Tick_When_Not_Top()
{
	return true;
}

void UIScene_MenuBackground::Refresh(UIManager* manager)
{
	// Refresh base stuff.
	UIScene::Refresh(manager);

	m_title_renderer.Reset();
}

void UIScene_MenuBackground::Enter(UIManager* manager)
{
	m_title_renderer.Reset();
}	

void UIScene_MenuBackground::Exit(UIManager* manager)
{
}	

void UIScene_MenuBackground::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_MenuBackground::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Render the background elements.
	UIScene::Draw(time, manager, scene_index);

	// Render the title image.
	UIPanel* panel			= Find_Element<UIPanel*>("title_image");
	UIPanel* tagline_panel	= Find_Element<UIPanel*>("title_tagline");
	Rect2D screen_box			= panel->Get_Screen_Box();
	Rect2D tagline_screen_box	= tagline_panel->Get_Screen_Box();

	float logo_x	= screen_box.X + (screen_box.Width * 0.5f);
	float logo_y	= screen_box.Y + (screen_box.Height * 0.5f);

	float tag_x		= tagline_screen_box.X + (tagline_screen_box.Width * 0.5f);
	float tag_y		= tagline_screen_box.Y + (tagline_screen_box.Height * 0.5f);
	
	// We always tick in the draw call
	// as we are a background and sometimes will not get ticked depending on the current
	// top-scene's settings.
	m_title_renderer.Tick(time);
	m_title_renderer.Draw(time, Vector2(logo_x, logo_y), Vector2(tag_x, tag_y));
}



