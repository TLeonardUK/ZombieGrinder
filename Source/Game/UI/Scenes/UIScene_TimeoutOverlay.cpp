// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_TimeoutOverlay.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UITextPanel.h"
#include "Engine/UI/Elements/UISimpleListView.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/UIScene_Game.h"

#include "Engine/Network/NetUser.h"
#include "Game/Network/GameNetManager.h"
#include "Game/Network/GameNetUser.h"

#include "Game/Game/ChatManager.h"

#include "Engine/Localise/Locale.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

UIScene_TimeoutOverlay::UIScene_TimeoutOverlay()
{
	Set_Layout("TimeoutOverlay");
}

UIScene_TimeoutOverlay::~UIScene_TimeoutOverlay()
{
}

bool UIScene_TimeoutOverlay::Should_Tick_When_Not_Top()
{
	return false;
}

const char* UIScene_TimeoutOverlay::Get_Name()
{
	return "UIScene_TimeoutOverlay";
}

bool UIScene_TimeoutOverlay::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_TimeoutOverlay::Should_Render_Lower_Scenes_Background()
{
	return true;
}

bool UIScene_TimeoutOverlay::Is_Focusable()
{
	return false;
}

bool UIScene_TimeoutOverlay::Should_Display_Cursor()
{
	return false;
}

bool UIScene_TimeoutOverlay::Should_Display_Focus_Cursor()
{
	return false;
}

void UIScene_TimeoutOverlay::Enter(UIManager* manager)
{
}	

void UIScene_TimeoutOverlay::Exit(UIManager* manager)
{
}	

void UIScene_TimeoutOverlay::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
}

void UIScene_TimeoutOverlay::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Tick(time, manager, scene_index);
}

void UIScene_TimeoutOverlay::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	float time_elapsed = GameNetManager::Get()->Game_Client()->Get_Connection()->Get_Time_Since_Last_Contact() / 1000.0f;

	if (GameNetManager::Get()->Game_Client() != NULL &&
		time_elapsed >= 5.0f)
	{
		UILabel* label = Find_Element<UILabel*>("label");

		label->Set_Value(SF("#connect_overlay_error", time_elapsed));

		UIScene::Draw(time, manager, scene_index);
	}
}

void UIScene_TimeoutOverlay::Recieve_Event(UIManager* manager, UIEvent e)
{
}