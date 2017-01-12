// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/UIScene_ColorSelector.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UIListView.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_ColorSelector::UIScene_ColorSelector(Color* color)
	: m_color(color)
{
	Set_Layout("color_selector");	
}

bool UIScene_ColorSelector::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_ColorSelector::Get_Name()
{
	return "UIScene_ColorSelector";
}

bool UIScene_ColorSelector::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_ColorSelector::Should_Display_Cursor()
{
	return true;
}

bool UIScene_ColorSelector::Is_Focusable()
{
	return true;
}

void UIScene_ColorSelector::Enter(UIManager* manager)
{
	Find_Element<UISlider*>("red_slider")->Set_Progress(m_color->R / 255.0f);
	Find_Element<UISlider*>("green_slider")->Set_Progress(m_color->G / 255.0f);
	Find_Element<UISlider*>("blue_slider")->Set_Progress(m_color->B / 255.0f);
	Find_Element<UISlider*>("alpha_slider")->Set_Progress(m_color->A / 255.0f);
	Find_Element<UIPanel*>("color_panel")->Set_Background_Color(*m_color);
}	

void UIScene_ColorSelector::Exit(UIManager* manager)
{
}	

void UIScene_ColorSelector::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_color->R = (u8)(255.0f * Find_Element<UISlider*>("red_slider")->Get_Progress());
	m_color->G = (u8)(255.0f * Find_Element<UISlider*>("green_slider")->Get_Progress());
	m_color->B = (u8)(255.0f * Find_Element<UISlider*>("blue_slider")->Get_Progress());
	m_color->A = (u8)(255.0f * Find_Element<UISlider*>("alpha_slider")->Get_Progress());
	Find_Element<UIPanel*>("color_panel")->Set_Background_Color(*m_color);

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_ColorSelector::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_ColorSelector::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UIFadeOutTransition()));
			}
		}
		break;
	}
}
