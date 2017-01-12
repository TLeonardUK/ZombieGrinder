// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_VoteOverlay.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UITextPanel.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UIProgressBar.h"

#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Input/Input.h"

#include "Engine/UI/UIManager.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Game/UI/Scenes/Editor/UIScene_Editor.h"
#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Game/ChatManager.h"
#include "Game/Game/VoteManager.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

UIScene_VoteOverlay::UIScene_VoteOverlay(UIScene_Game* game)
	: m_game_scene(game)
{
	Set_Layout("VoteOverlay");

	UIProgressBar* bar = Find_Element<UIProgressBar*>("vote_progress");
	m_original_bg_color = bar->Get_Background_Color();
	bar->Set_Background_Color(Color(0, 0, 0, 0));
}

UIScene_VoteOverlay::~UIScene_VoteOverlay()
{
}

bool UIScene_VoteOverlay::Should_Tick_When_Not_Top()
{
	return false;
}

const char* UIScene_VoteOverlay::Get_Name()
{
	return "UIScene_VoteOverlay";
}

bool UIScene_VoteOverlay::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_VoteOverlay::Should_Render_Lower_Scenes_Background()
{
	return false;
}

bool UIScene_VoteOverlay::Is_Focusable()
{
	return true;
}

bool UIScene_VoteOverlay::Should_Display_Cursor()
{
	return true;
}

bool UIScene_VoteOverlay::Should_Display_Focus_Cursor()
{
	return false;
}

void UIScene_VoteOverlay::Enter(UIManager* manager)
{
}	

void UIScene_VoteOverlay::Exit(UIManager* manager)
{
}	

void UIScene_VoteOverlay::Refresh(UIManager* manager)
{
	UIScene::Refresh(manager);
}

void UIScene_VoteOverlay::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	VoteManager* vm = VoteManager::Get();

	Vote* vote = vm->Get_Active_Vote();
	int totals = vm->Get_Total_Votes();
	int passes = vm->Get_Total_Pass_Votes();
	
	UILabel* label = Find_Element<UILabel*>("vote_description");
	UIProgressBar* bar = Find_Element<UIProgressBar*>("vote_progress");

	if (vote != NULL)
	{
		float remaining = vm->Get_Time_Remaining();
		label->Set_Value(StringHelper::Format("%s (%i)", vote->Get_Description().c_str(), (int)ceilf(remaining / 1000.0f)));
	}
	else
	{
		label->Set_Value("");
	}

	if (totals > 0)
	{
		float progress = (float)passes / (float)totals;
		bar->Set_Progress(progress);
		bar->Set_Background_Color(m_original_bg_color);
	}
	else
	{
		bar->Set_Progress(0.0f);
		bar->Set_Background_Color(Color(0, 0, 0, 0));
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_VoteOverlay::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}
