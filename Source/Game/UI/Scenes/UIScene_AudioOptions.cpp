// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_AudioOptions.h"
#include "Engine/UI/Transitions/UISlideInTransition.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Audio/AudioRenderer.h"

#include "Engine/Localise/Locale.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Game/Runner/Game.h"
#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

UIScene_AudioOptions::UIScene_AudioOptions()
{
	Set_Layout("audio_options");	
}

const char* UIScene_AudioOptions::Get_Name()
{
	return "UIScene_AudioOptions";
}

bool UIScene_AudioOptions::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_AudioOptions::Should_Display_Cursor()
{
	return true;
}

bool UIScene_AudioOptions::Is_Focusable()
{
	return true;
}

UIScene* UIScene_AudioOptions::Get_Background(UIManager* manager)
{
	if (manager->Get_Scene_By_Type<UIScene_Game*>() != NULL)
		return UIScene::Get_Background(manager);
	else
		return manager->Retrieve_Persistent_Scene("UIScene_MenuBackgroundSimple");
}	

void UIScene_AudioOptions::Enter(UIManager* manager)
{
	Find_Element<UISlider*>("bgm_slider")->Set_Progress(*EngineOptions::audio_bgm_volume);
	Find_Element<UISlider*>("sfx_slider")->Set_Progress(*EngineOptions::audio_sfx_volume);
	Find_Element<UISlider*>("voice_recv_slider")->Set_Progress(*EngineOptions::audio_voice_recv_volume);
	Find_Element<UISlider*>("voice_send_slider")->Set_Progress(*EngineOptions::audio_voice_send_volume);
	Find_Element<UICheckBox*>("voice_enabled_checkbox")->Set_Checked(*EngineOptions::audio_voice_enabled);
	Find_Element<UICheckBox*>("voice_push_to_talk_checkbox")->Set_Checked(*EngineOptions::audio_push_to_talk_enabled);
}	

void UIScene_AudioOptions::Exit(UIManager* manager)
{
	*EngineOptions::audio_bgm_volume = Find_Element<UISlider*>("bgm_slider")->Get_Progress();
	*EngineOptions::audio_sfx_volume = Find_Element<UISlider*>("sfx_slider")->Get_Progress();
	*EngineOptions::audio_voice_recv_volume = Find_Element<UISlider*>("voice_recv_slider")->Get_Progress();
	*EngineOptions::audio_voice_send_volume = Find_Element<UISlider*>("voice_send_slider")->Get_Progress();
	*EngineOptions::audio_voice_enabled = Find_Element<UICheckBox*>("voice_enabled_checkbox")->Get_Checked();
	*EngineOptions::audio_push_to_talk_enabled = Find_Element<UICheckBox*>("voice_push_to_talk_checkbox")->Get_Checked();
}	

void UIScene_AudioOptions::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	AudioRenderer::Get()->Set_BGM_Volume(Find_Element<UISlider*>("bgm_slider")->Get_Progress());
	AudioRenderer::Get()->Set_SFX_Volume(Find_Element<UISlider*>("sfx_slider")->Get_Progress());

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_AudioOptions::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIScene::Draw(time, manager, scene_index);
}

void UIScene_AudioOptions::Recieve_Event(UIManager* manager, UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::Element_Focus:
		{
		}
		break;
	case UIEventType::Button_Click:
		{	
			if (e.Source->Get_Name() == "back_button")
			{
				manager->Go(UIAction::Pop(new UISlideInTransition()));
			}
		}
		break;
	}
}
