// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Base.h"
#include "Game/UI/Scenes/UIScene_Splash.h"
#include "Game/UI/Scenes/UIScene_Loading.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_MenuBackground.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_MenuBackgroundSimple.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_LoadingBackground.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_GameBackground.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_SubMenuBackground.h"
#include "Game/UI/Scenes/Editor/PfxEditor/UIScene_PFXBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Engine/Engine/EngineOptions.h"

#include "Game/Runner/Game.h"

UIScene_Base::UIScene_Base()
{
	Set_Layout("base");	
}

const char* UIScene_Base::Get_Name()
{
	return "UIScene_Base";
}

bool UIScene_Base::Can_Accept_Invite()
{
	return false;
}

bool UIScene_Base::Should_Render_Lower_Scenes()
{
	return false;
}

void UIScene_Base::Enter(UIManager* manager)
{
	// Initialize all persistent background scenes.
	manager->Store_Persistent_Scene(new UIScene_MenuBackground());
	manager->Store_Persistent_Scene(new UIScene_MenuBackgroundSimple());
	manager->Store_Persistent_Scene(new UIScene_LoadingBackground());
	manager->Store_Persistent_Scene(new UIScene_GameBackground());
	manager->Store_Persistent_Scene(new UIScene_SubMenuBackground());
	manager->Store_Persistent_Scene(new UIScene_PFXBackground());

	// If running as a server, go straight to loading.
	if (*EngineOptions::server == true)
	{
		manager->Go(UIAction::Push(new UIScene_Loading()));
	}
	else
	{
		manager->Go(UIAction::Push(new UIScene_Splash()));
	}
}	

void UIScene_Base::Exit(UIManager* manager)
{
}	

void UIScene_Base::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Base::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}



