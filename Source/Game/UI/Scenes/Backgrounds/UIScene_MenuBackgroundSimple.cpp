// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Backgrounds/UIScene_MenuBackgroundSimple.h"
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

UIScene_MenuBackgroundSimple::UIScene_MenuBackgroundSimple()
{
	Set_Layout("menu_background_simple");	
}

const char* UIScene_MenuBackgroundSimple::Get_Name()
{
	return "UIScene_MenuBackgroundSimple";
}

bool UIScene_MenuBackgroundSimple::Should_Render_Lower_Scenes()
{
	return false;//true;
}

bool UIScene_MenuBackgroundSimple::Should_Tick_When_Not_Top()
{
	return true;
}

void UIScene_MenuBackgroundSimple::Refresh(UIManager* manager)
{
	// Refresh base stuff.
	UIScene::Refresh(manager);
}

void UIScene_MenuBackgroundSimple::Enter(UIManager* manager)
{
}	

void UIScene_MenuBackgroundSimple::Exit(UIManager* manager)
{
}	

void UIScene_MenuBackgroundSimple::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_MenuBackgroundSimple::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}



