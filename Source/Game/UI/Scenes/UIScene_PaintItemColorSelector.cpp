// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_PaintItemColorSelector.h"
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

#include "Game/Profile/ItemManager.h"

UIScene_PaintItemColorSelector::UIScene_PaintItemColorSelector(Color* color, Item* item)
	: m_color(color)
	, m_preview_item(item)
	, m_direction(0)
	, m_direction_timer(0.0f)
	, m_selected_option(0)
{
	Set_Layout("paint_item_color_selector");
}

bool UIScene_PaintItemColorSelector::Can_Accept_Invite()
{
	return false;
}

const char* UIScene_PaintItemColorSelector::Get_Name()
{
	return "UIScene_PaintItemColorSelector";
}

bool UIScene_PaintItemColorSelector::Should_Render_Lower_Scenes()
{
	return true;
}

bool UIScene_PaintItemColorSelector::Should_Display_Cursor()
{
	return true;
}

bool UIScene_PaintItemColorSelector::Is_Focusable()
{
	return true;
}

int UIScene_PaintItemColorSelector::Get_Selected_Index()
{
	return m_selected_option;
}

void UIScene_PaintItemColorSelector::Enter(UIManager* manager)
{
	Find_Element<UISlider*>("red_slider")->Set_Progress(m_color->R / 255.0f);
	Find_Element<UISlider*>("green_slider")->Set_Progress(m_color->G / 255.0f);
	Find_Element<UISlider*>("blue_slider")->Set_Progress(m_color->B / 255.0f);
	Find_Element<UISlider*>("alpha_slider")->Set_Progress(m_color->A / 255.0f);
	//Find_Element<UIPanel*>("color_panel")->Set_Background_Color(*m_color);

	Update_Preview();
}

void UIScene_PaintItemColorSelector::Exit(UIManager* manager)
{
}

void UIScene_PaintItemColorSelector::Update_Preview()
{
	UIPanel* preview = Find_Element<UIPanel*>("preview");

	if (m_preview_item->archetype->is_icon_direction_based && m_preview_item->archetype->item_slot != 1)
	{
		preview->Set_Background_Image(m_preview_item->archetype->icon_animations[m_direction]->Frames[0]);
		preview->Set_Background_Color(Color::White);
		preview->Set_Foreground_Image(m_preview_item->archetype->icon_tint_animations[m_direction]->Frames[0]);
		preview->Set_Foreground_Color(*m_color);
	}
	else
	{
		preview->Set_Background_Image(m_preview_item->archetype->icon_animations[0]->Frames[0]);
		preview->Set_Background_Color(Color::White);
		preview->Set_Foreground_Image(m_preview_item->archetype->icon_tint_animations[0]->Frames[0]);
		preview->Set_Foreground_Color(*m_color);
	}
}

void UIScene_PaintItemColorSelector::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_color->R = (u8)(255.0f * Find_Element<UISlider*>("red_slider")->Get_Progress());
	m_color->G = (u8)(255.0f * Find_Element<UISlider*>("green_slider")->Get_Progress());
	m_color->B = (u8)(255.0f * Find_Element<UISlider*>("blue_slider")->Get_Progress());
	m_color->A = (u8)(255.0f * Find_Element<UISlider*>("alpha_slider")->Get_Progress());
	//Find_Element<UIPanel*>("color_panel")->Set_Background_Color(*m_color);

	m_direction_timer += time.Get_Delta_Seconds();
	if (m_direction_timer >= 0.5f)
	{
		m_direction = (m_direction + 1) % 8;
		m_direction_timer = 0.0f;
	}

	Update_Preview();

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_PaintItemColorSelector::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_PaintItemColorSelector::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Button_Click:
	{
		if (e.Source->Get_Name() == "button_1")
		{
			m_selected_option = 0;
			manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));

			manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		}
		else if (e.Source->Get_Name() == "button_2")
		{
			m_selected_option = 1;
			manager->Get_Scene_From_Top(1)->Dispatch_Event(manager, UIEvent(UIEventType::Dialog_Close, NULL, this));

			manager->Go(UIAction::Pop(new UIFadeOutTransition()));
		}
	}
	break;
	}
}
