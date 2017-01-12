// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/UIScene_Video.h"
#include "Game/UI/Scenes/UIScene_Loading.h"
#include "Game/UI/Scenes/Backgrounds/UIScene_MenuBackground.h"
#include "Engine/UI/Layouts/UILayoutFactory.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/Transitions/UIFadeTransition.h"
#include "Engine/UI/Transitions/UIFadeOutTransition.h"
#include "Engine/UI/Elements/UIPanel.h"
#include "Engine/UI/Elements/UIProgressBar.h"
#include "Engine/UI/Elements/UILabel.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Network/NetManager.h"
#include "Engine/Network/NetClient.h"
#include "Engine/Network/NetServer.h"

#include "Game/Network/GameNetClient.h"
#include "Game/Network/GameNetUser.h"
#include "Game/Network/GameNetServer.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Game/UI/Scenes/UIScene_Game.h"

#include "Engine/Video/VideoHandle.h"
#include "Engine/Video/VideoRenderer.h"

#include "Game/Runner/GameOptions.h"
#include "Game/Runner/Game.h"

#include "Engine/Audio/AudioRenderer.h"

UIScene_Video::UIScene_Video(VideoHandle* video, UIAction next_screen)
	: m_video(video)
	, m_original_bgm_multiplier(0.0f)
	, m_next_screen(next_screen)
	, m_skip_timer(0.0f)
	, m_last_skip_timer(0.0f)
	, m_skip_alpha(0.0f)
{
	m_video_renderer = VideoRenderer::Create();
	m_video_renderer->Load(video->Get());

	Set_Layout("video");
}

UIScene_Video::~UIScene_Video()
{
	SAFE_DELETE(m_video_renderer);
}

const char* UIScene_Video::Get_Name()
{
	return "UIScene_Video";
}

bool UIScene_Video::Can_Accept_Invite()
{
	return false;
}

bool UIScene_Video::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Video::Should_Render_Lower_Scenes_Background()
{
	return false;
}

UIScene* UIScene_Video::Get_Background(UIManager* manager)
{
	return NULL;
}

void UIScene_Video::Enter(UIManager* manager)
{
	m_original_bgm_multiplier = AudioRenderer::Get()->Get_BGM_Volume_Multiplier();
	AudioRenderer::Get()->Set_BGM_Volume_Multiplier(0.0f);

	NetManager* netmanager = NetManager::Get();
	GameNetClient* client = static_cast<GameNetClient*>(netmanager->Client());
	client->Set_Game_State(GameNetClient_GameState::InCutscene);
}

void UIScene_Video::Exit(UIManager* manager)
{
	AudioRenderer::Get()->Set_BGM_Volume_Multiplier(m_original_bgm_multiplier);

	NetManager* netmanager = NetManager::Get();
	GameNetClient* client = static_cast<GameNetClient*>(netmanager->Client());
 	client->Set_Game_State(GameNetClient_GameState::InGame);
}

void UIScene_Video::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	UIPanel* video_panel = Find_Element<UIPanel*>("video");
	UILabel* subtitle_text = Find_Element<UILabel*>("subtitle");

	if (scene_index == manager->Get_Top_Scene_Index())
	{
		// Skip button.
		if (manager->Is_Down(OutputBindings::GUI_Select) || manager->Is_Down(OutputBindings::Interact))
		{
			m_skip_timer += time.Get_Delta_Seconds();
		}
		else
		{
			m_skip_timer = 0.0f;
		}

		if (m_skip_timer > SKIP_DURATION)
		{
			manager->Go(m_next_screen);
			return;
		}

		// Video state.
		VideoRendererState::Type state = m_video_renderer->Get_State();
		switch (state)
		{
		case VideoRendererState::Loading:
			{
				break;
			}
		case VideoRendererState::Ready:
			{
				m_video_renderer->Start();
				break;
			}
		case VideoRendererState::Running:
			{
				break;
			}
		case VideoRendererState::Error:
		case VideoRendererState::Idle:
		case VideoRendererState::Finished:
			{
				manager->Go(m_next_screen);
				//manager->Go(UIAction::Pop(new UIFadeOutTransition()));
				break;
			}
		}

		subtitle_text->Set_Value(m_video_renderer->Get_Subtitle());
		video_panel->Set_Background_Image(m_video_renderer->Get_Texture());
		video_panel->Set_Background_Color(state == VideoRendererState::Running ? Color::White : Color::Black);

		m_video_renderer->Tick(time.Get_Delta_Seconds());
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Video::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	m_skip_alpha = Math::Lerp(m_skip_alpha, (m_skip_timer > m_last_skip_timer) ? 1.0f : 0.0f, 8.0f * time.Get_Delta_Seconds());

	Color target_color(255.0f, 255.0f, 255.0f, 255.0f * m_skip_alpha);

	Find_Element<UIProgressBar*>("skip_progress_bar")->Set_Progress(Clamp(m_skip_timer / (SKIP_DURATION * 0.8f), 0.0f, 1.0f)); // *0.8 is so there is still a small "safety margin" before skipping.
	Find_Element<UIProgressBar*>("skip_progress_bar")->Set_Color(target_color);
	Find_Element<UILabel*>("skip_progress_text")->Set_Color(target_color);

	UIScene::Draw(time, manager, scene_index);
}



